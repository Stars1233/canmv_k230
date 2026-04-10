/* Copyright (c) 2026, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stddef.h>
#include "ide_dbg.h"

#include "hal_rvv_ops.h"

#if CONFIG_CANMV_IDE_SUPPORT

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "k_vb_comm.h"
#include "py/runtime.h"

#include "kd_display.h"

#include "mpi_gsdma_api.h"
#include "mpi_sys_api.h"
#include "mpi_vb_api.h"
#include "mpi_venc_api.h"

#include "py_modules.h"

/* Context for VENC resource management and caching */
typedef struct {
    int        inited;
    k_u32      chn_id;
    k_u32      pool_id; // Single pool for both VENC and output
    k_u32      width; // Cached Display width
    k_u32      height; // Cached Display height
    int        quality; // Cached Quality
    k_rotation rotation; // Cached Rotation state

    // Single output buffer (allocated from the same pool)
    k_vb_blk_handle output_block;
    k_u64           output_phys_addr;
    void*           output_virt_addr;
    size_t          output_size;

#define MAX_VENC_PACKS 64
    k_venc_pack static_packs[MAX_VENC_PACKS];
} ide_dbg_vo_wbc_ctx_t;

static ide_dbg_vo_wbc_ctx_t g_wbc_ctx = {
    .inited           = 0,
    .chn_id           = -1,
    .pool_id          = VB_INVALID_POOLID,
    .output_block     = VB_INVALID_HANDLE,
    .output_phys_addr = 0,
    .output_virt_addr = NULL,
    .output_size      = 0,
    .rotation         = K_VPU_ROTATION_0,
};

static int g_wbc_enable = 0;

/**
 * @brief Cleanup resources without checking inited flag
 * @note Internal cleanup function that always releases resources
 */
static void wbc_cleanup_resources(void)
{
    // Store chn_id locally before modifying it
    k_s32 chn_id_to_cleanup = g_wbc_ctx.chn_id;

    // Stop and destroy VENC channel if chn_id is valid
    if (chn_id_to_cleanup != -1) {
        // Try to detach VB pool first (before destroying channel)
        if (g_wbc_ctx.pool_id != VB_INVALID_POOLID) {
            kd_mpi_venc_detach_vb_pool(chn_id_to_cleanup);
        }

        kd_mpi_venc_stop_chn(chn_id_to_cleanup);
        kd_mpi_venc_destroy_chn(chn_id_to_cleanup);
        kd_mpi_venc_release_chn(chn_id_to_cleanup);
        g_wbc_ctx.chn_id = -1;
    }

    // Release output buffer
    if (g_wbc_ctx.output_virt_addr) {
        kd_mpi_sys_munmap(g_wbc_ctx.output_virt_addr, g_wbc_ctx.output_size);
        g_wbc_ctx.output_virt_addr = NULL;
    }

    if (g_wbc_ctx.output_block != VB_INVALID_HANDLE) {
        kd_mpi_vb_release_block(g_wbc_ctx.output_block);
        g_wbc_ctx.output_block     = VB_INVALID_HANDLE;
        g_wbc_ctx.output_phys_addr = 0;
        g_wbc_ctx.output_size      = 0;
    }

    // Destroy VB pool if valid
    if (g_wbc_ctx.pool_id != VB_INVALID_POOLID) {
        kd_mpi_vb_destory_pool(g_wbc_ctx.pool_id);
        g_wbc_ctx.pool_id = VB_INVALID_POOLID;
    }

    // Always release WBC frame
    py_display_wbc_dump_relase();

    // Reset all other state
    g_wbc_ctx.width    = 0;
    g_wbc_ctx.height   = 0;
    g_wbc_ctx.quality  = 0;
    g_wbc_ctx.rotation = K_VPU_ROTATION_0;
    hal_rvv_memset(g_wbc_ctx.static_packs, 0, sizeof(g_wbc_ctx.static_packs));
}

void ide_dbg_vo_wbc_start(int enable)
{
    if (enable) {
        ide_dbg_enable_vo_wbc();
    }

    g_wbc_enable = enable;
}

