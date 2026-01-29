#include <stdio.h>
#include <string.h>

#include "py/obj.h"
#include "py/runtime.h"

#include "mpthread.h"

#include "mpp_vb_mgmt.h"

#include "mpi_vicap_api.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// #define VB_MGMT_RECORD_MAX_CNT (128)
// #define VB_MGMT_RECORD_MAGIC_IN_USE (0x1234ABCD)

// struct vb_record_t
// {
//     void *virt_addr; // self.virt_addr = virt_addr
//     k_u32 magic;
//     k_u32 size;             // self.size = size, user set
//     k_vb_blk_handle handle; // self.handle = handle
// };

// static struct vb_record_t vb_records[VB_MGMT_RECORD_MAX_CNT];
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/* vicap mgmt */
static k_u8 vicap_dev_stat[VICAP_MAX_DEV_NUMS];

static k_s32 vb_mgmt_deinit_vicap(k_u32 id)
{
    k_s32 ret = 0;

    if (0x00 == vicap_dev_stat[id])
    {
        return 0;
    }

    if (0x00 != kd_mpi_vicap_stop_stream(id))
    {
        ret += 1;
        printf("vb_mgmt stop vicap %d stream failed.\n", id);
    }

    if (0x00 != kd_mpi_vicap_deinit(id))
    {
        ret += 1;
        printf("vb_mgmt stop vicap %d failed.\n", id);
    }

    vicap_dev_stat[id] = 0;

    return ret;
}

k_s32 vb_mgmt_vicap_dev_inited(k_u32 id)
{
    if (id > VICAP_MAX_DEV_NUMS)
    {
        return 1;
    }

    vicap_dev_stat[id] = 1;

    return 0;
}

k_s32 vb_mgmt_vicap_dev_deinited(k_u32 id)
{
    if (id > VICAP_MAX_DEV_NUMS)
    {
        return 1;
    }

    vicap_dev_stat[id] = 0;

    return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/* mpp link mgmt */
#define VB_MGMT_VICAP_IMAGE_MAX_CNT (32)
#define VB_MGMT_VICAP_IMAGE_MAGIC_IN_USE (0x1234DEAD)

static vb_mgmt_vicap_image vicap_images[VB_MGMT_VICAP_IMAGE_MAX_CNT];

k_s32 vb_mgmt_dump_vicap_frame(vb_mgmt_dump_vicap_config *cfg, vb_mgmt_vicap_image *image)
{
    vb_mgmt_vicap_image *_image = NULL;

    if((NULL == cfg) || (NULL == image)) {
        return 1;
    }

    for (int i = 0; i < VB_MGMT_VICAP_IMAGE_MAX_CNT; i++)
    {
        if (VB_MGMT_VICAP_IMAGE_MAGIC_IN_USE != vicap_images[i].magic)
        {
            _image = &vicap_images[i];

            _image->magic = VB_MGMT_VICAP_IMAGE_MAGIC_IN_USE;

            break;
        }
    }

    if(NULL == _image) {
        printf("no space to record vicap_image\n");

        return 2;
    }

    memcpy(&_image->cfg, cfg, sizeof(*cfg));

    MP_THREAD_GIL_EXIT();

    if(0x00 != kd_mpi_vicap_dump_frame(cfg->dev_num, cfg->chn_num, cfg->foramt, &_image->vf_info, cfg->milli_sec)) {
        MP_THREAD_GIL_ENTER();

        printf("vicap dump dev %u chn %u failed.\n", cfg->dev_num, cfg->chn_num);

        _image->magic = 0x00;

        return 3;
    }
    MP_THREAD_GIL_ENTER();

    k_u32 img_width = _image->vf_info.v_frame.width;
    k_u32 img_height = _image->vf_info.v_frame.height;

    extern k_u32 calc_video_size(k_pixel_format video_fmt, k_u16 width, k_u16 height);
    _image->image_size = calc_video_size(_image->vf_info.v_frame.pixel_format, img_width, img_height);

    if(0x00 == _image->image_size) {
        printf("unsupport image format %u\n", _image->vf_info.v_frame.pixel_format);

        kd_mpi_vicap_dump_release(_image->cfg.dev_num, _image->cfg.chn_num, &_image->vf_info);
        _image->magic = 0x00;

        return 4;
    }

    _image->vf_info.v_frame.virt_addr[0] = (k_u64)kd_mpi_sys_mmap_cached(_image->vf_info.v_frame.phys_addr[0], _image->image_size);

    if(0x00 == _image->vf_info.v_frame.virt_addr[0])
    {
        printf("mmap failed.\n");

        kd_mpi_vicap_dump_release(_image->cfg.dev_num, _image->cfg.chn_num, &_image->vf_info);
        _image->magic = 0x00;

        return 5;
    }

    memcpy(image, _image, sizeof(*_image));

    return 0;
}

k_s32 vb_mgmt_release_vicap_frame(vb_mgmt_vicap_image *image)
{
    k_s32 ret = 0;

    if((NULL == image) || (VB_MGMT_VICAP_IMAGE_MAGIC_IN_USE != image->magic))
    {
        return 1;
    }

    if(0x00 != (ret += kd_mpi_sys_munmap((void *)image->vf_info.v_frame.virt_addr[0], image->image_size)))
    {
        printf("release image failed(1).\n");
    }

    if(0x00 != (ret += kd_mpi_vicap_dump_release(image->cfg.dev_num, image->cfg.chn_num, &image->vf_info)))
    {
        printf("release image failed(2).\n");
    }

    if(0x00 == ret)
    {
        for (int i = 0; i < VB_MGMT_VICAP_IMAGE_MAX_CNT; i++)
        {
            if ((VB_MGMT_VICAP_IMAGE_MAGIC_IN_USE == vicap_images[i].magic) && \
                (image->vf_info.v_frame.phys_addr[0] == vicap_images[i].vf_info.v_frame.phys_addr[0]) && \
                (image->vf_info.v_frame.virt_addr[0] == vicap_images[i].vf_info.v_frame.virt_addr[0]))
            {
                vicap_images[i].magic = 0x00;
            }
        }
    }

    return ret;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
k_s32 vb_mgmt_init(void)
{
    for (int i = 0; i < VB_MGMT_VICAP_IMAGE_MAX_CNT; i++)
    {
        if (VB_MGMT_VICAP_IMAGE_MAGIC_IN_USE == vicap_images[i].magic)
        {
            printf("maybe not call vb_mgmt_deinit, the vicap images record %d is in use\n", i);

            vb_mgmt_release_vicap_frame(&vicap_images[i]);
        }

        vicap_images[i].magic = 0;
    }

    return 0;
}

k_s32 vb_mgmt_deinit(void)
{
    for (int i = 0; i < VB_MGMT_VICAP_IMAGE_MAX_CNT; i++)
    {
        if (VB_MGMT_VICAP_IMAGE_MAGIC_IN_USE == vicap_images[i].magic)
        {
            vb_mgmt_release_vicap_frame(&vicap_images[i]);
        }
    }

    for (int i = 0; i < VICAP_MAX_DEV_NUMS; i++)
    {
        vb_mgmt_deinit_vicap(i);
    }
    usleep(1000 * 50);

    return 0;
}
