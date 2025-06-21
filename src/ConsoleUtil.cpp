#include "../include/ConsoleUtil.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <vector>
#include <thread>
#include <chrono>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
#endif

void ConsoleUtil::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void ConsoleUtil::pauseAndWait() {
    printInfo("按 Enter 繼續...");
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void ConsoleUtil::moveCursor(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H";
}

void ConsoleUtil::hideCursor() {
    std::cout << "\033[?25l";
}

void ConsoleUtil::showCursor() {
    std::cout << "\033[?25h";
}

std::string ConsoleUtil::getColorCode(Color color) {
    return "\033[" + std::to_string(static_cast<int>(color)) + "m";
}

std::string ConsoleUtil::getBgColorCode(BgColor color) {
    return "\033[" + std::to_string(static_cast<int>(color)) + "m";
}

void ConsoleUtil::printColored(const std::string& text, Color color) {
    std::cout << getColorCode(color) << text << getColorCode(Color::RESET);
}

void ConsoleUtil::printColoredBg(const std::string& text, Color textColor, BgColor bgColor) {
    std::cout << getColorCode(textColor) << getBgColorCode(bgColor) 
              << text << getColorCode(Color::RESET) << getBgColorCode(BgColor::RESET);
}

std::string ConsoleUtil::colorText(const std::string& text, Color color) {
    return getColorCode(color) + text + getColorCode(Color::RESET);
}

void ConsoleUtil::printTitle(const std::string& title) {
    clearScreen();
    printSeparator('=', 60);
    std::cout << "\n";
    printColoredBg("  " + title + "  ", Color::BRIGHT_WHITE, BgColor::BLUE);
    std::cout << "\n";
    printSeparator('=', 60);
    std::cout << "\n";
}

void ConsoleUtil::printTitleWithSubtitle(const std::string& title, const std::string& subtitle) {
    clearScreen();
    printSeparator('=', 60);
    std::cout << "\n";
    printColoredBg("  " + title + " - " + subtitle + "  ", Color::BRIGHT_WHITE, BgColor::BLUE);
    std::cout << "\n";
    printSeparator('=', 60);
    std::cout << "\n";
}

void ConsoleUtil::printSubtitle(const std::string& subtitle) {
    std::cout << "\n";
    printColored("── " + subtitle + " ──", Color::BRIGHT_CYAN);
    std::cout << "\n";
}

void ConsoleUtil::printSuccess(const std::string& message) {
    printColored("[OK] " + message, Color::BRIGHT_GREEN);
    std::cout << std::endl;
}

void ConsoleUtil::printError(const std::string& message) {
    printColored("[ERROR] " + message, Color::BRIGHT_RED);
    std::cout << std::endl;
}

void ConsoleUtil::printWarning(const std::string& message) {
    printColored("[WARNING] " + message, Color::BRIGHT_YELLOW);
    std::cout << std::endl;
}

void ConsoleUtil::printInfo(const std::string& message) {
    printColored("[INFO] " + message, Color::BRIGHT_BLUE);
    std::cout << std::endl;
}

void ConsoleUtil::printSeparator(char character, int length) {
    printColored(std::string(length, character), Color::CYAN);
    std::cout << std::endl;
}

void ConsoleUtil::printBox(const std::string& content, int width) {
    std::string border = "+" + std::string(width - 2, '-') + "+";
    printColored(border, Color::CYAN);
    std::cout << "\n";
    
    int padding = (width - content.length() - 2) / 2;
    std::string line = "|" + std::string(padding, ' ') + content + 
                      std::string(width - content.length() - padding - 2, ' ') + "|";
    printColored(line, Color::CYAN);
    std::cout << "\n";
    
    printColored(border, Color::CYAN);
    std::cout << "\n";
}

void ConsoleUtil::printMenu(const std::vector<std::string>& options, const std::string& title) {
    if (!title.empty()) {
        printSubtitle(title);
    }
    
    int maxWidth = std::to_string(options.size()).length();
    
    for (size_t i = 0; i < options.size(); ++i) {
        std::cout << " ";
        std::cout << std::right << std::setw(maxWidth);
        printColored(std::to_string(i + 1), Color::BRIGHT_YELLOW);
        std::cout << ". " << options[i] << std::endl;
    }
    
    std::cout << "\n";
    printColored("請輸入您的選擇: ", Color::BRIGHT_WHITE);
}

void ConsoleUtil::printMenuOptions(const std::vector<std::string>& options) {
    int maxWidth = std::to_string(options.size()).length();
    
    for (size_t i = 0; i < options.size(); ++i) {
        std::cout << " ";
        std::cout << std::right << std::setw(maxWidth);
        printColored(std::to_string(i + 1), Color::BRIGHT_YELLOW);
        std::cout << ". " << options[i] << std::endl;
    }
    
    std::cout << "\n";
    printColored("請輸入您的選擇: ", Color::BRIGHT_WHITE);
}

void ConsoleUtil::printProgressBar(int current, int total, int width) {
    double progress = static_cast<double>(current) / total;
    int filled = static_cast<int>(progress * width);
    
    std::cout << "[";
    printColored(std::string(filled, '#'), Color::BRIGHT_GREEN);
    std::cout << std::string(width - filled, '-');
    std::cout << "] " << std::fixed << std::setprecision(1) 
              << (progress * 100) << "%\r";
    std::cout.flush();
}

void ConsoleUtil::printLoading(const std::string& message) {
    static const char spinner[] = "|/-\\";
    static int index = 0;
    
    std::cout << "\r";
    printColored(std::string(1, spinner[index]), Color::BRIGHT_CYAN);
    std::cout << " " << message << "...";
    std::cout.flush();
    
    index = (index + 1) % 4;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
} 