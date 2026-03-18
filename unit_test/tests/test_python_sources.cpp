#include <gtest/gtest.h>

#include <cctype>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;

namespace {

struct CatalogEntry {
    std::string rel_path;
    std::string suggestion;
    std::string priority;
};

const fs::path &source_root() {
    static const fs::path kRoot = fs::path(CANMV_SOURCE_ROOT);
    return kRoot;
}

const fs::path &unit_test_root() {
    static const fs::path kRoot = fs::path(CANMV_UNIT_TEST_ROOT);
    return kRoot;
}

std::string trim(const std::string &input) {
    const std::string ws = " \t\r\n";
    const size_t start = input.find_first_not_of(ws);
    if (start == std::string::npos) {
        return "";
    }
    const size_t end = input.find_last_not_of(ws);
    return input.substr(start, end - start + 1);
}

std::vector<std::string> split_pipe(const std::string &line) {
    std::vector<std::string> out;
    std::string cur;
    for (char ch : line) {
        if (ch == '|') {
            out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(ch);
        }
    }
    out.push_back(cur);
    return out;
}

std::string strip_ticks(const std::string &text) {
    if (text.size() >= 2 && text.front() == '`' && text.back() == '`') {
        return text.substr(1, text.size() - 2);
    }
    return text;
}

std::vector<CatalogEntry> load_catalog() {
    const fs::path md = unit_test_root() / "ALL_SOURCE_UNIT_TEST_SUGGESTIONS.md";
    std::ifstream in(md);
    if (!in.is_open()) {
        throw std::runtime_error("Cannot open catalog");
    }

    std::vector<CatalogEntry> entries;
    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("| `", 0) != 0) {
            continue;
        }
        const auto cols = split_pipe(line);
        if (cols.size() < 5) {
            continue;
        }

        CatalogEntry e;
        e.rel_path = strip_ticks(trim(cols[1]));
        e.suggestion = trim(cols[2]);
        e.priority = trim(cols[3]);
        entries.push_back(e);
    }
    return entries;
}

std::set<std::string> list_python_sources() {
    std::set<std::string> out;
    for (const auto &entry : fs::recursive_directory_iterator(source_root())) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const auto rel = fs::relative(entry.path(), source_root()).generic_string();
        if (rel.rfind("unit_test/", 0) == 0) {
            continue;
        }

        if (entry.path().extension() == ".py") {
            out.insert(rel);
        }
    }
    return out;
}

}  // namespace

TEST(PythonSourceTest, CatalogCoversAllPythonFiles) {
    const auto entries = load_catalog();
    const auto actual_py = list_python_sources();

    std::set<std::string> catalog_py;
    for (const auto &e : entries) {
        if (e.rel_path.size() >= 3 && e.rel_path.substr(e.rel_path.size() - 3) == ".py") {
            catalog_py.insert(e.rel_path);
        }
    }

    EXPECT_EQ(catalog_py, actual_py);
}

TEST(PythonSourceTest, PythonFilesAreReadableAndNonEmpty) {
    const auto actual_py = list_python_sources();
    ASSERT_FALSE(actual_py.empty());

    for (const auto &rel : actual_py) {
        SCOPED_TRACE(rel);

        const fs::path full = source_root() / rel;
        ASSERT_TRUE(fs::exists(full));
        ASSERT_TRUE(fs::is_regular_file(full));
        EXPECT_GT(fs::file_size(full), 0U);

        std::ifstream in(full);
        ASSERT_TRUE(in.is_open());

        std::string line;
        bool has_non_ws = false;
        while (std::getline(in, line)) {
            for (char ch : line) {
                if (!isspace(static_cast<unsigned char>(ch))) {
                    has_non_ws = true;
                    break;
                }
            }
            if (has_non_ws) {
                break;
            }
        }

        EXPECT_TRUE(has_non_ws) << "Python file contains only whitespace";
    }
}

TEST(PythonSourceTest, CatalogProvidesPythonSpecificGuidance) {
    const auto entries = load_catalog();

    bool saw_python_entry = false;
    for (const auto &e : entries) {
        if (e.rel_path.size() < 3 || e.rel_path.substr(e.rel_path.size() - 3) != ".py") {
            continue;
        }

        saw_python_entry = true;
        EXPECT_NE(e.suggestion.find("Python"), std::string::npos)
            << "Suggestion is not python-specific";
        EXPECT_TRUE(e.priority == "High" || e.priority == "Medium" || e.priority == "Low");
    }

    EXPECT_TRUE(saw_python_entry);
}
