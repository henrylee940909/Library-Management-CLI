#ifndef PASSWORD_UTIL_H
#define PASSWORD_UTIL_H

#include <string>

namespace PasswordUtil {
    std::string hashPassword(const std::string& password);
    
    bool verifyPassword(const std::string& password, const std::string& hash);
    
    std::string generateSalt(size_t length = 16);
    
    std::string getPasswordInput(const std::string& prompt);
}

#endif // PASSWORD_UTIL_H 