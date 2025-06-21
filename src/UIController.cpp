#include "../include/UIController.h"
#include "../include/Library.h"
#include "../include/ConsoleUtil.h"
#include <iostream>
#include <limits>

UIController::UIController(Library* lib) : library(lib) {}

int UIController::getMenuChoice() {
    int choice;
    std::cin >> choice;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}

void UIController::showInvalidChoice() {
    ConsoleUtil::printError("無效的選擇，請重試");
    ConsoleUtil::pauseAndWait();
}

void UIController::showUserInfo() {
}

bool UIController::handleLogoutChoice(int choice, int logoutOption, int exitOption) {
    if (choice == logoutOption) {
        ConsoleUtil::printSuccess("已登出");
        return false;
    } else if (choice == exitOption) {
        ConsoleUtil::printSuccess("資料已儲存，系統即將退出");
        ConsoleUtil::pauseAndWait();
        exit(0);
    }
    return true;
}

std::vector<std::string> UIController::getAdminMenuOptions() {
    return {
        "新增使用者", "設置罰款政策", "新增圖書", "刪除圖書", "編輯圖書",
        "搜尋圖書", "檢視書籍", "借閱圖書", "歸還圖書", "修改密碼", 
        "檢視統計資料", "檢視逾期圖書", "登出", "退出系統"
    };
}

std::vector<std::string> UIController::getStaffMenuOptions() {
    return {
        "新增圖書", "刪除圖書", "編輯圖書", "搜尋圖書", "檢視書籍", 
        "借閱圖書", "歸還圖書", "修改密碼", "檢視逾期圖書", "登出", "退出系統"
    };
}

std::vector<std::string> UIController::getReaderMenuOptions() {
    return {
        "搜尋圖書", "檢視書籍", "借閱圖書", "歸還圖書", "檢視我的借閱",
        "修改密碼", "檢視推薦", "登出", "退出系統"
    };
}

bool UIController::showAdminMenu() {
    auto options = getAdminMenuOptions();
    
    ConsoleUtil::printTitleWithSubtitle("圖書館管理系統", "管理員主選單");
    showUserInfo();
    ConsoleUtil::printMenuOptions(options);
    
    int choice = getMenuChoice();
    
    switch (choice) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12: 
            break;
        case 13: case 14: 
            return !handleLogoutChoice(choice, 13, 14);
        default:
            showInvalidChoice();
    }
    return true;
}

bool UIController::showStaffMenu() {
    auto options = getStaffMenuOptions();
    
    ConsoleUtil::printTitleWithSubtitle("圖書館管理系統", "館員主選單");
    showUserInfo();
    ConsoleUtil::printMenuOptions(options);
    
    int choice = getMenuChoice();
    
    switch (choice) {
        case 1: 
        case 2: 
        case 3: 
        case 4: 
        case 5: 
        case 6: 
        case 7: 
        case 8: 
        case 9: 
            break;
        case 10: case 11:
            return !handleLogoutChoice(choice, 10, 11);
        default:
            showInvalidChoice();
    }
    return true;
}

bool UIController::showReaderMenu() {
    auto options = getReaderMenuOptions();
    
    ConsoleUtil::printTitleWithSubtitle("圖書館管理系統", "讀者主選單");
    showUserInfo();
    ConsoleUtil::printMenuOptions(options);
    
    int choice = getMenuChoice();
    
    switch (choice) {
        case 1: 
        case 2: 
        case 3: 
        case 4: 
        case 5: 
        case 6: 
        case 7: 
            break;
        case 8: case 9:
            return !handleLogoutChoice(choice, 8, 9);
        default:
            showInvalidChoice();
    }
    return true;
}

std::string UIController::getUserInput(const std::string& prompt) {
    std::string input;
    std::cout << prompt << ": ";
    std::getline(std::cin, input);
    return input;
}

Role UIController::selectRole() {
    ConsoleUtil::printInfo("選擇角色 (1=館員, 2=讀者): ");
    int choice = getMenuChoice();
    return (choice == 1) ? Role::Staff : Role::Reader;
} 