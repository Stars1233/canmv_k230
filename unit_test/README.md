# CANMV unit_test

This directory contains a host-side GoogleTest scaffold for CANMV source validation.

## Current coverage
- Functional unit tests for C sources:
  - `port/omv/common/array.c`
  - `port/omv/common/ringbuf.c`
  - `port/omv/alloc/unaligned_memcpy.c`
  - `port/omv/common/mutex.c`
  - `port/omv/common/ff_wrapper.c`
- Source-wide catalog-driven tests for all `*.c`, `*.cpp`, and `*.py` files under the CANMV root:
  - catalog completeness and metadata validation
  - file readability/non-empty checks
  - Python file catalog coverage checks
- Auto-generated catalog:
  - `ALL_SOURCE_UNIT_TEST_SUGGESTIONS.md`

## Run
```bash
cd unit_test
./compile_run.sh
./build/canmv_unit_tests
```

`compile_run.sh` always performs a clean build and verifies/installs gtest when missing.
- Some environment will not auto trigger unit test, hence manually run it.
