/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * USB debug support.
 *
 */
#ifndef __IDE_DBG_H__
#define __IDE_DBG_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "generated/autoconf.h"

typedef enum {
    EXEC_STRING,  // SCRIPT_EXEC (0x05): exec_payload = source string
    EXEC_FILE,    // USBDBG_FILE_EXEC: exec_payload = file path
} exec_type_t;

#define USBDBG_VTOUCH_DEVICE_ID           (0x7e)
#define USBDBG_VTOUCH_QUEUE_LEN           (16)

struct ide_dbg_vtouch_event {
    uint16_t x;
    uint16_t y;
    uint8_t event;
    uint8_t track_id;
    uint16_t width;
    uint32_t timestamp_ms;
};

#if CONFIG_CANMV_IDE_SUPPORT

#define NEW_SAVE_FILE_IMPL

/**
  * To add a new debugging command, increment the last command value used.
  * Set the MSB of the value if the request has a device-to-host data phase.
  * Add the command to usr/openmv.py using the same value.
  * Handle the command control and data in/out (if any) phases in usbdbg.c.
  *
  * See usbdbg.c for examples.
  */
enum usbdbg_cmd {
    USBDBG_NONE             = 0x00,

    // Shared by both legacy Qt IDE protocol and capability protocol.
    USBDBG_SCRIPT_EXEC      = 0x05,
    USBDBG_SCRIPT_STOP      = 0x06,
    USBDBG_FB_ENABLE        = 0x0D,
    USBDBG_TX_INPUT         = 0x11,
    USBDBG_FW_VERSION       = 0x80,
    USBDBG_FRAME_SIZE       = 0x81,
    USBDBG_FRAME_DUMP       = 0x82,
    USBDBG_ARCH_STR         = 0x83,
    USBDBG_TX_BUF_LEN       = 0x8E,
    USBDBG_TX_BUF           = 0x8F,
    USBDBG_VERIFYFILE       = 0xA1,

    // Legacy-only commands. Do not change these opcodes or their wire
    // semantics; old IDE builds rely on them.
    USBDBG_SCRIPT_SAVE      = 0x07,
    // USBDBG_FILE_SAVE        = 0x07,
    USBDBG_TEMPLATE_SAVE    = 0x08,
    USBDBG_DESCRIPTOR_SAVE  = 0x09,
    USBDBG_ATTR_WRITE       = 0x0B,
    USBDBG_SYS_RESET        = 0x0C,
    USBDBG_SET_TIME         = 0x12,
    USBDBG_CREATEFILE       = 0x20,
    USBDBG_WRITEFILE        = 0x21,
    USBDBG_SCRIPT_RUNNING   = 0x87,
    // USBDBG_FILE_SAVE_STATUS = 0x88,
    USBDBG_ATTR_READ        = 0x8A,
    USBDBG_QUERY_STATUS     = 0x8D, // 0xFFEEBBAA
    USBDBG_SENSOR_ID        = 0x90,
    USBDBG_QUERY_FILE_STAT  = 0xA0,

    // Capability protocol v2 extensions. Keep this range contiguous; append
    // new extension-only commands after USBDBG_VTOUCH_EVENT.
    USBDBG_CAPABILITIES     = 0xA2,
    USBDBG_FW_VERSION_FULL  = 0xA3,
    USBDBG_SCRIPT_STATUS    = 0xA4,
    USBDBG_QUERY_FILE_STAT2 = 0xA5,
    USBDBG_LIST_DIR         = 0xA6,
    USBDBG_READ_FILE        = 0xA7,
    USBDBG_CREATEFILE2      = 0xA8,
    USBDBG_WRITEFILE2       = 0xA9,
    USBDBG_DELETE_FILE      = 0xAA,
    USBDBG_RENAME_FILE      = 0xAB,
    USBDBG_MKDIR            = 0xAC,
    USBDBG_RMDIR            = 0xAD,
    USBDBG_FILE_EXEC        = 0xAE,
    USBDBG_VTOUCH_STATUS    = 0xAF,
    USBDBG_VTOUCH_EVENT     = 0xB0,
};

