#ifndef TEXTUTILS_H
#define TEXTUTILS_H

#include <string>
#include <vector>
#include <unordered_set>

class TextUtils {
public:
    static std::vector<std::string> tokenize(const std::string& text);
    
    static std::vector<std::string> removeDuplicates(const std::vector<std::string>& tokens);
    
    static std::unordered_set<std::string> extractBookTerms(const std::string& title,
                                                           const std::string& author,
                                                           const std::string& synopsis,
                                                           const std::vector<std::string>& categories);

private:
    static int getUTF8CharLength(unsigned char firstByte);
    static std::string extractUTF8Char(const std::string& text, size_t& pos);
    static void processASCIIChar(char c, std::string& currentToken, std::vector<std::string>& tokens);
};

#endif // TEXTUTILS_H 