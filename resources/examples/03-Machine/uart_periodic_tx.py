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
# True: request immediate transmission whenever a new frame is published.
SEND_NOW_ON_UPDATE = True

FRAME_HEADER = b"\xA5\x5A"
FRAME_TRAILER = 0x0D
FRAME_OVERHEAD = 6
MIN_PAYLOAD_LEN = 1
MAX_PAYLOAD_LEN = 16
MIN_FRAME_LEN = FRAME_OVERHEAD + MIN_PAYLOAD_LEN
MAX_FRAME_LEN = FRAME_OVERHEAD + MAX_PAYLOAD_LEN
# Loopback test pattern: payload byte n is (sequence + n) & 0xFF.


def make_frame(sequence):
    payload_len = MIN_PAYLOAD_LEN + sequence % (
        MAX_PAYLOAD_LEN - MIN_PAYLOAD_LEN + 1
    )
    frame_len = FRAME_OVERHEAD + payload_len
    frame = bytearray(frame_len)
    frame[0] = FRAME_HEADER[0]
    frame[1] = FRAME_HEADER[1]
    frame[2] = frame_len
    frame[3] = sequence
    for index in range(payload_len):
        frame[4 + index] = (sequence + index) & 0xFF

    checksum = 0
    for index in range(frame_len - 2):
        checksum ^= frame[index]
    frame[-2] = checksum
    frame[-1] = FRAME_TRAILER
    return bytes(frame)


def valid_frame(frame):
    """Validate the frame and the loopback test's payload pattern."""
    frame_len = len(frame)
    if frame_len < MIN_FRAME_LEN or frame_len > MAX_FRAME_LEN:
        return False
    if frame[0:2] != FRAME_HEADER or frame[2] != frame_len:
        return False
    if frame[-1] != FRAME_TRAILER:
        return False

    payload_len = frame_len - FRAME_OVERHEAD
    for index in range(payload_len):
        if frame[4 + index] != ((frame[3] + index) & 0xFF):
            return False

    checksum = 0
    for index in range(frame_len - 2):
        checksum ^= frame[index]
    return frame[-2] == checksum


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
        elif frame_len == 2:
            if MIN_FRAME_LEN <= byte <= MAX_FRAME_LEN:
                frame_buffer[2] = byte
                frame_len = 3
            elif byte == FRAME_HEADER[0]:
                frame_buffer[0] = byte
                frame_len = 1
            else:
                frame_len = 0
        else:
            frame_buffer[frame_len] = byte
            frame_len += 1
            if frame_len == frame_buffer[2]:
                frames.append(bytes(frame_buffer[:frame_len]))
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
    immediate_requests = 0
    immediate_submitted = 0
    sent_state_errors = 0
    interval_count = 0
    interval_total_us = 0
    interval_min_us = None
    interval_max_us = None
    last_receive_us = None
    received_min_len = None
    received_max_len = None
    rx_frame = bytearray(MAX_FRAME_LEN)
    rx_frame_len = 0

    try:
        uart = configure_uart()
        transmitter = UARTPeriodicTx(
            UART.UART3,
            TIMER_ID,
            PERIOD_MS,
            max_len=MAX_FRAME_LEN,
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
            (
                "UARTPeriodicTx loopback test: period={} ms, duration={} ms, "
                "repeat_last={}, send_now={}"
            ).format(
                PERIOD_MS, TEST_DURATION_MS, REPEAT_LAST, SEND_NOW_ON_UPDATE
            )
        )

        while time.ticks_diff(time.ticks_ms(), start_ms) < TEST_DURATION_MS:
            now_ms = time.ticks_ms()
            if time.ticks_diff(now_ms, next_update_ms) >= 0:
                sequence = (sequence + 1) & 0xFF
                sent_now = transmitter.update(
                    make_frame(sequence), send_now=SEND_NOW_ON_UPDATE
                )
                if SEND_NOW_ON_UPDATE:
                    immediate_requests += 1
                    if sent_now:
                        immediate_submitted += 1
                        if not transmitter.is_sent():
                            sent_state_errors += 1
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
                    frame_len = len(frame)
                    if received_min_len is None or frame_len < received_min_len:
                        received_min_len = frame_len
                    if received_max_len is None or frame_len > received_max_len:
                        received_max_len = frame_len

            time.sleep_ms(1)

        sent, short_write, errors, skipped = transmitter.stats()
        latest_sent = transmitter.is_sent()
        last_error = transmitter.last_error()
        print("frames: sent={}, received={}, invalid={}".format(sent, received, invalid))
        print(
            "frame length: min={}, max={}".format(
                received_min_len, received_max_len
            )
        )
        print(
            "immediate: submitted={}, requested={}, state_errors={}".format(
                immediate_submitted, immediate_requests, sent_state_errors
            )
        )
        print("latest packet submitted={}".format(latest_sent))
        print(
            "tx stats: short_write={}, errors={}, skipped={}, last_error={}".format(
                short_write, errors, skipped, last_error
            )
        )
        if interval_count:
            print(
                "rx interval us: avg={}, min={}, max={}".format(
                    interval_total_us // interval_count,
                    interval_min_us,
                    interval_max_us,
                )
            )

        expected = TEST_DURATION_MS // (PERIOD_MS if REPEAT_LAST else UPDATE_PERIOD_MS)
        expected_skipped = 0 if REPEAT_LAST and not SEND_NOW_ON_UPDATE else None
        immediate_ok = not SEND_NOW_ON_UPDATE or immediate_submitted >= immediate_requests - 3
        dynamic_lengths_ok = (
            received_min_len is not None
            and received_max_len is not None
            and received_min_len < received_max_len
        )
        if (
            received >= max(expected, sent) - 3
            and invalid == 0
            and short_write == 0
            and errors == 0
            and last_error == 0
            and immediate_ok
            and sent_state_errors == 0
            and dynamic_lengths_ok
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
