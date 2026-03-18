#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
CORPUS_DIR="${SCRIPT_DIR}/corpus"

log() {
    printf '[fuzz_test] %s\n' "$*"
}

detect_clang() {
    local c_var="$1"
    local cxx_var="$2"

    if [[ -n "${!c_var:-}" && -n "${!cxx_var:-}" ]]; then
        return
    fi

    local suffix=""
    for candidate in "" -18 -17 -16 -15 -14 -13 -12; do
        if command -v "clang${candidate}" >/dev/null 2>&1 && command -v "clang++${candidate}" >/dev/null 2>&1; then
            suffix="${candidate}"
            break
        fi
    done

    if [[ -z "${!c_var:-}" ]]; then
        export "${c_var}=clang${suffix}"
    fi
    if [[ -z "${!cxx_var:-}" ]]; then
        export "${cxx_var}=clang++${suffix}"
    fi
}

ensure_libstdcxx_link_path() {
    if [[ "${CXX}" != clang++* ]]; then
        return
    fi

    if [[ -e /usr/lib/x86_64-linux-gnu/libstdc++.so ]]; then
        return
    fi

    if ! command -v g++ >/dev/null 2>&1; then
        return
    fi

    local libstdcxx_path
    libstdcxx_path="$(g++ -print-file-name=libstdc++.so)"
    if [[ -z "${libstdcxx_path}" || "${libstdcxx_path}" == "libstdc++.so" ]]; then
        return
    fi

    local lib_dir
    lib_dir="$(dirname "${libstdcxx_path}")"
    if [[ "${LDFLAGS:-}" == *"${lib_dir}"* ]]; then
        return
    fi

    export LDFLAGS="${LDFLAGS:-} -L${lib_dir}"
}

ensure_clang_gcc_toolchain() {
    if [[ "${CXX}" != clang++* ]]; then
        return
    fi

    if [[ "${CXXFLAGS:-}" == *"--gcc-toolchain="* ]]; then
        return
    fi

    export CXXFLAGS="${CXXFLAGS:-} --gcc-toolchain=/usr"
}

require_tool() {
    local tool="$1"
    if ! command -v "$tool" >/dev/null 2>&1; then
        log "Missing required tool: ${tool}"
        exit 1
    fi
}

run_harness_sample() {
    local harness="$1"
    local name
    local corpus
    local asan_opts
    local rc

    name="$(basename "${harness}")"
    corpus="${CORPUS_DIR}/${name}"
    asan_opts="${ASAN_OPTIONS:-}"
    if [[ "${asan_opts}" != *"detect_leaks="* ]]; then
        asan_opts="${asan_opts:+${asan_opts}:}detect_leaks=0"
    fi

    log "Sample execution: ${name} (timed for 300 seconds)"
    if [[ -d "${corpus}" ]]; then
        log "Using corpus: ${corpus}"
        set +e
        ASAN_OPTIONS="${asan_opts}" "${harness}" -max_total_time=300 "${corpus}"
        rc=$?
        set -e
    else
        log "Corpus not found for ${name}; running without initial corpus"
        set +e
        ASAN_OPTIONS="${asan_opts}" "${harness}" -max_total_time=300
        rc=$?
        set -e
    fi

    if [[ ${rc} -eq 0 ]]; then
        log "Sample execution passed: ${name}"
    else
        log "Sample execution failed: ${name} (exit code ${rc})"
    fi
    return "${rc}"
}

main() {
    local harnesses=()
    local harness
    local failed_samples=0

    detect_clang CC CXX
    ensure_libstdcxx_link_path
    ensure_clang_gcc_toolchain

    require_tool cmake
    require_tool "${CC}"
    require_tool "${CXX}"
    require_tool bash
    require_tool find

    log "Generating source-wide fuzzing catalog"
    "${SCRIPT_DIR}/scripts/generate_fuzzing_catalog.sh"

    log "Removing previous build directory for a clean libFuzzer compile"
    rm -rf "${BUILD_DIR}"

    log "Configuring with CC=${CC} CXX=${CXX}"
    cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=RelWithDebInfo

    log "Building fuzz harnesses"
    cmake --build "${BUILD_DIR}" --clean-first -j"$(nproc)"

    log "Build complete. Harness binaries:"
    mapfile -t harnesses < <(find "${BUILD_DIR}" -maxdepth 1 -type f -name 'fuzz_*' -printf '%p\n' | sort)
    for harness in "${harnesses[@]}"; do
        printf '  %s\n' "$(basename "${harness}")"
    done

    if [[ ${#harnesses[@]} -eq 0 ]]; then
        log "No harness binaries found; skipping sample execution"
        return 0
    fi

    log "Starting sample execution for each harness (300 seconds each)"
    for harness in "${harnesses[@]}"; do
        if ! run_harness_sample "${harness}"; then
            failed_samples=1
        fi
    done

    if [[ ${failed_samples} -ne 0 ]]; then
        log "One or more harness sample executions failed"
        return 1
    fi

    log "All harness sample executions completed successfully"
}

main "$@"
