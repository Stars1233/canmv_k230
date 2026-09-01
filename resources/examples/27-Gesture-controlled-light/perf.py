import time
import gc


class Perf:
    def __init__(self):
        self.fps = 0.0
        self.cpu = 0.0
        self.infer_ms = 0.0
        self.lat_ms = 0.0
        self.mem_pct = 0.0
        self.mem_free_kb = 0
        self.dt = 66
        self._t0 = time.ticks_ms()
        self._prev = self._t0

    @staticmethod
    # Exponential Moving Average.
    def _ema(old, new, k=0.25):
        return old + (new - old) * k

    def begin(self):
        # called at the very top of the loop
        self._t0 = time.ticks_ms()
        loop = time.ticks_diff(self._t0, self._prev)
        self._prev = self._t0

        # update telemetry values with EMA smoothing
        if loop > 0:
            self.dt = loop
            self.fps = self._ema(self.fps, 1000.0 / loop)
            self.lat_ms = self._ema(self.lat_ms, loop)
        return self._t0

    def end(self, infer_ms):
        # called near the end of the loop to measure inference time, CPU usage and memory

        work = time.ticks_diff(time.ticks_ms(), self._t0)

        # cpu usage
        if self.lat_ms > 1:
            self.cpu = self._ema(self.cpu, min(100.0, 100.0 * work / self.lat_ms))

        # inference time
        self.infer_ms = self._ema(self.infer_ms, infer_ms)

        # memory
        free = gc.mem_free()
        alloc = gc.mem_alloc()
        total = free + alloc
        self.mem_free_kb = free // 1024
        if total > 0:
            self.mem_pct = self._ema(self.mem_pct, 100.0 * alloc / total)
