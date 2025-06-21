#include "../include/VisualizationUtil.h"
#include "../include/SortUtil.h"
#include "../include/ConsoleUtil.h"
#include <iostream>
#include <iomanip>
#include <vector>

namespace VisualizationUtil {
    
    // 計算字符串的實際顯示寬度（考慮中文字符佔用2個字符寬度）
    size_t getDisplayWidth(const std::string& str) {
        size_t width = 0;
        for (size_t i = 0; i < str.length(); ) {
            unsigned char c = str[i];
            if (c >= 0x80) {
                // 多字節字符（包括中文），通常佔用2個顯示寬度
                width += 2;
                // 跳過UTF-8多字節字符的後續字節
                if ((c & 0xE0) == 0xC0) i += 2;      // 2字節字符
                else if ((c & 0xF0) == 0xE0) i += 3; // 3字節字符
                else if ((c & 0xF8) == 0xF0) i += 4; // 4字節字符
                else i += 1; // 安全回退
            } else {
                // ASCII字符，佔用1個顯示寬度
                width += 1;
                i += 1;
            }
        }
        return width;
    }
    
    void printSeparator(int length, char symbol = '=') {
        std::cout << std::string(length, symbol) << std::endl;
    }
    
    void printHeader(const std::string& title) {
        int titleLength = title.length();
        int totalWidth = titleLength + 16;
        
        std::cout << std::endl;
        printSeparator(totalWidth, '=');
        std::cout << ConsoleUtil::colorText("       " + title + "       ", ConsoleUtil::Color::BRIGHT_CYAN) << std::endl;
        printSeparator(totalWidth, '=');
        std::cout << std::endl;
    }
    
    template<typename K>
    int findMaxValue(const std::vector<std::pair<K, int>>& data) {
        int maxVal = 0;
        for (const auto& item : data) {
            if (item.second > maxVal) {
                maxVal = item.second;
            }
        }
        return maxVal;
    }
    
    template<typename K>
    int findMaxValue(const std::unordered_map<K, int>& data) {
        int maxVal = 0;
        for (const auto& item : data) {
            if (item.second > maxVal) {
                maxVal = item.second;
            }
        }
        return maxVal;
    }
    
    int calculateBarLength(int value, int maxValue, int maxWidth) {
        if (maxValue == 0) return 0;
        return static_cast<int>((static_cast<double>(value) / maxValue) * maxWidth);
    }
    
    std::string createProgressBar(int length, bool isTop = false) {
        if (length <= 0) return "";
        
        std::string bar;
        if (isTop) {
            for (int i = 0; i < length; i++) {
                if (i < length * 0.7) {
                    bar += ConsoleUtil::colorText("█", ConsoleUtil::Color::BRIGHT_GREEN);
                } else if (i < length * 0.9) {
                    bar += ConsoleUtil::colorText("█", ConsoleUtil::Color::BRIGHT_YELLOW);
                } else {
                    bar += ConsoleUtil::colorText("█", ConsoleUtil::Color::BRIGHT_RED);
                }
            }
        } else {
            std::string barStr;
            for (int i = 0; i < length; ++i) barStr += "█";
            bar = ConsoleUtil::colorText(barStr, ConsoleUtil::Color::BRIGHT_BLUE);
        }
        
        return bar;
    }
    
