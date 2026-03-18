This directory contains starter seed corpora for the libFuzzer targets in `fuzz_test/`.

Layout:
- `fuzz_array/`: digit-heavy sequences where ASCII `0`..`7` map cleanly to the array harness operations via `% 8`.
- `fuzz_ringbuf/`: even/odd ASCII bytes to exercise put/get balance, burst writes, and draining patterns.
- `fuzz_unaligned_memcpy/`: short and triplet-oriented inputs to hit both the `< 3` fast return and multi-copy paths.
- `fuzz_ff_wrapper/`: small file payloads that vary seek/truncate positions and the typed read/write helpers.

Example:
`ASAN_OPTIONS=detect_leaks=0 ./build/fuzz_array corpus/fuzz_array/*`
