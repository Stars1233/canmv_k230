#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

extern "C" {
#include "ff_wrapper.h"
}

namespace {

class TempPath {
public:
    TempPath() : valid_(false) {
        char tmpl[] = "/tmp/canmv_ff_wrapper_fuzz_XXXXXX";
        const int fd = mkstemp(tmpl);
        if (fd >= 0) {
            close(fd);
            strncpy(path_, tmpl, sizeof(path_) - 1U);
            path_[sizeof(path_) - 1U] = '\0';
            valid_ = true;
        }
    }

    ~TempPath() {
        if (valid_) {
            unlink(path_);
        }
    }

    const char *path() const {
        return path_;
    }

private:
    char path_[sizeof("/tmp/canmv_ff_wrapper_fuzz_XXXXXX")] = {0};
    bool valid_;
};

UINT min_uint(UINT lhs, UINT rhs) {
    return lhs < rhs ? lhs : rhs;
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    TempPath tmp;
    if (tmp.path()[0] == '\0') {
        return 0;
    }

    FIL fp = nullptr;
    if (f_open_helper(&fp, tmp.path(), const_cast<char *>("wb+")) != FR_OK) {
        return 0;
    }

    if (size > 0) {
        UINT bw = 0;
        (void) f_write(&fp, data, size, &bw);
        file_sync(&fp);

        const UINT seek = data[0] % static_cast<UINT>(size + 1U);
        file_seek(&fp, seek);

        uint8_t scratch[64] = {0};
        const UINT to_read = min_uint((UINT) sizeof(scratch), (UINT) (size - seek));
        if (to_read > 0) {
            UINT br = 0;
            (void) f_read(&fp, scratch, to_read, &br);
        }

        file_seek(&fp, seek);
        file_truncate(&fp);
        file_sync(&fp);
    }

    file_close(&fp);

    FIL typed = nullptr;
    file_read_write_open_always(&typed, tmp.path());
    write_byte(&typed, size > 0 ? data[0] : 0U);
    write_word(&typed, size > 2 ? static_cast<uint16_t>((data[1] << 8U) | data[2]) : 0x1234U);
    write_long(&typed, static_cast<uint32_t>(size));
    file_sync(&typed);

    file_seek(&typed, 0);
    uint8_t b = 0;
    uint16_t w = 0;
    uint32_t l = 0;
    read_byte(&typed, &b);
    read_word(&typed, &w);
    read_long(&typed, &l);
    (void) b;
    (void) w;
    (void) l;

    file_close(&typed);
    return 0;
}