#ifdef NEW_SAVE_FILE_IMPL

#define USBDBG_SVFILE_NAME_LEN            (64)

#define USBDBG_SVFILE_ERR_NONE            (1024 + 0)
#define USBDBG_SVFILE_ERR_PATH_ERR        (1024 + 1)
#define USBDBG_SVFILE_ERR_CHKSUM_ERR      (1024 + 2)
#define USBDBG_SVFILE_ERR_WRITE_ERR       (1024 + 3)
#define USBDBG_SVFILE_ERR_CHUNK_ERR       (1024 + 4)

#define USBDBG_SVFILE_VERIFY_NOT_OPEN     (1024 + 5)
#define USBDBG_SVFILE_VERIFY_SHA2_ERR     (1024 + 6)
#define USBDBG_SVFILE_VERIFY_ERR_NONE     (1024 + 7)

#define USBDBG_SVFILE_ERR_OPEN_ERR        (1024 + 8)

#define USBDBG_SVFILE_ERR_FILE_NOT_FOUND   (1024 + 9)
#define USBDBG_SVFILE_ERR_FILE_EXISTS      (1024 + 10)
#define USBDBG_SVFILE_ERR_INVALID_PATH     (1024 + 11)
#define USBDBG_SVFILE_ERR_DIR_NOT_EMPTY    (1024 + 12)
#define USBDBG_SVFILE_ERR_PERM_DENIED      (1024 + 13)
#define USBDBG_SVFILE_ERR_IO_ERROR         (1024 + 14)

#define USBDBG_CAP_PROTOCOL_VERSION        2
#define USBDBG_CAP_LIST_DIR               (1 << 0)
#define USBDBG_CAP_READ_FILE              (1 << 1)
#define USBDBG_CAP_WRITE_FILE             (1 << 2)
#define USBDBG_CAP_DELETE_FILE            (1 << 3)
#define USBDBG_CAP_RENAME_FILE            (1 << 4)
#define USBDBG_CAP_MKDIR                  (1 << 5)
#define USBDBG_CAP_RMDIR                  (1 << 6)
#define USBDBG_CAP_FILE_EXEC              (1 << 7)
#define USBDBG_CAP_VIRTUAL_TOUCH          (1 << 8)
#define USBDBG_CAP_REPL_INPUT             (1 << 9)

#define USBDBG_MAX_PATH_LEN               512
#define USBDBG_FILE_CHUNK_MAX             (128 * 1024)

struct ide_dbg_listdir_entry {
    uint8_t  type;
    uint8_t  name_len;
    uint32_t size;
    uint32_t mtime;
};

struct ide_dbg_svfil_info_t {
    int chunk_size;
    char name[USBDBG_SVFILE_NAME_LEN + 4];
    uint8_t sha256[32];
};

struct ide_dbg_svfil_t {
    int errcode;

    FILE* file;

    uint8_t *chunk_buffer;
    struct ide_dbg_svfil_info_t info;
};

struct ide_dbg_vtouch_info {
    uint32_t supported;
    uint32_t enabled;
    uint32_t range_x;
    uint32_t range_y;
    uint32_t queue_depth;
};

#else
bool      ide_dbg_need_save_file();
void      ide_save_file(); 
#endif

typedef enum{
    IDE_DBG_STATUS_OK = 0,
    IDE_DBG_DISPATCH_STATUS_ERR = 1,
    IDE_DBG_DISPATCH_STATUS_WAIT,
    IDE_DBG_DISPATCH_STATUS_DATA,
    IDE_DBG_DISPATCH_STATUS_BUSY,
} ide_dbg_status_t;

enum frame_state {
    FRAME_HEAD,
    FRAME_CMD,
    FRAME_RECV,
    FRAME_DATA_LENGTH,
    FRAME_DISPATCH
};

