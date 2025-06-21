#ifndef SEARCH_UTIL_H
#define SEARCH_UTIL_H

#include <string>
#include <iterator>
#include <functional>   // std::less
#include <vector>

namespace SearchUtil {

/* -----------------------------------------------------------
 * ❶ 字串 / 字元 搜尋
 *    - indexOf : 回傳第一個匹配位置，找不到 → -1
 *    - contains: bool 版捷徑
 * ---------------------------------------------------------- */
int indexOf(const std::string& text, const std::string& pattern);
int indexOf(const std::string& text, char ch);

inline bool contains(const std::string& text, const std::string& pattern) {
    return indexOf(text, pattern) != -1;
}
inline bool contains(const std::string& text, char ch) {
    return indexOf(text, ch) != -1;
}

/* -----------------------------------------------------------
 * ❷ 已排序序列 → 二分搜尋 (O(log n))
 * ---------------------------------------------------------- */
template <typename RandomIt, typename T, typename Compare = std::less<>>
RandomIt binaryFind(RandomIt first, RandomIt last,
                    const T& value, Compare comp = Compare{}) {
    using diff_t = typename std::iterator_traits<RandomIt>::difference_type;
    diff_t count = last - first;
    while (count > 0) {
        diff_t step = count / 2;
        RandomIt mid = first + step;
        if (comp(*mid, value)) {
            first = mid + 1;
            count -= step + 1;
        } else if (comp(value, *mid)) {
            count = step;
        } else {
            return mid;               // found
        }
    }
    return last;                       // not found
}
template <typename RandomIt, typename T, typename Compare = std::less<>>
inline bool binaryContains(RandomIt first, RandomIt last,
                           const T& value, Compare comp = Compare{}) {
    return binaryFind(first, last, value, comp) != last;
}

/* -----------------------------------------------------------
 * ❸ 非排序序列 → 線性搜尋 (O(n))
 * ---------------------------------------------------------- */
template <typename InputIt, typename T>
InputIt linearFind(InputIt first, InputIt last, const T& value) {
    for (; first != last; ++first)
        if (*first == value) return first;
    return last;
}
template <typename InputIt, typename T>
inline bool linearContains(InputIt first, InputIt last, const T& value) {
    return linearFind(first, last, value) != last;
}

/* -----------------------------------------------------------
 * ❹ map / unordered_map → 手刻 key 搜尋
 * ---------------------------------------------------------- */
template <typename MapType, typename KeyType>
typename MapType::iterator mapFind(MapType& m, const KeyType& key) {
    for (auto it = m.begin(); it != m.end(); ++it)
        if (it->first == key) return it;
    return m.end();
}
template <typename MapType, typename KeyType>
typename MapType::const_iterator mapFind(const MapType& m, const KeyType& key) {
    for (auto it = m.begin(); it != m.end(); ++it)
        if (it->first == key) return it;
    return m.end();
}
template <typename MapType, typename KeyType>
inline bool mapContains(const MapType& m, const KeyType& key) {
    return mapFind(m, key) != m.end();
}

} // namespace SearchUtil
#endif // SEARCH_UTIL_H
