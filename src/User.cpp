#include "../include/User.h"
#include "../include/PasswordUtil.h"
#include <iostream>
#include <iomanip>

User::User() : role(Role::Reader) {}

User::User(const std::string& username, const std::string& passwordHash, Role role)
    : username(username), passwordHash(passwordHash), role(role) {}

// 取值方法
std::string User::getUsername() const { return username; }
std::string User::getPasswordHash() const { return passwordHash; }
Role User::getRole() const { return role; }

// 設值方法
void User::setUsername(const std::string& username) { this->username = username; }
void User::setPasswordHash(const std::string& passwordHash) { this->passwordHash = passwordHash; }
void User::setRole(Role role) { this->role = role; }

// 使用者驗證
bool User::checkPassword(const std::string& password) const {
    return PasswordUtil::verifyPassword(password, passwordHash);
}

void User::setNewPassword(const std::string& password) {
    this->passwordHash = PasswordUtil::hashPassword(password);
}

// 角色輔助方法
std::string User::getRoleName() const {
    switch (role) {
        case Role::Admin: return "Admin";
        case Role::Staff: return "Staff";
        case Role::Reader: return "Reader";
        default: return "Unknown";
    }
}

bool User::canBorrowBooks() const {
    return true; // All users can borrow books
}

bool User::canManageUsers() const {
    return role == Role::Admin; // Only admins can manage users
}

bool User::canManageBooks() const {
    return role == Role::Admin || role == Role::Staff; // Admins and staff can manage books
}

// Display
void User::display() const {
    std::cout << std::left
              << std::setw(15) << username
              << std::setw(10) << getRoleName()
              << std::endl;
} 