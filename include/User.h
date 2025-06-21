#ifndef USER_H
#define USER_H

#include <string>
#include <vector>

enum class Role {
    Admin,
    Staff,
    Reader
};

class User {
private:
    std::string username;
    std::string passwordHash;
    Role role;

public:
    User();
    User(const std::string& username, const std::string& passwordHash, Role role);
    
    // 取值方法
    std::string getUsername() const;
    std::string getPasswordHash() const;
    Role getRole() const;
    
    // 設值方法
    void setUsername(const std::string& username);
    void setPasswordHash(const std::string& passwordHash);
    void setRole(Role role);
    
    // 使用者驗證
    bool checkPassword(const std::string& password) const;
    void setNewPassword(const std::string& password);
    
    // 角色輔助方法
    std::string getRoleName() const;
    bool canBorrowBooks() const;
    bool canManageUsers() const;
    bool canManageBooks() const;
    
    // 顯示功能
    void display() const;
};

#endif // USER_H 