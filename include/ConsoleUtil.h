#ifndef CONSOLE_UTIL_H
#define CONSOLE_UTIL_H

#include <string>
#include <iostream>
#include <vector>

class ConsoleUtil {
public:
    // 顏色代碼
    enum class Color {
        RESET = 0,
        BLACK = 30,
        RED = 31,
        GREEN = 32,
        YELLOW = 33,
        BLUE = 34,
        MAGENTA = 35,
        CYAN = 36,
        WHITE = 37,
        BRIGHT_BLACK = 90,
        BRIGHT_RED = 91,
        BRIGHT_GREEN = 92,
        BRIGHT_YELLOW = 93,
        BRIGHT_BLUE = 94,
        BRIGHT_MAGENTA = 95,
        BRIGHT_CYAN = 96,
        BRIGHT_WHITE = 97
    };

    // 背景顏色代碼
    enum class BgColor {
        RESET = 0,
        BLACK = 40,
        RED = 41,
        GREEN = 42,
        YELLOW = 43,
        BLUE = 44,
        MAGENTA = 45,
        CYAN = 46,
        WHITE = 47
    };

    // 基本功能
    static void clearScreen();
    static void pauseAndWait();
    static void moveCursor(int row, int col);
    static void hideCursor();
    static void showCursor();

    // 顏色輸出
    static void printColored(const std::string& text, Color color = Color::RESET);
    static void printColoredBg(const std::string& text, Color textColor, BgColor bgColor);
    static std::string colorText(const std::string& text, Color color);

    // 格式化輸出
    static void printTitle(const std::string& title);
    static void printTitleWithSubtitle(const std::string& title, const std::string& subtitle);
    static void printSubtitle(const std::string& subtitle);
    static void printSuccess(const std::string& message);
    static void printError(const std::string& message);
    static void printWarning(const std::string& message);
    static void printInfo(const std::string& message);
    
    // 分隔線和框線
    static void printSeparator(char character = '=', int length = 50);
    static void printBox(const std::string& content, int width = 50);
    static void printMenu(const std::vector<std::string>& options, const std::string& title = "");
    static void printMenuOptions(const std::vector<std::string>& options); // 只顯示選項，不顯示標題

    // 進度和狀態
    static void printProgressBar(int current, int total, int width = 30);
    static void printLoading(const std::string& message = "處理中");

private:
    static std::string getColorCode(Color color);
    static std::string getBgColorCode(BgColor color);
};

#endif // CONSOLE_UTIL_H 