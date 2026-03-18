#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

const fs::path &source_root() {
    static const fs::path kRoot = fs::path(CANMV_SOURCE_ROOT);
    return kRoot;
}

std::string read_text(const fs::path &path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("failed to open source file: " + path.string());
    }
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

std::string extract_function_body(const std::string &text, const std::string &func_name) {
    const std::string needle = func_name + "(";
    const size_t sig_pos = text.find(needle);
    if (sig_pos == std::string::npos) {
        throw std::runtime_error("function signature not found: " + func_name);
    }

    const size_t brace_open = text.find('{', sig_pos);
    if (brace_open == std::string::npos) {
        throw std::runtime_error("opening brace not found for function: " + func_name);
    }

    int depth = 0;
    bool in_string = false;
    bool in_char = false;
    bool escape = false;
    for (size_t i = brace_open; i < text.size(); ++i) {
        const char ch = text[i];

        if (escape) {
            escape = false;
            continue;
        }
        if (ch == '\\') {
            escape = true;
            continue;
        }

        if (!in_char && ch == '"' ) {
            in_string = !in_string;
            continue;
        }
        if (!in_string && ch == '\'') {
            in_char = !in_char;
            continue;
        }
        if (in_string || in_char) {
            continue;
        }

        if (ch == '{') {
            ++depth;
        } else if (ch == '}') {
            --depth;
            if (depth == 0) {
                return text.substr(brace_open, i - brace_open + 1);
            }
        }
    }

    throw std::runtime_error("unterminated function body: " + func_name);
}

std::vector<std::string> non_comment_lines(const std::string &text) {
    std::vector<std::string> lines;
    size_t start = 0;
    while (start <= text.size()) {
        size_t end = text.find('\n', start);
        if (end == std::string::npos) {
            end = text.size();
        }
        std::string line = text.substr(start, end - start);
        size_t non_ws = line.find_first_not_of(" \t");
        if (non_ws != std::string::npos && line.compare(non_ws, 2, "//") != 0) {
            lines.push_back(line);
        }
        if (end == text.size()) {
            break;
        }
        start = end + 1;
    }
    return lines;
}

}  // namespace

TEST(RecentFixesGuardrailsTest, SaveWavArgumentIndexIsInRange) {
    const std::string text = read_text(source_root() / "port/ai_demo/ai_demo.c");
    const std::string body = extract_function_body(text, "save_wav");
    EXPECT_NE(body.find("args[3]"), std::string::npos);
    EXPECT_EQ(body.find("args[4]"), std::string::npos);
}

TEST(RecentFixesGuardrailsTest, BodySegArgumentIndexIsInRange) {
    const std::string text = read_text(source_root() / "port/ai_demo/ai_demo.c");
    const std::string body = extract_function_body(text, "aidemo_body_seg_postprocess");
    EXPECT_NE(body.find("args[4]"), std::string::npos);
    EXPECT_EQ(body.find("args[5]"), std::string::npos);
}

TEST(RecentFixesGuardrailsTest, TtsPreprocessFreesOwnedBuffers) {
    const std::string text = read_text(source_root() / "port/ai_demo/ai_demo.c");
    const std::string body = extract_function_body(text, "tts_zh_preprocess");
    EXPECT_NE(body.find("free(tts_zh_out->data);"), std::string::npos);
    EXPECT_NE(body.find("free(tts_zh_out->len_data);"), std::string::npos);
    EXPECT_NE(body.find("free(tts_zh_out);"), std::string::npos);
}

TEST(RecentFixesGuardrailsTest, MachineWdtUsesBoundedFormatting) {
    const std::string text = read_text(source_root() / "port/machine/machine_wdt.c");
    EXPECT_NE(text.find("snprintf("), std::string::npos);
    EXPECT_EQ(text.find("sprintf("), std::string::npos);
}

TEST(RecentFixesGuardrailsTest, CvLiteUsesCheckedImageAllocationHelper) {
    const std::string text = read_text(source_root() / "port/cv_lite/cv_lite.c");
    EXPECT_NE(text.find("cv_lite_malloc_u8"), std::string::npos);

    const std::regex raw_malloc_pattern(R"(uint8_t\s*\*\s*result\s*=\s*\(uint8_t\s*\*\)\s*malloc\s*\()");
    EXPECT_FALSE(std::regex_search(text, raw_malloc_pattern));
}

TEST(RecentFixesGuardrailsTest, CvLiteHasNoLiteralMpObjGetCalls) {
    const std::string text = read_text(source_root() / "port/cv_lite/cv_lite.c");
    const std::regex literal_get_pattern(R"(mp_obj_get_(int|float)\(\s*\d+\s*\))");
    EXPECT_FALSE(std::regex_search(text, literal_get_pattern));
}

TEST(RecentFixesGuardrailsTest, CvLiteDistCoeffBufferIsFixedSize) {
    const std::string text = read_text(source_root() / "port/cv_lite/cv_lite.c");
    const auto lines = non_comment_lines(text);
    for (const std::string &line : lines) {
        EXPECT_EQ(line.find("float dist_coeffs[dist_coeff_len]"), std::string::npos);
    }
}

TEST(RecentFixesGuardrailsTest, AiDemoNoRuntimeSizedBoxPointStackArray) {
    const std::string text = read_text(source_root() / "port/ai_demo/ai_demo.c");
    const auto lines = non_comment_lines(text);
    for (const std::string &line : lines) {
        EXPECT_EQ(line.find("BoxPoint8 boxpoint8[box_cnt]"), std::string::npos);
    }
}

TEST(RecentFixesGuardrailsTest, AiCubeNoRuntimeSizedStrideArrays) {
    const std::string text = read_text(source_root() / "port/ai_cube/ai_cube.c");
    const std::regex vla_pattern(R"(\b(int|float)\s+\w+\s*\[\s*\w+_mp->len\s*\])");
    EXPECT_FALSE(std::regex_search(text, vla_pattern));
}

TEST(RecentFixesGuardrailsTest, TtsZhLastCharIndexingIsBoundsSafe) {
    const std::string text =
        read_text(source_root() / "port/ai_demo/tts_zh/tts_zh_preprocess.cpp");
    EXPECT_EQ(text.find("t[t.length()]"), std::string::npos);
    EXPECT_NE(text.find("t[t.length() - 1]"), std::string::npos);
}