    void drawBarChart(const std::vector<std::pair<std::string, int>>& data, const std::string& title, int maxWidth) {
        if (data.empty()) {
            printHeader(title);
            ConsoleUtil::printWarning("暫無數據可顯示");
            std::cout << std::endl;
            return;
        }
        
        printHeader(title);
        
        int maxVal = findMaxValue(data);
        size_t maxLabelDisplayWidth = 0;
        
        for (const auto& item : data) {
            maxLabelDisplayWidth = std::max(maxLabelDisplayWidth, getDisplayWidth(item.first));
        }
        maxLabelDisplayWidth = std::max(maxLabelDisplayWidth, size_t(15)); // 最小寬度
        
        int maxRankWidth = std::to_string(data.size()).length();
        
        int rank = 1;
        for (const auto& item : data) {
            int barLength = calculateBarLength(item.second, maxVal, maxWidth);
            bool isTop = rank <= 3;
            
            std::string paddedRankStr = "[" + std::string(maxRankWidth - std::to_string(rank).length(), ' ') + std::to_string(rank) + "]";
            
            if (rank == 1) {
                std::cout << ConsoleUtil::colorText(paddedRankStr, ConsoleUtil::Color::BRIGHT_YELLOW);
            } else if (rank == 2) {
                std::cout << ConsoleUtil::colorText(paddedRankStr, ConsoleUtil::Color::BRIGHT_WHITE);
            } else if (rank == 3) {
                std::cout << ConsoleUtil::colorText(paddedRankStr, ConsoleUtil::Color::BRIGHT_YELLOW);
            } else {
                std::cout << ConsoleUtil::colorText(paddedRankStr, ConsoleUtil::Color::BRIGHT_BLACK);
            }
            
            std::string labelToShow = item.first;
            if (getDisplayWidth(labelToShow) > maxLabelDisplayWidth) {
                labelToShow = item.first.substr(0, maxLabelDisplayWidth);
            }
            
            std::cout << " " << labelToShow;
            
            size_t currentDisplayWidth = getDisplayWidth(labelToShow);
            size_t spacesToAdd = maxLabelDisplayWidth - currentDisplayWidth;
            std::cout << std::string(spacesToAdd, ' ');
            
            std::cout << " " << createProgressBar(barLength, isTop);
            
            std::cout << " " << ConsoleUtil::colorText("(" + std::to_string(item.second) + ")", 
                                                      ConsoleUtil::Color::BRIGHT_CYAN);
            std::cout << std::endl;
            
            rank++;
        }
        
        std::cout << std::endl;
        std::cout << ConsoleUtil::colorText("統計項目: " + std::to_string(data.size()) + " 個", 
                                          ConsoleUtil::Color::BRIGHT_BLACK) << std::endl;
        std::cout << std::endl;
    }
    
    void drawBarChart(const std::unordered_map<std::string, int>& data, const std::string& title, int maxWidth) {
        std::vector<std::pair<std::string, int>> sortedData;
        for (const auto& item : data) {
            sortedData.push_back(item);
        }
        
        SortUtil::sort(sortedData, [](const auto& a, const auto& b) {
            return a.second > b.second;
        });
        
        drawBarChart(sortedData, title, maxWidth);
    }
    
    void drawBarChart(const std::unordered_map<int, int>& data, const std::string& title, int maxWidth) {
        if (data.empty()) {
            printHeader(title);
            ConsoleUtil::printWarning("暫無數據可顯示");
            std::cout << std::endl;
            return;
        }
        
        std::vector<std::pair<std::string, int>> sortedData;
        for (const auto& item : data) {
            sortedData.push_back({std::to_string(item.first), item.second});
        }
        
        SortUtil::sort(sortedData, [](const auto& a, const auto& b) { 
            return a.second > b.second; 
        });
        
        drawBarChart(sortedData, title, maxWidth);
    }
    
    void drawLineChart(const std::vector<std::pair<std::string, int>>& data, const std::string& title, int maxWidth) {
        if (data.empty()) {
            printHeader(title);
            ConsoleUtil::printWarning("暫無數據可顯示");
            std::cout << std::endl;
            return;
        }
        
        printHeader(title);
        
        int maxVal = findMaxValue(data);
        size_t maxLabelDisplayWidth = 0;
        
        for (const auto& item : data) {
            maxLabelDisplayWidth = std::max(maxLabelDisplayWidth, getDisplayWidth(item.first));
        }
        maxLabelDisplayWidth = std::max(maxLabelDisplayWidth, size_t(10));
        
        int totalChange = 0;
        for (size_t i = 1; i < data.size(); i++) {
            totalChange += data[i].second - data[i-1].second;
        }
        
        ConsoleUtil::Color trendColor = ConsoleUtil::Color::BRIGHT_BLUE;
        std::string trendText = "持平";
        if (totalChange > 0) {
            trendColor = ConsoleUtil::Color::BRIGHT_GREEN;
            trendText = "上升趨勢 ↗";
        } else if (totalChange < 0) {
            trendColor = ConsoleUtil::Color::BRIGHT_RED;
            trendText = "下降趨勢 ↘";
        }
        
        std::cout << "趨勢分析: " << ConsoleUtil::colorText(trendText, trendColor) << std::endl << std::endl;
        
        for (const auto& item : data) {
            int barLength = calculateBarLength(item.second, maxVal, maxWidth);
            
            std::cout << ConsoleUtil::colorText(item.first, ConsoleUtil::Color::BRIGHT_YELLOW);
            
            size_t currentDisplayWidth = getDisplayWidth(item.first);
            size_t spacesToAdd = maxLabelDisplayWidth - currentDisplayWidth + 2;
            std::cout << std::string(spacesToAdd, ' ');
            
            if (barLength > 0) {
                std::cout << ConsoleUtil::colorText("●", ConsoleUtil::Color::BRIGHT_GREEN);
                std::string lineStr;
                for (int i = 0; i < barLength - 1; ++i) lineStr += "─";
                std::cout << ConsoleUtil::colorText(lineStr, ConsoleUtil::Color::BRIGHT_BLUE);
            }
            
            std::cout << " " << ConsoleUtil::colorText("(" + std::to_string(item.second) + ")", 
                                                      ConsoleUtil::Color::BRIGHT_CYAN);
            std::cout << std::endl;
        }
        
        std::cout << std::endl;
        std::cout << ConsoleUtil::colorText("時間範圍: " + std::to_string(data.size()) + " 個月", 
                                          ConsoleUtil::Color::BRIGHT_BLACK) << std::endl;
        std::cout << std::endl;
    }
    
