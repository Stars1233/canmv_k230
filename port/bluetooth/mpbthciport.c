/*
 * SPDX-FileCopyrightText: 2020 Jim Mussared
 * SPDX-License-Identifier: MIT
 */
#include "py/runtime.h"
#include "py/mphal.h"

#if MICROPY_PY_BLUETOOTH && MICROPY_BLUETOOTH_NIMBLE

#if !MICROPY_PY_THREAD
#error The RT-Smart HCI port requires MICROPY_PY_THREAD
#endif

#include "extmod/modbluetooth.h"
#include "extmod/mpbthci.h"
#include "drv_hci.h"

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

uint8_t mp_bluetooth_hci_cmd_buf[4 + 256];

STATIC drv_hci_inst_t *hci_inst;
STATIC pthread_t hci_poll_thread_id;
STATIC bool hci_poll_thread_started;

extern bool mp_bluetooth_hci_poll(void);
extern bool mp_bluetooth_hci_active(void);

#if MICROPY_PY_BLUETOOTH_USE_SYNC_EVENTS
STATIC volatile bool events_task_is_scheduled;

STATIC mp_obj_t run_events_scheduled_task(mp_obj_t none_in) {
    (void)none_in;
    MICROPY_PY_BLUETOOTH_ENTER
    events_task_is_scheduled = false;
    MICROPY_PY_BLUETOOTH_EXIT
    mp_bluetooth_hci_poll();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(run_events_scheduled_task_obj, run_events_scheduled_task);
#endif

STATIC void *hci_poll_thread(void *argument) {
    (void)argument;

    #if MICROPY_PY_BLUETOOTH_USE_SYNC_EVENTS
    events_task_is_scheduled = false;
    while (mp_bluetooth_hci_active()) {
        MICROPY_PY_BLUETOOTH_ENTER
        if (!events_task_is_scheduled) {
            events_task_is_scheduled = mp_sched_schedule(
                MP_OBJ_FROM_PTR(&run_events_scheduled_task_obj), mp_const_none);
        }
        MICROPY_PY_BLUETOOTH_EXIT
        usleep(1000);
    }
    #else
    while (mp_bluetooth_hci_poll()) {
        usleep(1000);
    }
    #endif

    return NULL;
}

int mp_bluetooth_hci_uart_init(uint32_t port, uint32_t baudrate) {
    int error;

    (void)port;
    (void)baudrate;
    if (hci_inst) {
        return 0;
    }

    error = drv_hci_inst_create_auto(&hci_inst, NULL, 0);
    if (error != 0) {
        printf("bluetooth: cannot open an HCI device: %d\n", error);
        return -1;
    }

    error = pthread_create(&hci_poll_thread_id, NULL, hci_poll_thread, NULL);
    if (error != 0) {
        drv_hci_inst_destroy(&hci_inst);
        return -1;
    }
    hci_poll_thread_started = true;
    return 0;
}

int mp_bluetooth_hci_uart_deinit(void) {
    if (!hci_inst) {
        return 0;
    }

    if (hci_poll_thread_started) {
        pthread_join(hci_poll_thread_id, NULL);
        hci_poll_thread_started = false;
    }
    drv_hci_inst_destroy(&hci_inst);
    return 0;
}

int mp_bluetooth_hci_uart_set_baudrate(uint32_t baudrate) {
    // The selected HCI device is logical, so UART baud rate does not apply.
    (void)baudrate;
    return 0;
}

int mp_bluetooth_hci_uart_any(void) {
    return drv_hci_poll(hci_inst, 0) > 0;
}

int mp_bluetooth_hci_uart_readchar(void) {
    uint8_t byte;
    ssize_t length;

    if (!hci_inst) {
        return -1;
    }
    length = drv_hci_read(hci_inst, &byte, 1);
    return length == 1 ? byte : -1;
}

int mp_bluetooth_hci_uart_readpacket(mp_bluetooth_hci_uart_readchar_t handler) {
    uint8_t buffer[512];
    ssize_t length;

    if (!hci_inst || !handler) {
        return -1;
    }
    length = drv_hci_read(hci_inst, buffer, sizeof(buffer));
    if (length <= 0) {
        return -1;
    }
    for (ssize_t index = 0; index < length; ++index) {
        handler(buffer[index]);
    }
    return length;
}

int mp_bluetooth_hci_uart_write(const uint8_t *buffer, size_t length) {
    if (!hci_inst || !buffer || !length) {
        return -1;
    }
    return drv_hci_write_packet(hci_inst, buffer, length, 1000);
}

int mp_bluetooth_hci_controller_init(void) {
    // Opening the selected HCI device starts its controller.
    return 0;
}

int mp_bluetooth_hci_controller_deinit(void) {
    // Closing the selected HCI device stops its controller.
    return 0;
}

int mp_bluetooth_hci_controller_sleep_maybe(void) {
    return 0;
}

bool mp_bluetooth_hci_controller_woken(void) {
    return true;
}

int mp_bluetooth_hci_controller_wakeup(void) {
    return 0;
}

#endif /* MICROPY_PY_BLUETOOTH && MICROPY_BLUETOOTH_NIMBLE */