typedef struct {
    enum frame_state state;
    enum usbdbg_cmd cmd;
    uint32_t data_length;
    void* data;
    // private
    uint32_t recv_lack;
    enum frame_state recv_next;
    void* recv_data;
    const uint8_t* frame_data;
    size_t frame_length;
    size_t frame_offset;
} ide_dbg_state_t;

void ide_dbg_start(void);
bool ide_dbg_attach(void);
bool ide_dbg_is_connected(void);
bool ide_dbg_is_script_running(void);
bool ide_dbg_has_script(void);
char *ide_dbg_get_script(void);
exec_type_t ide_dbg_get_exec_type(void);
void ide_dbg_on_script_start(void);
void ide_dbg_on_script_end(void);
void ide_dbg_on_ide_script_end(void);
void ide_dbg_on_soft_reset(void);
bool ide_dbg_take_soft_reset_request(void);
void ide_dbg_clear_soft_reset_request(void);
void ide_dbg_vo_wbc_stop(void);
void ide_dbg_vo_wbc_start(int enable);
void ide_dbg_interrupt(void);
void ide_dbg_stdout_tx(const char *data, size_t size);
bool ide_dbg_stdout_capture(const char *data, size_t size);
void ide_dbg_stdout_raw_input(void);
void interrupt_repl(void);
void ide_set_fb(const void* data, uint32_t size, uint32_t width, uint32_t height);
void ide_dbg_enable_vo_wbc(void);
void ide_dbg_vtouch_open(uint32_t range_x, uint32_t range_y);
void ide_dbg_vtouch_close(void);
bool ide_dbg_vtouch_is_enabled(void);
void ide_dbg_vtouch_clear(void);
uint32_t ide_dbg_vtouch_read(struct ide_dbg_vtouch_event *out, uint32_t max_events);

#else // !CONFIG_CANMV_IDE_SUPPORT

static inline void ide_dbg_start(void) {}
static inline bool ide_dbg_attach(void) { return false; }
static inline bool ide_dbg_is_connected(void) { return false; }
static inline bool ide_dbg_is_script_running(void) { return false; }
static inline bool ide_dbg_has_script(void) { return false; }
static inline char *ide_dbg_get_script(void) { return NULL; }
static inline exec_type_t ide_dbg_get_exec_type(void) { return EXEC_STRING; }
static inline void ide_dbg_on_script_start(void) {}
static inline void ide_dbg_on_script_end(void) {}
static inline void ide_dbg_on_ide_script_end(void) {}
static inline void ide_dbg_on_soft_reset(void) {}
static inline bool ide_dbg_take_soft_reset_request(void) { return false; }
static inline void ide_dbg_clear_soft_reset_request(void) {}
static inline void ide_dbg_vo_wbc_stop(void) {}
static inline void ide_dbg_vo_wbc_start(int enable) { (void)enable; }
static inline void ide_dbg_interrupt(void) {}
static inline void ide_dbg_stdout_tx(const char *data, size_t size) { (void)data; (void)size; }
static inline bool ide_dbg_stdout_capture(const char *data, size_t size) { (void)data; (void)size; return false; }
static inline void ide_dbg_stdout_raw_input(void) {}
static inline void interrupt_repl(void) {}
static inline void ide_set_fb(const void* data, uint32_t size, uint32_t width, uint32_t height) { (void)data; (void)size; (void)width; (void)height; }
static inline void ide_dbg_enable_vo_wbc(void) {}
static inline void ide_dbg_vtouch_open(uint32_t range_x, uint32_t range_y) { (void)range_x; (void)range_y; }
static inline void ide_dbg_vtouch_close(void) {}
static inline bool ide_dbg_vtouch_is_enabled(void) { return false; }
static inline void ide_dbg_vtouch_clear(void) {}
static inline uint32_t ide_dbg_vtouch_read(void *out, uint32_t max_events) { (void)out; (void)max_events; return 0; }

#endif // CONFIG_CANMV_IDE_SUPPORT

#endif /* __USBDBG_H__ */
