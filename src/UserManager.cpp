#include "../include/UserManager.h"
#include "../include/PasswordUtil.h"
#include "../include/SearchUtil.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include "../include/SimpleJSON.h"

using JSONValue = SimpleJSON::JSONValue;

UserManager::UserManager() : currentUser(nullptr) {}

UserManager::UserManager(const std::string& filename) : currentUser(nullptr) {
    loadFromFile(filename);
}

void UserManager::setFilename(const std::string& filename) {
    // This method might be used for setting a default filename
    // Implementation can be added if needed
}

// User operations
bool UserManager::addUser(const std::string& username, const std::string& password, Role role) {
    // Check if user already exists
    if (SearchUtil::mapFind(users, username) != users.end()) {
        return false;
    }
    
    std::string passwordHash = PasswordUtil::hashPassword(password);
    users.emplace(username, User(username, passwordHash, role));
    return true;
}

bool UserManager::updateUser(const std::string& username, const User& updatedUser) {
    auto it = SearchUtil::mapFind(users, username);
    if (it == users.end()) {
        return false;
    }
    
    it->second = updatedUser;
    return true;
}

bool UserManager::deleteUser(const std::string& username) {
    auto it = SearchUtil::mapFind(users, username);
    if (it == users.end()) {
        return false;
    }
    
    users.erase(it);
    return true;
}

User* UserManager::findUser(const std::string& username) {
    auto it = SearchUtil::mapFind(users, username);
    if (it == users.end()) {
        return nullptr;
    }
    
    return &(it->second);
}

const User* UserManager::findUser(const std::string& username) const {
    auto it = SearchUtil::mapFind(users, username);
    if (it == users.end()) {
        return nullptr;
    }
    
    return &(it->second);
}

// Authentication
bool UserManager::login(const std::string& username, const std::string& password) {
    auto it = SearchUtil::mapFind(users, username);
    if (it == users.end()) {
        return false;
    }
    
    if (it->second.checkPassword(password)) {
        currentUser = &(it->second);
        return true;
    }
    
    return false;
}

void UserManager::logout() {
    currentUser = nullptr;
}

bool UserManager::isLoggedIn() const {
    return currentUser != nullptr;
}

User* UserManager::getCurrentUser() const {
    return currentUser;
}

// Password management
bool UserManager::changePassword(const std::string& username, 
                                 const std::string& oldPassword, 
                                 const std::string& newPassword) {
    User* user = findUser(username);
    if (!user) {
        return false;
    }
    
    if (!user->checkPassword(oldPassword)) {
        return false;
    }
    
    user->setNewPassword(newPassword);
    return true;
}

// Check permissions
bool UserManager::hasPermission(Role requiredRole) const {
    if (!isLoggedIn()) {
        return false;
    }
    
    Role userRole = currentUser->getRole();
    
    // Admin has all permissions
    if (userRole == Role::Admin) {
        return true;
    }
    
    // Staff can do staff-level operations
    if (userRole == Role::Staff && (requiredRole == Role::Staff || requiredRole == Role::Reader)) {
        return true;
    }
    
    // Reader can only do reader-level operations
    if (userRole == Role::Reader && requiredRole == Role::Reader) {
        return true;
    }
    
    return false;
}

// File operations
bool UserManager::loadFromFile(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        std::string jsonStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        auto j = SimpleJSON::parseJSON(jsonStr);
        
        users.clear();
        
        if (!j->isArray()) {
            return false;
        }
        
        const auto& jsonArray = j->getArray();
        for (const auto& userJsonPtr : jsonArray) {
            std::string username = userJsonPtr->at("username")->getString();
            std::string passwordHash = userJsonPtr->at("passwordHash")->getString();
            
            // Parse role as string instead of int
            Role role;
            if (userJsonPtr->at("role")->isString()) {
                // Handle string role format
                std::string roleStr = userJsonPtr->at("role")->getString();
                if (roleStr == "Admin") {
                    role = Role::Admin;
                } else if (roleStr == "Staff") {
                    role = Role::Staff;
                } else if (roleStr == "Reader") {
                    role = Role::Reader;
                } else {
                    // Default to Reader for unknown roles
                    role = Role::Reader;
                }
            } else {
                // Handle legacy integer role format
                role = static_cast<Role>(userJsonPtr->at("role")->getInt());
            }
            
            users.emplace(username, User(username, passwordHash, role));
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading users: " << e.what() << std::endl;
        return false;
    }
}

bool UserManager::saveToFile(const std::string& filename) const {
    try {
        auto j = SimpleJSON::JSONValue::createArray();
        
        for (const auto& pair : users) {
            const User& user = pair.second;
            auto userJson = SimpleJSON::JSONValue::createObject();
            
            userJson->set("username", user.getUsername());
            userJson->set("passwordHash", user.getPasswordHash());
            
            // Save role as string instead of integer
            userJson->set("role", user.getRoleName());
            
            j->push_back(userJson);
        }
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        file << SimpleJSON::stringifyJSON(j, 4);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving users: " << e.what() << std::endl;
        return false;
    }
}

// First-time setup
bool UserManager::isFirstRun() const {
    return users.empty();
}

bool UserManager::setupAdminAccount(const std::string& username, const std::string& password) {
    if (!isFirstRun()) {
        return false;
    }
    
    return addUser(username, password, Role::Admin);
}

// Get all users
const std::vector<User>& UserManager::getAllUsers() const {
    static std::vector<User> result;
    result.clear();
    result.reserve(users.size());
    
    for (const auto& pair : users) {
        result.push_back(pair.second);
    }
    
    return result;
}

// Display
void UserManager::displayAllUsers() const {
    std::cout << "===== User List =====" << std::endl;
    std::cout << std::left 
              << std::setw(15) << "Username" 
              << std::setw(10) << "Role" 
              << std::endl;
    std::cout << std::string(25, '-') << std::endl;
    
    for (const auto& pair : users) {
        pair.second.display();
    }
    
    std::cout << std::string(25, '-') << std::endl;
} 