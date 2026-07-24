import _thread
import gc
import time


SHORT_WORKER_COUNT = 4
SHORT_WORKER_ROUNDS = 80
PERF_THREAD_COUNTS = (1, 2, 4)
CPU_ROUNDS_PER_THREAD = 1200000
LOCK_ROUNDS_PER_THREAD = 20000
SLEEP_ROUNDS_PER_THREAD = 100
SLEEP_INTERVAL_MS = 5
NO_SLEEP_TEST_MS = 2000
NO_SLEEP_PROBE_MS = 10
LONG_WORKER_COUNT = 3

stats_lock = _thread.allocate_lock()
short_workers_done = 0
heartbeats = [0] * LONG_WORKER_COUNT
perf_ready = 0
perf_done = 0
perf_start = False
perf_elapsed_ms = [0] * max(PERF_THREAD_COUNTS)
perf_checksums = [0] * max(PERF_THREAD_COUNTS)
lock_perf_counter = 0
no_sleep_start = False
no_sleep_stop = False
no_sleep_ready = 0
no_sleep_done = 0
no_sleep_iterations = [0] * max(PERF_THREAD_COUNTS)


def short_worker(worker_id, rounds):
    global short_workers_done

    checksum = 0
    for sequence in range(rounds):
        payload = bytearray(128)
        payload[0] = (worker_id + sequence) & 0xFF
        checksum += payload[0]
        if sequence % 8 == 0:
            gc.collect()
        time.sleep_ms(1)

    with stats_lock:
        short_workers_done += 1
    print("short worker", worker_id, "done, checksum", checksum)


def perf_worker_begin():
    global perf_ready

    with stats_lock:
        perf_ready += 1

    while not perf_start:
        time.sleep_ms(1)


def perf_worker_end(worker_id, started_ms, checksum):
    global perf_done

    elapsed_ms = time.ticks_diff(time.ticks_ms(), started_ms)
    with stats_lock:
        perf_elapsed_ms[worker_id] = elapsed_ms
        perf_checksums[worker_id] = checksum
        perf_done += 1


def cpu_perf_worker(worker_id, rounds):
    perf_worker_begin()
    started_ms = time.ticks_ms()
    value = worker_id + 1

    for sequence in range(rounds):
        value = (value * 33 + sequence) & 0x7FFFFF

    perf_worker_end(worker_id, started_ms, value)


def lock_perf_worker(worker_id, rounds):
    global lock_perf_counter

    perf_worker_begin()
    started_ms = time.ticks_ms()

    for _ in range(rounds):
        with stats_lock:
            lock_perf_counter += 1

    perf_worker_end(worker_id, started_ms, rounds + worker_id)


def sleep_perf_worker(worker_id, rounds):
    perf_worker_begin()
    started_ms = time.ticks_ms()

    for _ in range(rounds):
        time.sleep_ms(SLEEP_INTERVAL_MS)

    perf_worker_end(worker_id, started_ms, rounds + worker_id)


def wait_perf_state(field_name, target, timeout_ms):
    deadline = time.ticks_add(time.ticks_ms(), timeout_ms)

    while True:
        with stats_lock:
            value = perf_ready if field_name == "ready" else perf_done

        if value == target:
            return
        if time.ticks_diff(deadline, time.ticks_ms()) <= 0:
            raise RuntimeError("perf {} timeout: {}/{}".format(field_name, value, target))
        time.sleep_ms(1)


def run_perf_case(name, worker, rounds, thread_count, check_lock=False):
    global perf_ready, perf_done, perf_start

    with stats_lock:
        perf_ready = 0
        perf_done = 0
        perf_start = False
        for worker_id in range(thread_count):
            perf_elapsed_ms[worker_id] = 0
            perf_checksums[worker_id] = 0

    for worker_id in range(thread_count):
        _thread.start_new_thread(worker, (worker_id, rounds))

    wait_perf_state("ready", thread_count, 10000)
    wall_started_ms = time.ticks_ms()
    perf_start = True
    wait_perf_state("done", thread_count, 30000)
    wall_ms = max(1, time.ticks_diff(time.ticks_ms(), wall_started_ms))

    with stats_lock:
        worker_times = tuple(perf_elapsed_ms[:thread_count])
        checksum = sum(perf_checksums[:thread_count])

    rate = thread_count * rounds * 1000 // wall_ms
    print("{} {} thread(s): {} iter/s, wall {} ms, workers {}..{} ms, sum {}".format(
        name, thread_count, rate, wall_ms, min(worker_times), max(worker_times), checksum))
    if check_lock and lock_perf_counter != thread_count * rounds:
        raise RuntimeError("lock counter mismatch: {}".format(lock_perf_counter))


def no_sleep_worker(worker_id):
    global no_sleep_ready, no_sleep_done

    with stats_lock:
        no_sleep_ready += 1

    while not no_sleep_start:
        time.sleep_ms(1)

    sequence = 0
    value = worker_id + 1
    while not no_sleep_stop:
        value = (value * 33 + sequence) & 0x7FFFFF
        sequence += 1

    with stats_lock:
        no_sleep_iterations[worker_id] = sequence
        no_sleep_done += 1