    // Draw a pie chart from vector
    void drawPieChart(const std::vector<std::pair<std::string, int>>& data, const std::string& title) {
        if (data.empty()) {
            printHeader(title);
            ConsoleUtil::printWarning("暫無數據可顯示");
            std::cout << std::endl;
            return;
        }
        
        printHeader(title);
        
        int total = 0;
        size_t maxLabelDisplayWidth = 0;
        
        for (const auto& item : data) {
            total += item.second;
            maxLabelDisplayWidth = std::max(maxLabelDisplayWidth, getDisplayWidth(item.first));
        }
        maxLabelDisplayWidth = std::max(maxLabelDisplayWidth, size_t(12));
        
        std::vector<ConsoleUtil::Color> colors = {
            ConsoleUtil::Color::BRIGHT_RED,
            ConsoleUtil::Color::BRIGHT_GREEN,
            ConsoleUtil::Color::BRIGHT_YELLOW,
            ConsoleUtil::Color::BRIGHT_BLUE,
            ConsoleUtil::Color::BRIGHT_MAGENTA,
            ConsoleUtil::Color::BRIGHT_CYAN,
            ConsoleUtil::Color::BRIGHT_WHITE
        };
        
        int colorIndex = 0;
        for (const auto& item : data) {
            double percentage = static_cast<double>(item.second) * 100.0 / total;
            int barLength = static_cast<int>(percentage / 3.0); // 3% per block
            
            ConsoleUtil::Color currentColor = colors[colorIndex % colors.size()];
            
            std::cout << item.first;
            
            size_t currentDisplayWidth = getDisplayWidth(item.first);
            size_t spacesToAdd = maxLabelDisplayWidth - currentDisplayWidth;
            std::cout << std::string(spacesToAdd, ' ');
            
            std::string barStr;
            for (int i = 0; i < barLength; ++i) barStr += "█";
            std::cout << " " << ConsoleUtil::colorText(barStr, currentColor);
            
            std::cout << " " << ConsoleUtil::colorText(
                std::to_string(static_cast<int>(percentage)) + "%", 
                ConsoleUtil::Color::BRIGHT_CYAN);
            std::cout << " " << ConsoleUtil::colorText(
                "(" + std::to_string(item.second) + ")", 
                ConsoleUtil::Color::BRIGHT_BLACK);
            std::cout << std::endl;
            
            colorIndex++;
        }
        
        std::cout << std::endl;
        std::cout << ConsoleUtil::colorText("總計: " + std::to_string(total) + " 項", 
                                          ConsoleUtil::Color::BRIGHT_WHITE) << std::endl;
        std::cout << std::endl;
    }
    
    // Draw a pie chart from unordered_map
    void drawPieChart(const std::unordered_map<std::string, int>& data, const std::string& title) {
        std::vector<std::pair<std::string, int>> sortedData;
        for (const auto& item : data) {
            sortedData.push_back(item);
        }
        
        // Sort by value (descending) for better visualization
        SortUtil::sort(sortedData, [](const auto& a, const auto& b) {
            return a.second > b.second;
        });
        
        drawPieChart(sortedData, title);
    }
    
    // Helper to convert unordered_map to sorted vector for visualization
    template<typename K, typename V>
    std::vector<std::pair<K, V>> sortMapByValue(const std::unordered_map<K, V>& map, bool ascending) {
        std::vector<std::pair<K, V>> pairs;
        for (const auto& item : map) {
            pairs.push_back(item);
        }
        
        // Use SortUtil instead of std::sort
        if (ascending) {
            SortUtil::sort(pairs, [](const auto& a, const auto& b) { 
                return a.second < b.second; 
            });
        } else {
            SortUtil::sort(pairs, [](const auto& a, const auto& b) { 
                return a.second > b.second; 
            });
        }
        
        return pairs;
    }
    
    // Explicit template instantiations
    template std::vector<std::pair<std::string, int>> sortMapByValue(
        const std::unordered_map<std::string, int>& map, bool ascending);
    template std::vector<std::pair<int, int>> sortMapByValue(
        const std::unordered_map<int, int>& map, bool ascending);
} 