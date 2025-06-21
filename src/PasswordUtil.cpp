#include "../include/PasswordUtil.h"
#include "../include/SearchUtil.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <random>

#ifdef _WIN32
#   include <conio.h>     // _getch()
#else
#   include <termios.h>   // tcgetattr / tcsetattr
#   include <unistd.h>
#endif

// Simple SHA-256 implementation (VERY simplified; use a real lib in prod)
std::string sha256(const std::string& str) {
    std::hash<std::string> hasher;
    size_t hash = hasher(str);
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hash;
    return ss.str();
}

namespace PasswordUtil {

    /* ---------- �H������ salt ---------- */
    std::string generateSalt(size_t length) {
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);

        std::string salt;
        salt.reserve(length);
        for (size_t i = 0; i < length; ++i) salt += alphanum[dis(gen)];
        return salt;
    }

    /* ---------- �[�Q���� ---------- */
    std::string hashPassword(const std::string& password) {
        std::string salt = generateSalt();
        std::string salted = salt + password;
        return salt + ":" + sha256(salted);      // SALT:HASH
    }

    /* ---------- ���� ---------- */
    bool verifyPassword(const std::string& password, const std::string& stored) {
        size_t colon = stored.find(':');
        if (colon == std::string::npos) return false;

        std::string salt = stored.substr(0, colon);
        std::string hash = stored.substr(colon + 1);
        return sha256(salt + password) == hash;
    }

    /* ---------- Ū���K�X�]���ÿ�J�^ ---------- */
    std::string getPasswordInput(const std::string& prompt) {
        std::cout << prompt << " ";

#ifdef _WIN32
        std::string pwd;
        char ch;
        while ((ch = _getch()) != '\r') {        // Enter
            if (ch == '\b') {                    // Backspace
                if (!pwd.empty()) {
                    pwd.pop_back();
                    std::cout << "\b \b";
                }
            }
            else {
                pwd.push_back(ch);
                std::cout << '*';
            }
        }
        std::cout << std::endl;
        return pwd;

#else   // POSIX �t��

        termios oldt;
        tcgetattr(STDIN_FILENO, &oldt);
        termios newt = oldt;
        newt.c_lflag &= ~ECHO;                   // ���� echo
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        std::string pwd;
        std::getline(std::cin, pwd);

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // �٭�
        std::cout << std::endl;
        return pwd;
#endif
    }

} // namespace PasswordUtil