def wait_no_sleep_state(field_name, target, timeout_ms):
    deadline = time.ticks_add(time.ticks_ms(), timeout_ms)

    while True:
        with stats_lock:
            value = no_sleep_ready if field_name == "ready" else no_sleep_done

        if value == target:
            return
        if time.ticks_diff(deadline, time.ticks_ms()) <= 0:
            raise RuntimeError("no-sleep {} timeout: {}/{}".format(field_name, value, target))
        time.sleep_ms(1)


def run_no_sleep_case(thread_count):
    global no_sleep_start, no_sleep_stop, no_sleep_ready, no_sleep_done

    with stats_lock:
        no_sleep_start = False
        no_sleep_stop = False
        no_sleep_ready = 0
        no_sleep_done = 0
        for worker_id in range(thread_count):
            no_sleep_iterations[worker_id] = 0

    for worker_id in range(thread_count):
        _thread.start_new_thread(no_sleep_worker, (worker_id,))

    wait_no_sleep_state("ready", thread_count, 10000)

    started_ms = time.ticks_ms()
    previous_us = time.ticks_us()
    deadline = time.ticks_add(started_ms, NO_SLEEP_TEST_MS)
    max_gap_us = 0
    total_gap_us = 0
    probe_count = 0
    no_sleep_start = True

    while time.ticks_diff(deadline, time.ticks_ms()) > 0:
        time.sleep_ms(NO_SLEEP_PROBE_MS)
        current_us = time.ticks_us()
        gap_us = time.ticks_diff(current_us, previous_us)
        previous_us = current_us
        max_gap_us = max(max_gap_us, gap_us)
        total_gap_us += gap_us
        probe_count += 1

    no_sleep_stop = True
    wait_no_sleep_state("done", thread_count, 10000)
    wall_ms = max(1, time.ticks_diff(time.ticks_ms(), started_ms))

    with stats_lock:
        counts = tuple(no_sleep_iterations[:thread_count])

    total_iterations = sum(counts)
    rate = total_iterations * 1000 // wall_ms
    average_gap_us = total_gap_us // max(1, probe_count)
    late_us = max(0, max_gap_us - NO_SLEEP_PROBE_MS * 1000)
    print("busy {} thread(s): {} iter/s, counts {}..{}, probe avg {} us, max {} us, late {} us".format(
        thread_count, rate, min(counts), max(counts), average_gap_us, max_gap_us, late_us))


def endless_no_sleep_worker(worker_id):
    sequence = 0
    value = worker_id + 1

    while True:
        value = (value * 33 + sequence) & 0x7FFFFF
        sequence += 1
        if sequence & 0x3FFF == 0:
            with stats_lock:
                heartbeats[worker_id] = sequence


def long_worker(worker_id):
    sequence = 0
    live_objects = []

    while True:
        payload = bytearray(256)
        payload[0] = sequence & 0xFF
        live_objects.append((sequence, payload))
        if len(live_objects) > 8:
            live_objects.pop(0)

        with stats_lock:
            heartbeats[worker_id] = sequence

        if sequence % 32 == 0:
            gc.collect()

        sequence += 1
        time.sleep_ms(5 + worker_id)


print("thread port regression test")
print("phase 1: create and finish short-lived detached threads")

for worker_id in range(SHORT_WORKER_COUNT):
    _thread.start_new_thread(short_worker, (worker_id, SHORT_WORKER_ROUNDS))

deadline = time.ticks_add(time.ticks_ms(), 10000)
while True:
    with stats_lock:
        done = short_workers_done

    if done == SHORT_WORKER_COUNT:
        break
    if time.ticks_diff(deadline, time.ticks_ms()) <= 0:
        raise RuntimeError("short worker timeout: {}/{}".format(done, SHORT_WORKER_COUNT))

    gc.collect()
    time.sleep_ms(10)

print("phase 1 passed")
print("phase 2: compare multi-thread throughput")
print("CPU throughput is expected to flatten when the GIL is the bottleneck")
for thread_count in PERF_THREAD_COUNTS:
    run_perf_case("cpu", cpu_perf_worker, CPU_ROUNDS_PER_THREAD, thread_count)

print("lock throughput measures shared mutex contention")
for thread_count in PERF_THREAD_COUNTS:
    lock_perf_counter = 0
    run_perf_case("lock", lock_perf_worker, LOCK_ROUNDS_PER_THREAD,
                  thread_count, check_lock=True)

print("sleep throughput should scale because sleeping releases execution time")
for thread_count in PERF_THREAD_COUNTS:
    run_perf_case("sleep", sleep_perf_worker, SLEEP_ROUNDS_PER_THREAD, thread_count)

print("phase 3: measure scheduling with workers that never sleep")
for thread_count in PERF_THREAD_COUNTS:
    run_no_sleep_case(thread_count)

print("phase 4: run workers during repeated garbage collection")
print("worker 0 never sleeps; all heartbeats must advance")

_thread.start_new_thread(endless_no_sleep_worker, (0,))
for worker_id in range(1, LONG_WORKER_COUNT):
    _thread.start_new_thread(long_worker, (worker_id,))

try:
    while True:
        gc.collect()

        with stats_lock:
            snapshot = tuple(heartbeats)

        print("heartbeats:", snapshot, "free heap:", gc.mem_free())
        time.sleep(1)
except KeyboardInterrupt:
    print("main stopped; worker threads are intentionally still running")

# Do not add os.exitpoint() or a worker stop flag here. Exiting this script with
# active workers verifies that the port injects SystemExit, waits for every
# detached worker, and completes VM cleanup without hanging or crashing.
