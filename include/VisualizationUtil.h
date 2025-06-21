#ifndef VISUALIZATION_UTIL_H
#define VISUALIZATION_UTIL_H

#include <unordered_map>
#include <vector>
#include <string>
#include <utility> // for std::pair

namespace VisualizationUtil {
    void drawBarChart(const std::vector<std::pair<std::string, int>>& data, const std::string& title, int maxWidth = 50);
    
    void drawBarChart(const std::unordered_map<std::string, int>& data, const std::string& title, int maxWidth = 50);
    
    void drawBarChart(const std::unordered_map<int, int>& data, const std::string& title, int maxWidth = 50);
    
    void drawLineChart(const std::vector<std::pair<std::string, int>>& data, const std::string& title, int maxWidth = 50);
    
    void drawPieChart(const std::vector<std::pair<std::string, int>>& data, const std::string& title);
    
    void drawPieChart(const std::unordered_map<std::string, int>& data, const std::string& title);
    
    template<typename K, typename V>
    std::vector<std::pair<K, V>> sortMapByValue(const std::unordered_map<K, V>& map, bool ascending = false);
}

#endif // VISUALIZATION_UTIL_H 