/**
 * @brief Release VENC channel and return the ID to the system.
 */
void ide_dbg_vo_wbc_stop(void)
{
    g_wbc_enable = 0;

    // Always cleanup resources regardless of inited flag
    // This handles partial initialization cases
    wbc_cleanup_resources();

    // Reset the inited flag after cleanup
    g_wbc_ctx.inited = 0;
}

/**
 * @brief Allocate output buffer from the same pool at init time
 */
static k_s32 wbc_alloc_output_buffer(k_u32 buffer_size)
{
    // Allocate output buffer from the same pool
    g_wbc_ctx.output_block = kd_mpi_vb_get_block(g_wbc_ctx.pool_id, buffer_size, NULL);
    if (g_wbc_ctx.output_block == VB_INVALID_HANDLE) {
        return -1;
    }

    g_wbc_ctx.output_phys_addr = kd_mpi_vb_handle_to_phyaddr(g_wbc_ctx.output_block);
    if (g_wbc_ctx.output_phys_addr == 0) {
        kd_mpi_vb_release_block(g_wbc_ctx.output_block);
        g_wbc_ctx.output_block = VB_INVALID_HANDLE;
        return -1;
    }

    g_wbc_ctx.output_size = buffer_size;

    // Map buffer to kernel space at init time (cached for CPU access)
    g_wbc_ctx.output_virt_addr = kd_mpi_sys_mmap_cached(g_wbc_ctx.output_phys_addr, buffer_size);
    if (!g_wbc_ctx.output_virt_addr) {
        kd_mpi_vb_release_block(g_wbc_ctx.output_block);
        g_wbc_ctx.output_block     = VB_INVALID_HANDLE;
        g_wbc_ctx.output_phys_addr = 0;
        g_wbc_ctx.output_size      = 0;
        return -1;
    }

    return 0;
}

