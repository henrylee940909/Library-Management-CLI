#include "../include/TextUtils.h"
#include <cctype>

int TextUtils::getUTF8CharLength(unsigned char firstByte) {
    if ((firstByte & 0x80) == 0) return 1;      // ASCII
    if ((firstByte & 0xE0) == 0xC0) return 2;   // 2-byte
    if ((firstByte & 0xF0) == 0xE0) return 3;   // 3-byte
    if ((firstByte & 0xF8) == 0xF0) return 4;   // 4-byte
    return 1; // 預設
}

std::string TextUtils::extractUTF8Char(const std::string& text, size_t& pos) {
    int charLen = getUTF8CharLength(text[pos]);
    if (pos + charLen <= text.size()) {
        std::string result = text.substr(pos, charLen);
        pos += charLen;
        return result;
    }
    pos++;
    return "";
}

void TextUtils::processASCIIChar(char c, std::string& currentToken, std::vector<std::string>& tokens) {
    if (std::isalnum(c) || c == '_') {
        currentToken += std::tolower(c);
    } else {
        if (!currentToken.empty()) {
            tokens.push_back(currentToken);
            currentToken.clear();
        }
    }
}

std::vector<std::string> TextUtils::tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string currentToken;
    
    for (size_t i = 0; i < text.size(); ) {
        unsigned char c = text[i];
        
        if ((c & 0x80) == 0) {
            // ASCII 字元
            processASCIIChar(c, currentToken, tokens);
            i++;
        } else {
            // Unicode 字元
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
            
            std::string unicodeChar = extractUTF8Char(text, i);
            if (!unicodeChar.empty()) {
                tokens.push_back(unicodeChar);
            }
        }
    }
    
    if (!currentToken.empty()) {
        tokens.push_back(currentToken);
    }
    
    return removeDuplicates(tokens);
}

std::vector<std::string> TextUtils::removeDuplicates(const std::vector<std::string>& tokens) {
    std::unordered_set<std::string> uniqueSet(tokens.begin(), tokens.end());
    return std::vector<std::string>(uniqueSet.begin(), uniqueSet.end());
}

std::unordered_set<std::string> TextUtils::extractBookTerms(const std::string& title,
                                                          const std::string& author,
                                                          const std::string& synopsis,
                                                          const std::vector<std::string>& categories) {
    std::unordered_set<std::string> allTerms;
    
    // 從標題取得詞彙
    auto titleTokens = tokenize(title);
    allTerms.insert(titleTokens.begin(), titleTokens.end());
    
    // 從作者取得詞彙
    auto authorTokens = tokenize(author);
    allTerms.insert(authorTokens.begin(), authorTokens.end());
    
    // 從簡介取得詞彙
    auto synopsisTokens = tokenize(synopsis);
    allTerms.insert(synopsisTokens.begin(), synopsisTokens.end());
    
    // 從分類取得詞彙
    for (const auto& category : categories) {
        auto categoryTokens = tokenize(category);
        allTerms.insert(categoryTokens.begin(), categoryTokens.end());
    }
    
    return allTerms;
} 