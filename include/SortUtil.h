#ifndef SORT_UTIL_H
#define SORT_UTIL_H

#include <vector>
#include <functional>

namespace SortUtil {
    template<typename InputIterator, typename OutputIterator, typename UnaryOperation>
    OutputIterator transform(InputIterator first, InputIterator last, 
                           OutputIterator result, UnaryOperation op) {
        while (first != last) {
            *result = op(*first);
            ++first;
            ++result;
        }
        return result;
    }
    
    template<typename T, typename U, typename UnaryOperation>
    std::vector<U> transform(const std::vector<T>& input, UnaryOperation op) {
        std::vector<U> result;
        result.reserve(input.size());
        
        for (const auto& element : input) {
            result.push_back(op(element));
        }
        
        return result;
    }
    
    template<typename InputIterator1, typename InputIterator2, 
             typename OutputIterator, typename BinaryOperation>
    OutputIterator transform(InputIterator1 first1, InputIterator1 last1,
                           InputIterator2 first2, OutputIterator result,
                           BinaryOperation op) {
        while (first1 != last1) {
            *result = op(*first1, *first2);
            ++first1;
            ++first2;
            ++result;
        }
        return result;
    }
    
    template<typename T1, typename T2, typename U, typename BinaryOperation>
    std::vector<U> transform(const std::vector<T1>& input1, 
                           const std::vector<T2>& input2, 
                           BinaryOperation op) {
        std::vector<U> result;
        size_t minSize = std::min(input1.size(), input2.size());
        result.reserve(minSize);
        
        for (size_t i = 0; i < minSize; ++i) {
            result.push_back(op(input1[i], input2[i]));
        }
        
        return result;
    }
    
    template<typename T, typename Compare>
    int medianOfThreeIndex(std::vector<T>& arr, int low, int high, Compare comp) {
        int mid = low + (high - low) / 2;
        
        if (comp(arr[mid], arr[low])) {
            std::swap(arr[low], arr[mid]);
        }
        if (comp(arr[high], arr[low])) {
            std::swap(arr[low], arr[high]);
        }
        if (comp(arr[high], arr[mid])) {
            std::swap(arr[mid], arr[high]);
        }
        
        return mid;
    }
    
    template<typename T, typename Compare>
    int partition(std::vector<T>& arr, int low, int high, Compare comp) {
        T pivot = arr[high];
        int i = low - 1;
        
        for (int j = low; j < high; j++) {
            if (comp(arr[j], pivot)) {
                i++;
                std::swap(arr[i], arr[j]);
            }
        }
        
        std::swap(arr[i + 1], arr[high]);
        return i + 1;
    }
    
    template<typename T, typename Compare>
    void quickSort(std::vector<T>& arr, int low, int high, Compare comp) {
        if (low < high) {
            int median = medianOfThreeIndex(arr, low, high, comp);
            
            std::swap(arr[median], arr[high]);
            
            int pi = partition(arr, low, high, comp);
            
            quickSort(arr, low, pi - 1, comp);
            quickSort(arr, pi + 1, high, comp);
        }
    }
    
    template<typename T, typename Compare>
    void sort(std::vector<T>& arr, Compare comp) {
        if (arr.size() <= 1) return;
        quickSort(arr, 0, arr.size() - 1, comp);
    }
    
    template<typename T>
    void sort(std::vector<T>& arr) {
        sort(arr, [](const T& a, const T& b) { return a < b; });
    }
    
    template<typename T>
    struct DefaultComparator {
        bool operator()(const T& a, const T& b) const {
            return a < b;
        }
    };
    
    template<typename T, typename Compare>
    void insertionSort(std::vector<T>& arr, Compare comp) {
        for (size_t i = 1; i < arr.size(); ++i) {
            T key = arr[i];
            int j = i - 1;
            
            while (j >= 0 && comp(key, arr[j])) {
                arr[j + 1] = arr[j];
                j--;
            }
            arr[j + 1] = key;
        }
    }
    
    template<typename T, typename Compare>
    int binarySearch(const std::vector<T>& arr, int left, int right, const T& key, Compare comp) {
        while (left <= right) {
            int mid = left + (right - left) / 2;
            
            if (comp(key, arr[mid])) {
                right = mid - 1;
            } else {
                left = mid + 1;
            }
        }
        
        return left;
    }
    
    template<typename InputIterator, typename UnaryFunction>
    UnaryFunction for_each(InputIterator first, InputIterator last, UnaryFunction f) {
        while (first != last) {
            f(*first);
            ++first;
        }
        return f;
    }
    
    template<typename T, typename UnaryFunction>
    UnaryFunction for_each(const std::vector<T>& container, UnaryFunction f) {
        for (const auto& element : container) {
            f(element);
        }
        return f;
    }
    
    template<typename InputIterator, typename T>
    InputIterator find(InputIterator first, InputIterator last, const T& value) {
        while (first != last) {
            if (*first == value) {
                return first;
            }
            ++first;
        }
        return last;
    }

    template<typename InputIterator, typename UnaryPredicate>
    InputIterator find_if(InputIterator first, InputIterator last, UnaryPredicate pred) {
        while (first != last) {
            if (pred(*first)) {
                return first;
            }
            ++first;
        }
        return last;
    }
    
    template<typename InputIterator, typename T>
    int count(InputIterator first, InputIterator last, const T& value) {
        int result = 0;
        while (first != last) {
            if (*first == value) {
                ++result;
            }
            ++first;
        }
        return result;
    }
    
    template<typename InputIterator, typename UnaryPredicate>
    int count_if(InputIterator first, InputIterator last, UnaryPredicate pred) {
        int result = 0;
        while (first != last) {
            if (pred(*first)) {
                ++result;
            }
            ++first;
        }
        return result;
    }
    
    template<typename InputIterator, typename UnaryPredicate>
    bool all_of(InputIterator first, InputIterator last, UnaryPredicate pred) {
        while (first != last) {
            if (!pred(*first)) {
                return false;
            }
            ++first;
        }
        return true;
    }
    
    template<typename InputIterator, typename UnaryPredicate>
    bool any_of(InputIterator first, InputIterator last, UnaryPredicate pred) {
        while (first != last) {
            if (pred(*first)) {
                return true;
            }
            ++first;
        }
        return false;
    }
    
    template<typename InputIterator, typename UnaryPredicate>
    bool none_of(InputIterator first, InputIterator last, UnaryPredicate pred) {
        return !any_of(first, last, pred);
    }
    
    template<typename InputIterator, typename OutputIterator>
    OutputIterator copy(InputIterator first, InputIterator last, OutputIterator result) {
        while (first != last) {
            *result = *first;
            ++first;
            ++result;
        }
        return result;
    }
    
    template<typename InputIterator, typename OutputIterator, typename UnaryPredicate>
    OutputIterator copy_if(InputIterator first, InputIterator last, 
                          OutputIterator result, UnaryPredicate pred) {
        while (first != last) {
            if (pred(*first)) {
                *result = *first;
                ++result;
            }
            ++first;
        }
        return result;
    }
    
    template<typename T, typename UnaryPredicate>
    void remove_if(std::vector<T>& container, UnaryPredicate pred) {
        auto writePos = container.begin();
        for (auto readPos = container.begin(); readPos != container.end(); ++readPos) {
            if (!pred(*readPos)) {
                if (writePos != readPos) {
                    *writePos = *readPos;
                }
                ++writePos;
            }
        }
        container.erase(writePos, container.end());
    }
}

#endif // SORT_UTIL_H 