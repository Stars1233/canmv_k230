#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
CORPUS_DIR="${SCRIPT_DIR}/corpus"
FUZZ_MAX_TOTAL_TIME="${FUZZ_MAX_TOTAL_TIME:-30}"
FUZZ_RUNS="${FUZZ_RUNS:-}"
FUZZ_SMOKE_ONLY="${FUZZ_SMOKE_ONLY:-1}"

log() {
    printf '[fuzz_test] %s\n' "$*"
}

run_privileged() {
    if [[ "${EUID}" -eq 0 ]]; then
        "$@"
        return
    fi

    if command -v sudo >/dev/null 2>&1; then
        sudo "$@"
        return
    fi

    log "Command requires elevated privilege, but sudo is unavailable: $*"
    exit 1
}

install_clang() {
    if command -v apt-get >/dev/null 2>&1; then
        log "clang not found. Installing clang with apt-get"
        run_privileged apt-get update
        run_privileged apt-get install -y clang
        return
    fi

    log "clang not found, and apt-get is unavailable. Please install clang and clang++."
    exit 1
}

detect_clang() {
    local c_var="$1"
    local cxx_var="$2"

    if [[ -n "${!c_var:-}" && -n "${!cxx_var:-}" ]]; then
        return
    fi

    local suffix=""
    local found=0
    for candidate in "" -18 -17 -16 -15 -14 -13 -12; do
        if command -v "clang${candidate}" >/dev/null 2>&1 && command -v "clang++${candidate}" >/dev/null 2>&1; then
            suffix="${candidate}"
            found=1
            break
        fi
    done

    if [[ ${found} -eq 0 ]]; then
        install_clang
        for candidate in "" -18 -17 -16 -15 -14 -13 -12; do
            if command -v "clang${candidate}" >/dev/null 2>&1 && command -v "clang++${candidate}" >/dev/null 2>&1; then
                suffix="${candidate}"
                found=1
                break
            fi
        done
    fi

    if [[ ${found} -eq 0 ]]; then
        log "clang installation did not provide both clang and clang++"
        exit 1
    fi

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
    local attempt
    local attempt_log

    name="$(basename "${harness}")"
    corpus="${CORPUS_DIR}/${name}"
    asan_opts="${ASAN_OPTIONS:-}"
    if [[ "${asan_opts}" != *"detect_leaks="* ]]; then
        asan_opts="${asan_opts:+${asan_opts}:}detect_leaks=0"
    fi

    if [[ "${FUZZ_SMOKE_ONLY}" == "1" ]]; then
        log "Sample execution: ${name} (replaying checked-in corpus only)"
    elif [[ -n "${FUZZ_RUNS}" ]]; then
        log "Sample execution: ${name} (${FUZZ_RUNS} runs)"
    else
        log "Sample execution: ${name} (timed for ${FUZZ_MAX_TOTAL_TIME} seconds)"
    fi
    if [[ -d "${corpus}" ]]; then
        log "Using corpus: ${corpus}"
        set +e
        if [[ "${FUZZ_SMOKE_ONLY}" == "1" ]]; then
            rc=1
            for attempt in 1 2 3 4 5; do
                attempt_log="$(mktemp -t "canmv_${name}_smoke_XXXXXX.log")"
                {
                    ASAN_OPTIONS="${asan_opts}" "${harness}" -runs=1 "${corpus}"
                } >"${attempt_log}" 2>&1
                rc=$?
                if [[ ${rc} -eq 0 ]]; then
                    cat "${attempt_log}"
                    rm -f "${attempt_log}"
                    break
                fi
                if [[ ${attempt} -lt 5 ]]; then
                    log "Smoke replay retry ${attempt}/5 after exit code ${rc}: ${name}"
                    rm -f "${attempt_log}"
                fi
            done
            if [[ ${rc} -ne 0 ]]; then
                log "Smoke replay remained failing after retries: ${name}"
                cat "${attempt_log}"
                rm -f "${attempt_log}"
            fi
        elif [[ -n "${FUZZ_RUNS}" ]]; then
            ASAN_OPTIONS="${asan_opts}" "${harness}" -runs="${FUZZ_RUNS}" "${corpus}"
            rc=$?
        else
            ASAN_OPTIONS="${asan_opts}" "${harness}" -max_total_time="${FUZZ_MAX_TOTAL_TIME}" "${corpus}"
            rc=$?
        fi
        set -e
    else
        log "Corpus not found for ${name}; running without initial corpus"
        set +e
        if [[ "${FUZZ_SMOKE_ONLY}" == "1" ]]; then
            ASAN_OPTIONS="${asan_opts}" "${harness}" -runs=1
        elif [[ -n "${FUZZ_RUNS}" ]]; then
            ASAN_OPTIONS="${asan_opts}" "${harness}" -runs="${FUZZ_RUNS}"
        else
            ASAN_OPTIONS="${asan_opts}" "${harness}" -max_total_time="${FUZZ_MAX_TOTAL_TIME}"
        fi
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

    if [[ "${FUZZ_SMOKE_ONLY}" == "1" ]]; then
        log "Starting sample execution for each harness (checked-in corpus replay only)"
    elif [[ -n "${FUZZ_RUNS}" ]]; then
        log "Starting sample execution for each harness (${FUZZ_RUNS} runs each)"
    else
        log "Starting sample execution for each harness (${FUZZ_MAX_TOTAL_TIME} seconds each)"
    fi
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