static k_s32 ide_dbg_vo_wbc_init(void)
{
    k_s32              ret;
    k_video_frame_info probe_vf;
    k_u32              display_width, display_height;
    k_rotation         target_rot = K_VPU_ROTATION_0;

    //  Detect Rotation by probing dimensions from the WBC
    ret = py_display_wbc_dump(&probe_vf, 1000, 1);
    if (0x00 != ret) {
        goto _err;
    }
    py_display_wbc_dump_relase();

    // Clear any existing resources first (in case of previous failed init)
    wbc_cleanup_resources();

    // Cache Display Resolution and Quality (only once)
    ret = kd_display_get_resolution(&display_width, &display_height);
    if (ret != 0) {
        goto _err;
    }
    g_wbc_ctx.quality = py_display_wbc_quality();

    // If dimensions differ from display, check hardware rotation flags
    if (probe_vf.v_frame.width != display_width || probe_vf.v_frame.height != display_height) {
        k_vo_dev_attr vo_attr;
        if (kd_mpi_vo_get_dev_attr(&vo_attr) == 0) {
            if (vo_attr.dev_rot_flg & GDMA_ROTATE_DEGREE_90)
                target_rot = K_VPU_ROTATION_90;
            else if (vo_attr.dev_rot_flg & GDMA_ROTATE_DEGREE_180)
                target_rot = K_VPU_ROTATION_180;
            else if (vo_attr.dev_rot_flg & GDMA_ROTATE_DEGREE_270)
                target_rot = K_VPU_ROTATION_270;
        }
    }

    // Request VENC channel dynamically
    ret = kd_mpi_venc_request_chn(&g_wbc_ctx.chn_id);
    if (ret != 0) {
        // Don't set chn_id to -1 here, cleanup will handle it
        goto _err;
    }

    // Create single VB Pool for both VENC and output
    k_u32 blk_size = (display_width * display_height * 3 / 2);
    blk_size       = VB_ALIGN_UP(blk_size, 4096);

    // Create pool with 2 blocks: 1 for VENC, 1 for our output buffer
    g_wbc_ctx.pool_id = kd_mpi_vb_create_pool_ex(blk_size, 2, VB_REMAP_MODE_NOCACHE);
    if (g_wbc_ctx.pool_id == VB_INVALID_POOLID) {
        goto _err;
    }

    //  Allocate output buffer from the same pool
    ret = wbc_alloc_output_buffer(blk_size);
    if (ret != 0) {
        goto _err;
    }

    //  Setup VENC attributes
    k_venc_chn_attr attr;
    hal_rvv_memset(&attr, 0, sizeof(attr));
    attr.venc_attr.type = K_PT_JPEG;

    // Set input dimensions: Swap width/height if WBC is providing a rotated buffer
    if (target_rot == K_VPU_ROTATION_90 || target_rot == K_VPU_ROTATION_270) {
        attr.venc_attr.pic_width  = display_height;
        attr.venc_attr.pic_height = display_width;
    } else {
        attr.venc_attr.pic_width  = display_width;
        attr.venc_attr.pic_height = display_height;
    }

    g_wbc_ctx.width  = attr.venc_attr.pic_width;
    g_wbc_ctx.height = attr.venc_attr.pic_height;

    attr.rc_attr.rc_mode                    = K_VENC_RC_MODE_MJPEG_FIXQP;
    attr.rc_attr.mjpeg_fixqp.src_frame_rate = 30;
    attr.rc_attr.mjpeg_fixqp.dst_frame_rate = 30;
    attr.rc_attr.mjpeg_fixqp.q_factor       = g_wbc_ctx.quality;

    // venc hardware use only one buffer in pool.
    kd_mpi_venc_attach_vb_pool_ex(g_wbc_ctx.chn_id, g_wbc_ctx.pool_id, 1);

    ret = kd_mpi_venc_create_chn(g_wbc_ctx.chn_id, &attr);
    if (ret != 0) {
        goto _err;
    }

    //  Apply rotation to the VENC channel
    if (target_rot != K_VPU_ROTATION_0) {
        kd_mpi_venc_set_rotation(g_wbc_ctx.chn_id, target_rot);
    }

    ret = kd_mpi_venc_start_chn(g_wbc_ctx.chn_id);
    if (ret != 0) {
        goto _err;
    }

    g_wbc_ctx.rotation = target_rot;
    g_wbc_ctx.inited   = 1;

    return 0;

_err:
    printf("ERROR: idg dbg vo wbc init failed\n");

    // Clean up any partially allocated resources
    wbc_cleanup_resources();
    return -1;
}

/**
 * @brief High-speed dump and encode. Operates purely on cached state.
 */
