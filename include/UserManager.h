#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <vector>
#include <unordered_map>
#include <string>
#include "User.h"

class UserManager {
private:
    std::unordered_map<std::string, User> users; // username -> User
    User* currentUser;

public:
    UserManager();
    UserManager(const std::string& filename);

    void setFilename(const std::string& filename);

    // 使用者操作
    bool addUser(const std::string& username, const std::string& password, Role role);
    bool deleteUser(const std::string& username);
    bool updateUser(const std::string& username, const User& updatedUser);
    User* findUser(const std::string& username);
    const User* findUser(const std::string& username) const;

    // 身份驗證
    bool login(const std::string& username, const std::string& password);
    void logout();
    bool isLoggedIn() const;
    User* getCurrentUser() const;

    // 密碼管理
    bool changePassword(const std::string& username, const std::string& oldPassword, 
                       const std::string& newPassword);

    // 檢查權限
    bool hasPermission(Role requiredRole) const;

    // 檔案操作
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename) const;

    // 首次設定
    bool isFirstRun() const;
    bool setupAdminAccount(const std::string& username, const std::string& password);

    // 取得所有使用者
    const std::vector<User>& getAllUsers() const;

    // 顯示功能
    void displayAllUsers() const;
};

#endif // USER_MANAGER_H 