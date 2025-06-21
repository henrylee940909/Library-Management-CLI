#include "SearchUtil.h"

namespace SearchUtil {

    int indexOf(const std::string& text, const std::string& pattern) {
        if (pattern.empty()) return 0;
        const size_t n = text.size(), m = pattern.size();
        if (m > n) return -1;

        for (size_t i = 0; i <= n - m; ++i) {
            size_t j = 0;
            for (; j < m && text[i + j] == pattern[j]; ++j);
            if (j == m) return static_cast<int>(i);
        }
        return -1;
    }
    int indexOf(const std::string& text, char ch) {
        for (size_t i = 0; i < text.size(); ++i)
            if (text[i] == ch) return static_cast<int>(i);
        return -1;
    }

} // namespace SearchUtil
