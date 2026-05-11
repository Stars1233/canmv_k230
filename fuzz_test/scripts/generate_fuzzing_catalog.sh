#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FUZZ_TEST_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
SOURCE_ROOT="$(cd "${FUZZ_TEST_DIR}/.." && pwd)"
OUT_FILE="${FUZZ_TEST_DIR}/ALL_SOURCE_FUZZING_SUGGESTIONS.md"

fuzz_focus_for() {
    local rel="$1"
    case "$rel" in
        *.py)
            echo "Parser and API-shape fuzzing on-device: import syntax, malformed literals, and boundary arguments." ;;
        *.h|*.hpp)
            echo "Header/interface fuzz triage: compile inclusion, macro boundary values, struct layout assumptions, and declaration drift." ;;
        port/omv/alloc/unaligned_memcpy.c)
            echo "Alignment-aware length and offset fuzzing to validate fast-path copies against edge lengths." ;;
        port/omv/common/array.c)
            echo "State-machine fuzzing over insert/remove/resize/sort operations with valid index constraints." ;;
        port/omv/common/ringbuf.c)
            echo "Producer/consumer sequence fuzzing across wraparound, empty reads, and full-buffer drops." ;;
        port/omv/common/ff_wrapper.c)
            echo "File I/O API fuzzing with short reads, truncation, seek boundaries, and persistence round-trips." ;;
        port/omv/alloc/*)
            echo "Allocation and buffer-boundary fuzzing with small, zero, and oversized lengths." ;;
        port/omv/common/*)
            echo "Host-side API sequence fuzzing for ownership, bounds, and state transitions." ;;
        port/omv/imlib/*)
            echo "Structured image payload fuzzing with malformed headers, dimensions, and algorithm edge cases." ;;
        port/multi_media/*)
            echo "Session/state fuzzing with malformed stream metadata and invalid sequencing." ;;
        port/modules/*|port/mp_modules/*|port/omv/modules/*)
            echo "Argument-parser fuzzing for MicroPython bindings with malformed object layouts." ;;
        port/builtin_py/mpp_binding/*)
            echo "MPP binding wrapper fuzzing with generated argument tuples, fake MPP return codes, and module-table consistency checks." ;;
        port/ai_cube/*|port/ai_demo/*|port/cv_lite/*|port/kpu/*)
            echo "Tensor metadata and model-wrapper fuzzing with malformed shapes, sizes, and buffer ownership." ;;
        port/machine/*)
            echo "Driver argument and state fuzzing with fake HAL/syscall backends for init/deinit/error paths." ;;
        resources/examples/*)
            echo "Example parser/import fuzzing plus dependency-availability annotations for board-only APIs." ;;
        resources/libs/*)
            echo "Library parser and API fuzzing with malformed values, short buffers, and MicroPython compatibility limits." ;;
        tools/*)
            echo "CLI/input fuzzing for malformed arguments, generated-output stability, and path edge cases." ;;
        *)
            echo "Compile-target triage first, then entry-point fuzzing around argument validation and error paths." ;;
    esac
}

priority_for() {
    local rel="$1"
    case "$rel" in
        port/omv/common/array.c|port/omv/common/ringbuf.c|port/omv/common/ff_wrapper.c|port/omv/alloc/unaligned_memcpy.c)
            echo "High" ;;
        port/omv/alloc/*|port/omv/common/*|port/machine/*|port/builtin_py/mpp_binding/*)
            echo "High" ;;
        *.py|*.h|*.hpp)
            echo "Medium" ;;
        *)
            echo "Low" ;;
    esac
}

harness_status_for() {
    local rel="$1"
    case "$rel" in
        port/omv/common/array.c)
            printf '%s\n' '`fuzz_array` (implemented, host-buildable)' ;;
        port/omv/common/ringbuf.c)
            printf '%s\n' '`fuzz_ringbuf` (implemented, host-buildable)' ;;
        port/omv/alloc/unaligned_memcpy.c)
            printf '%s\n' '`fuzz_unaligned_memcpy` (implemented, host-buildable)' ;;
        port/omv/common/ff_wrapper.c)
            printf '%s\n' '`fuzz_ff_wrapper` (implemented, host-buildable)' ;;
        port/ai_demo/ai_demo.c|port/cv_lite/cv_lite.c|port/ai_cube/ai_cube.c|port/machine/machine_wdt.c|port/ai_demo/tts_zh/tts_zh_preprocess.cpp)
            printf '%s\n' '`fuzz_recent_fixes_guardrails` (implemented, source-regression guardrails)' ;;
        *.h|*.hpp)
            printf '%s\n' 'Catalogued only; interface fuzzing requires compile or generated-value harnesses' ;;
        *.py)
            printf '%s\n' 'Catalogued only; requires MicroPython or parser-specific harness' ;;
        *)
            printf '%s\n' 'Catalogued only; hardware/runtime dependencies need extra shims' ;;
    esac
}

mapfile -t sources < <(
    find "${SOURCE_ROOT}" \
        \( -path "${SOURCE_ROOT}/unit_test" -o -path "${FUZZ_TEST_DIR}" \) -prune -o \
        -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' -o -name '*.py' \) -print | sort
)

count_c=0
count_cc=0
count_cpp=0
count_h=0
count_hpp=0
count_py=0

for src in "${sources[@]}"; do
    case "$src" in
        *.c) ((++count_c)) ;;
        *.cc) ((++count_cc)) ;;
        *.cpp) ((++count_cpp)) ;;
        *.h) ((++count_h)) ;;
        *.hpp) ((++count_hpp)) ;;
        *.py) ((++count_py)) ;;
    esac
done

{
    echo "# CANMV Source-Wide Fuzzing Suggestions"
    echo
    echo "Auto-generated on $(date -u +'%Y-%m-%d %H:%M:%S UTC')."
    echo
    printf 'Root scanned: `%s`\n' "${SOURCE_ROOT}"
    echo
    echo "Excluded from scan:"
    printf '%s\n' '- `unit_test/`'
    printf '%s\n' '- `fuzz_test/`'
    echo
    echo "| Source File | Suggested Fuzz Focus | Priority | Harness Status |"
    echo "| --- | --- | --- | --- |"

    for src in "${sources[@]}"; do
        rel="${src#${SOURCE_ROOT}/}"
        focus="$(fuzz_focus_for "${rel}")"
        priority="$(priority_for "${rel}")"
        status="$(harness_status_for "${rel}")"
        printf '| `%s` | %s | %s | %s |\n' "${rel}" "${focus}" "${priority}" "${status}"
    done

    echo
    echo "Total source files covered: ${#sources[@]}"
    echo "Language breakdown: C=${count_c}, CC=${count_cc}, C++=${count_cpp}, H=${count_h}, HPP=${count_hpp}, Python=${count_py}"
    echo
    echo "## Implemented libFuzzer harnesses"
    printf '%s\n' '- `fuzz_array` for `port/omv/common/array.c`'
    printf '%s\n' '- `fuzz_ringbuf` for `port/omv/common/ringbuf.c`'
    printf '%s\n' '- `fuzz_unaligned_memcpy` for `port/omv/alloc/unaligned_memcpy.c`'
    printf '%s\n' '- `fuzz_ff_wrapper` for `port/omv/common/ff_wrapper.c`'
    printf '%s\n' '- `fuzz_recent_fixes_guardrails` for post-fix invariants in `port/ai_demo/ai_demo.c`, `port/cv_lite/cv_lite.c`, `port/ai_cube/ai_cube.c`, `port/machine/machine_wdt.c`, and `port/ai_demo/tts_zh/tts_zh_preprocess.cpp`'
    echo
    echo "## Next expansion order"
    printf '%s\n' '1. Remaining `port/omv/common/*` and `port/omv/alloc/*` files with existing host shims'
    printf '%s\n' '2. `port/omv/imlib/*` entry points behind small image-format seed corpora'
    printf '%s\n' '3. `port/modules/*`, `port/mp_modules/*`, and `port/builtin_py/mpp_binding/*` argument-shape fuzzers with fake MicroPython objects'
    echo "4. Python parser/import fuzzing through a host MicroPython build"
    echo "5. HAL-bound modules once dedicated fake backends exist"
} > "${OUT_FILE}"