int ide_dbg_vo_wbc_dump_and_encode(void** buffer, size_t* buffer_size, uint32_t* image_width, uint32_t* image_height)
{
    if (!g_wbc_enable) {
        return -1;
    }

    // Dump the frame from WBC
    k_video_frame_info vf_info;
    if (py_display_wbc_dump(&vf_info, 1000, 1) != 0) {
        // printf("ide dbg vo wbc failed to dump WBC frame\n");
        return -1;
    }

    // Initialize once only
    if (!g_wbc_ctx.inited) {
        if (ide_dbg_vo_wbc_init() != 0) {
            py_display_wbc_dump_relase();

            return -1;
        }
    }

    // Send to VENC
    if (kd_mpi_venc_send_frame(g_wbc_ctx.chn_id, &vf_info, 1000) != 0) {
        py_display_wbc_dump_relase();

        printf("ide dbg vo wbc failed to send frame to VENC\n");
        return -1;
    }

    // NOTE: Do NOT release the WBC block here! VENC reads from it asynchronously.
    // We must hold the WBC block until kd_mpi_venc_get_stream() returns, which
    // guarantees the VPU has finished reading from the WBC buffer. Releasing early
    // allows the WBC ISR to reuse this block as a DMA write target while the VPU
    // is still reading, causing memory corruption.

    // Query VENC status first to get pack count
    k_venc_chn_status venc_status;
    hal_rvv_memset(&venc_status, 0, sizeof(venc_status));

    if (kd_mpi_venc_query_status(g_wbc_ctx.chn_id, &venc_status) != 0) {
        py_display_wbc_dump_relase();

        printf("ide dbg vo wbc failed to query VENC status\n");
        return -1;
    }

    // Get stream from VENC using static packs
    k_venc_stream stream;
    hal_rvv_memset(&stream, 0, sizeof(stream));

    // Set pack_cnt based on status query
    stream.pack_cnt = (venc_status.cur_packs > 0) ? venc_status.cur_packs : 1;

    // IMPORTANT: Cap pack count to our static array size
    if (stream.pack_cnt > MAX_VENC_PACKS) {
        printf("ide dbg vo wbc warning: truncating packs from %d to %d\n", stream.pack_cnt, MAX_VENC_PACKS);
        stream.pack_cnt = MAX_VENC_PACKS;
    }

    // Use static memory instead of malloc
    stream.pack = g_wbc_ctx.static_packs;

    // Initialize pack array
    hal_rvv_memset(stream.pack, 0, sizeof(k_venc_pack) * stream.pack_cnt);

    if (kd_mpi_venc_get_stream(g_wbc_ctx.chn_id, &stream, 1000) != 0) {
        py_display_wbc_dump_relase();

        printf("ide dbg vo wbc failed to get stream from VENC\n");
        return -1;
    }

    // VENC has finished reading the WBC buffer — safe to release now
    py_display_wbc_dump_relase();

    // Check if we have valid packets
    if (stream.pack_cnt == 0 || stream.pack[0].phys_addr == 0) {
        // Still need to release stream even if it has no valid packets
        kd_mpi_venc_release_stream(g_wbc_ctx.chn_id, &stream);

        printf("ide dbg vo wbc no valid packets in stream\n");
        return -1;
    }

    // Calculate total length and copy data
    size_t total_len = 0;
    for (k_u32 i = 0; i < stream.pack_cnt && i < MAX_VENC_PACKS; i++) {
        if (stream.pack[i].phys_addr != 0) {
            total_len += stream.pack[i].len;
        }
    }

    if (0x00 == total_len) {
        kd_mpi_venc_release_stream(g_wbc_ctx.chn_id, &stream);

        printf("ide dbg vo wbc total length is 0\n");
        return -1;
    }

    // Check if buffer is large enough
    if (total_len > g_wbc_ctx.output_size) {
        kd_mpi_venc_release_stream(g_wbc_ctx.chn_id, &stream);

        printf("ide dbg vo wbc Output buffer too small: need %zu, have %zu\n", total_len, g_wbc_ctx.output_size);
        return -1;
    }

    // Copy data using DMA for each packet
    k_u64 dst_offset = 0;
    for (k_u32 i = 0; i < stream.pack_cnt && i < MAX_VENC_PACKS; i++) {
        if (stream.pack[i].len > 0 && stream.pack[i].phys_addr != 0) {
            kd_mpi_dma_memcpy((void*)(uintptr_t)(g_wbc_ctx.output_phys_addr + dst_offset),
                              (void*)(uintptr_t)stream.pack[i].phys_addr, stream.pack[i].len);
            dst_offset += stream.pack[i].len;
        }
    }

    // Invalidate cache
    kd_mpi_sys_mmz_invalidate_cache(g_wbc_ctx.output_phys_addr, g_wbc_ctx.output_virt_addr, total_len);

    // Return buffer info
    *buffer      = g_wbc_ctx.output_virt_addr;
    *buffer_size = total_len;

    *image_width  = g_wbc_ctx.width;
    *image_height = g_wbc_ctx.height;

    // Release stream
    kd_mpi_venc_release_stream(g_wbc_ctx.chn_id, &stream);

    // printf("%s->%d Success: encoded %zu bytes, %dx%d, packs=%d\n", __func__, __LINE__, total_len, *image_width,
    // *image_height,
    //        stream.pack_cnt);

    return 0;
}

#endif // CONFIG_CANMV_IDE_SUPPORT
