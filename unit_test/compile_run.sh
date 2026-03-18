#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

log() {
    printf '[unit_test] %s\n' "$*"
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

require_tool() {
    local tool="$1"
    if ! command -v "$tool" >/dev/null 2>&1; then
        log "Missing required tool: ${tool}"
        exit 1
    fi
}

install_gtest() {
    local build_dir="/tmp/gtest-build"
    local src_dir=""

    if [[ -d /usr/src/googletest ]]; then
        src_dir="/usr/src/googletest"
    elif [[ -d /usr/src/gtest ]]; then
        src_dir="/usr/src/gtest"
    fi

    if [[ -z "${src_dir}" ]]; then
        if command -v apt-get >/dev/null 2>&1; then
            log "Installing gtest with apt-get"
            run_privileged apt-get update
            run_privileged apt-get install -y libgtest-dev googletest
        else
            log "apt-get not available. Falling back to source install from GitHub."
        fi

        if [[ -d /usr/src/googletest ]]; then
            src_dir="/usr/src/googletest"
        elif [[ -d /usr/src/gtest ]]; then
            src_dir="/usr/src/gtest"
        fi
    fi

    if [[ -z "${src_dir}" ]]; then
        require_tool git
        src_dir="/tmp/googletest"
        rm -rf "${src_dir}"
        log "Cloning googletest"
        git clone --depth 1 https://github.com/google/googletest.git "${src_dir}"
    fi

    log "Building and installing gtest from ${src_dir}"
    rm -rf "${build_dir}"
    cmake -S "${src_dir}" -B "${build_dir}" -DCMAKE_BUILD_TYPE=Release
    cmake --build "${build_dir}" -j"$(nproc)"
    run_privileged cmake --install "${build_dir}"
}

ensure_gtest() {
    if [[ -f /usr/include/gtest/gtest.h ]]; then
        log "gtest headers found in /usr/include"
        return
    fi

    if pkg-config --exists gtest 2>/dev/null; then
        log "gtest found via pkg-config"
        return
    fi

    log "gtest not found. Installing..."
    install_gtest

    if [[ ! -f /usr/include/gtest/gtest.h ]] && ! pkg-config --exists gtest 2>/dev/null; then
        log "gtest installation did not complete successfully"
        exit 1
    fi
}

main() {
    require_tool cmake
    require_tool g++
    require_tool bash

    ensure_gtest

    log "Generating source-wide test suggestion catalog"
    "${SCRIPT_DIR}/scripts/generate_test_suggestions.sh"

    log "Removing previous build directory for clean compile"
    rm -rf "${BUILD_DIR}"

    log "Configuring"
    cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug

    log "Building"
    cmake --build "${BUILD_DIR}" --clean-first -j"$(nproc)"

    log "Running tests"
    ctest --test-dir "${BUILD_DIR}" --output-on-failure
}

main "$@"
