#ifndef UICONTROLLER_H
#define UICONTROLLER_H

#include <string>
#include <vector>
#include "User.h"

class Library;

class UIController {
private:
    Library* library;
    
    // 選單輔助
    int getMenuChoice();
    void showInvalidChoice();
    void showUserInfo();
    bool handleLogoutChoice(int choice, int logoutOption, int exitOption);
    
    // 選單選項建構
    std::vector<std::string> getAdminMenuOptions();
    std::vector<std::string> getStaffMenuOptions();
    std::vector<std::string> getReaderMenuOptions();

public:
    explicit UIController(Library* lib);
    
    // 主要選單
    bool showAdminMenu();
    bool showStaffMenu();
    bool showReaderMenu();
    
    // 輔助方法
    std::string getUserInput(const std::string& prompt);
    Role selectRole();
};

#endif // UICONTROLLER_H 