"""UARTPeriodicTx UART3 loopback test.

This example requires CONFIG_ENABLE_MODULE_UART_PERIODIC_TX=y.
Connect IO50 (UART3_TXD) to IO51 (UART3_RXD) before running it. The receive
timestamps are a software sanity check; use a logic analyzer on IO50 when
measuring wire-level timing.
"""

from machine import FPIOA, UART
from uart_periodic_tx import UARTPeriodicTx
import time


UART_TX_PIN = 50
UART_RX_PIN = 51
TIMER_ID = 0
BAUDRATE = 115200
PERIOD_MS = 50
TEST_DURATION_MS = 5000
UPDATE_PERIOD_MS = 50
# True: send the most recently published frame on every timer tick.
# False: send each successfully updated frame once, then skip until update().
REPEAT_LAST = True

FRAME_HEADER = b"\xA5\x5A"
FRAME_TRAILER = 0x0D
FRAME_LEN = 8


def make_frame(sequence):
    inverse = sequence ^ 0xFF
    checksum = 0xA5 ^ 0x5A ^ sequence ^ inverse ^ 0x3C ^ 0xC3
    return bytes((0xA5, 0x5A, sequence, inverse, 0x3C, 0xC3, checksum, FRAME_TRAILER))


def valid_frame(frame):
    if len(frame) != FRAME_LEN:
        return False
    if frame[0:2] != FRAME_HEADER or frame[7] != FRAME_TRAILER:
        return False
    if frame[3] != (frame[2] ^ 0xFF):
        return False
    return frame[6] == (frame[0] ^ frame[1] ^ frame[2] ^ frame[3] ^ frame[4] ^ frame[5])


def collect_frames(data, frame_buffer, frame_len):
    """Return complete frames without deleting from a bytearray."""
    frames = []

    for byte in data:
        if frame_len == 0:
            if byte == FRAME_HEADER[0]:
                frame_buffer[0] = byte
                frame_len = 1
        elif frame_len == 1:
            if byte == FRAME_HEADER[1]:
                frame_buffer[1] = byte
                frame_len = 2
            elif byte == FRAME_HEADER[0]:
                frame_buffer[0] = byte
            else:
                frame_len = 0
        else:
            frame_buffer[frame_len] = byte
            frame_len += 1
            if frame_len == FRAME_LEN:
                frames.append(bytes(frame_buffer))
                frame_len = 0

    return frames, frame_len


def configure_uart():
    fpioa = FPIOA()
    fpioa.set_function(UART_TX_PIN, FPIOA.UART3_TXD)
    fpioa.set_function(UART_RX_PIN, FPIOA.UART3_RXD)
    return UART(
        UART.UART3,
        baudrate=BAUDRATE,
        bits=UART.EIGHTBITS,
        parity=UART.PARITY_NONE,
        stop=UART.STOPBITS_ONE,
        timeout=0,
    )


def run_test():
    uart = None
    transmitter = None
    received = 0
    invalid = 0
    interval_count = 0
    interval_total_us = 0
    interval_min_us = None
    interval_max_us = None
    last_receive_us = None
    rx_frame = bytearray(FRAME_LEN)
    rx_frame_len = 0

    try:
        uart = configure_uart()
        transmitter = UARTPeriodicTx(
            UART.UART3,
            TIMER_ID,
            PERIOD_MS,
            max_len=FRAME_LEN,
            baudrate=BAUDRATE,
            bits=UART.EIGHTBITS,
            parity=UART.PARITY_NONE,
            stop=UART.STOPBITS_ONE,
            repeat_last=REPEAT_LAST,
        )

        sequence = 0
        transmitter.update(make_frame(sequence))
        transmitter.start()

        start_ms = time.ticks_ms()
        next_update_ms = time.ticks_add(start_ms, UPDATE_PERIOD_MS)
        print(
            "UARTPeriodicTx loopback test: period={} ms, duration={} ms, repeat_last={}".format(
                PERIOD_MS, TEST_DURATION_MS, REPEAT_LAST
            )
        )

        while time.ticks_diff(time.ticks_ms(), start_ms) < TEST_DURATION_MS:
            now_ms = time.ticks_ms()
            if time.ticks_diff(now_ms, next_update_ms) >= 0:
                sequence = (sequence + 1) & 0xFF
                transmitter.update(make_frame(sequence))
                next_update_ms = time.ticks_add(next_update_ms, UPDATE_PERIOD_MS)

            data = uart.read()
            if data:
                frames, rx_frame_len = collect_frames(data, rx_frame, rx_frame_len)
                for frame in frames:
                    if not valid_frame(frame):
                        invalid += 1
                        continue
                    now_us = time.ticks_us()
                    if last_receive_us is not None:
                        interval_us = time.ticks_diff(now_us, last_receive_us)
                        if interval_us > 0:
                            interval_count += 1
                            interval_total_us += interval_us
                            if interval_min_us is None or interval_us < interval_min_us:
                                interval_min_us = interval_us
                            if interval_max_us is None or interval_us > interval_max_us:
                                interval_max_us = interval_us
                    last_receive_us = now_us
                    received += 1

            time.sleep_ms(1)

        sent, short_write, errors, skipped = transmitter.stats()
        print("frames: sent={}, received={}, invalid={}".format(sent, received, invalid))
        print("tx stats: short_write={}, errors={}, skipped={}".format(short_write, errors, skipped))
        if interval_count:
            print(
                "rx interval us: avg={}, min={}, max={}".format(
                    interval_total_us // interval_count,
                    interval_min_us,
                    interval_max_us,
                )
            )

        expected = TEST_DURATION_MS // (PERIOD_MS if REPEAT_LAST else UPDATE_PERIOD_MS)
        expected_skipped = 0 if REPEAT_LAST else None
        if (
            received >= expected - 3
            and invalid == 0
            and short_write == 0
            and errors == 0
            and (expected_skipped is None or skipped == expected_skipped)
        ):
            print("PASS")
        else:
            print("FAIL: verify the IO50-to-IO51 loopback wire and UART configuration")
    finally:
        if transmitter is not None:
            transmitter.deinit()
        if uart is not None:
            uart.deinit()


run_test()
