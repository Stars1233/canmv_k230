#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <unistd.h>

extern "C" {
#include "ff_wrapper.h"
}

namespace {

class TempFilePath {
public:
    TempFilePath() {
        char tmpl[] = "/tmp/canmv_ff_wrapper_test_XXXXXX";
        int fd = mkstemp(tmpl);
        EXPECT_GE(fd, 0);
        if (fd >= 0) {
            close(fd);
        }
        path_ = tmpl;
    }

    ~TempFilePath() {
        if (!path_.empty()) {
            unlink(path_.c_str());
        }
    }

    const std::string &path() const { return path_; }

private:
    std::string path_;
};

}  // namespace

TEST(FfWrapperTest, OpenWriteReadCloseRoundTrip) {
    TempFilePath tmp;
    ASSERT_FALSE(tmp.path().empty());

    FIL fp = nullptr;
    ASSERT_EQ(f_open_helper(&fp, tmp.path().c_str(), const_cast<char *>("wb+")), FR_OK);

    const char payload[] = "abc123";
    UINT bw = 0;
    ASSERT_EQ(f_write(&fp, payload, sizeof(payload), &bw), FR_OK);
    EXPECT_EQ(bw, sizeof(payload));

    file_sync(&fp);
    file_seek(&fp, 0);

    char out[sizeof(payload)] = {0};
    UINT br = 0;
    ASSERT_EQ(f_read(&fp, out, sizeof(out), &br), FR_OK);
    EXPECT_EQ(br, sizeof(out));
    EXPECT_EQ(memcmp(out, payload, sizeof(payload)), 0);

    EXPECT_GT(f_size(&fp), 0U);
    EXPECT_FALSE(f_eof(&fp));

    file_close(&fp);
}

TEST(FfWrapperTest, ByteWordLongHelpersRoundTrip) {
    TempFilePath tmp;
    ASSERT_FALSE(tmp.path().empty());

    FIL fp = nullptr;
    file_read_write_open_always(&fp, tmp.path().c_str());

    write_byte(&fp, 0x7A);
    write_word(&fp, 0x12B4);
    write_long(&fp, 0x89ABCDEFu);

    EXPECT_EQ(file_tell_w_buf(&fp), 7U);
    EXPECT_EQ(file_size_w_buf(&fp), 7U);

    file_seek(&fp, 0);

    uint8_t b = 0;
    uint16_t w = 0;
    uint32_t l = 0;
    read_byte(&fp, &b);
    read_word(&fp, &w);
    read_long(&fp, &l);

    EXPECT_EQ(b, 0x7A);
    EXPECT_EQ(w, 0x12B4);
    EXPECT_EQ(l, 0x89ABCDEFu);

    file_close(&fp);
}

TEST(FfWrapperTest, TruncateShrinksFileAtCurrentPosition) {
    TempFilePath tmp;
    ASSERT_FALSE(tmp.path().empty());

    FIL fp = nullptr;
    file_read_write_open_always(&fp, tmp.path().c_str());

    std::array<uint8_t, 16> bytes{};
    for (size_t i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<uint8_t>(i);
    }

    UINT bw = 0;
    ASSERT_EQ(f_write(&fp, bytes.data(), bytes.size(), &bw), FR_OK);
    ASSERT_EQ(bw, bytes.size());

    file_seek(&fp, 5);
    file_truncate(&fp);
    file_sync(&fp);
    EXPECT_EQ(file_size_w_buf(&fp), 5U);

    file_close(&fp);

    FIL rp = nullptr;
    file_read_open(&rp, tmp.path().c_str());
    EXPECT_EQ(f_size(&rp), 5U);
    file_close(&rp);
}
