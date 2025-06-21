#ifndef SIMPLE_JSON_H
#define SIMPLE_JSON_H

#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <memory>
#include <variant>
#include <stdexcept>
#include "../include/SortUtil.h"
#include "../include/SearchUtil.h"

namespace SimpleJSON {

    class JSONValue;

    enum class JSONType {
        Null,
        Boolean,
        Number,
        String,
        Array,
        Object
    };

    class JSONValue {
    private:
        JSONType type;
        std::variant<
            std::nullptr_t,
            bool,
            double,
            std::string,
            std::vector<std::shared_ptr<JSONValue>>,
            std::unordered_map<std::string, std::shared_ptr<JSONValue>>
        > data;

    public:
        JSONValue() : type(JSONType::Null), data(nullptr) {}
        JSONValue(std::nullptr_t) : type(JSONType::Null), data(nullptr) {}
        JSONValue(bool value) : type(JSONType::Boolean), data(value) {}
        JSONValue(int value) : type(JSONType::Number), data(static_cast<double>(value)) {}
        JSONValue(double value) : type(JSONType::Number), data(value) {}
        JSONValue(const std::string& value) : type(JSONType::String), data(value) {}
        JSONValue(const char* value) : type(JSONType::String), data(std::string(value)) {}

        // 建構陣列
        JSONValue(const std::vector<std::shared_ptr<JSONValue>>& array) : type(JSONType::Array), data(array) {}

        // 建構物件
        JSONValue(const std::unordered_map<std::string, std::shared_ptr<JSONValue>>& object) : type(JSONType::Object), data(object) {}

        // 取得類型
        JSONType getType() const { return type; }

        // 類型檢查
        bool isNull() const { return type == JSONType::Null; }
        bool isBoolean() const { return type == JSONType::Boolean; }
        bool isNumber() const { return type == JSONType::Number; }
        bool isString() const { return type == JSONType::String; }
        bool isArray() const { return type == JSONType::Array; }
        bool isObject() const { return type == JSONType::Object; }

        // 取值方法
        bool getBool() const {
            if (!isBoolean()) throw std::runtime_error("Not a boolean");
            return std::get<bool>(data);
        }

        double getNumber() const {
            if (!isNumber()) throw std::runtime_error("Not a number");
            return std::get<double>(data);
        }

        int getInt() const {
            if (!isNumber()) throw std::runtime_error("Not a number");
            return static_cast<int>(std::get<double>(data));
        }

        std::string getString() const {
            if (!isString()) throw std::runtime_error("Not a string");
            return std::get<std::string>(data);
        }

        const std::vector<std::shared_ptr<JSONValue>>& getArray() const {
            if (!isArray()) throw std::runtime_error("Not an array");
            return std::get<std::vector<std::shared_ptr<JSONValue>>>(data);
        }

        const std::unordered_map<std::string, std::shared_ptr<JSONValue>>& getObject() const {
            if (!isObject()) throw std::runtime_error("Not an object");
            return std::get<std::unordered_map<std::string, std::shared_ptr<JSONValue>>>(data);
        }

        // 陣列和物件存取
        std::shared_ptr<JSONValue> at(size_t index) const {
            if (!isArray()) throw std::runtime_error("Not an array");
            const auto& array = std::get<std::vector<std::shared_ptr<JSONValue>>>(data);
            if (index >= array.size()) throw std::out_of_range("Array index out of range");
            return array[index];
        }

        std::shared_ptr<JSONValue> at(const std::string& key) const {
            if (!isObject()) throw std::runtime_error("Not an object");
            const auto& object = std::get<std::unordered_map<std::string, std::shared_ptr<JSONValue>>>(data);
            auto it = SearchUtil::mapFind(object, key);
            if (it == object.end()) throw std::out_of_range("Object key not found");
            return it->second;
        }

        bool contains(const std::string& key) const {
            if (!isObject()) return false;
            const auto& object = std::get<std::unordered_map<std::string, std::shared_ptr<JSONValue>>>(data);
            return SearchUtil::mapContains(object, key);
        }

        static std::shared_ptr<JSONValue> createObject() {
            std::unordered_map<std::string, std::shared_ptr<JSONValue>> object;
            return std::make_shared<JSONValue>(object);
        }

        static std::shared_ptr<JSONValue> createArray() {
            std::vector<std::shared_ptr<JSONValue>> array;
            return std::make_shared<JSONValue>(array);
        }

        void set(const std::string& key, std::shared_ptr<JSONValue> value) {
            if (!isObject()) {
                throw std::runtime_error("Not an object");
            }
            auto& object = std::get<std::unordered_map<std::string, std::shared_ptr<JSONValue>>>(data);
            object[key] = value;
        }

        void push_back(std::shared_ptr<JSONValue> value) {
            if (!isArray()) {
                throw std::runtime_error("Not an array");
            }
            auto& array = std::get<std::vector<std::shared_ptr<JSONValue>>>(data);
            array.push_back(value);
        }

        void set(const std::string& key, bool value) {
            set(key, std::make_shared<JSONValue>(value));
        }

        void set(const std::string& key, int value) {
            set(key, std::make_shared<JSONValue>(value));
        }

        void set(const std::string& key, double value) {
            set(key, std::make_shared<JSONValue>(value));
        }

        void set(const std::string& key, const std::string& value) {
            set(key, std::make_shared<JSONValue>(value));
        }

        void push_back(bool value) {
            push_back(std::make_shared<JSONValue>(value));
        }

        void push_back(int value) {
            push_back(std::make_shared<JSONValue>(value));
        }

        void push_back(double value) {
            push_back(std::make_shared<JSONValue>(value));
        }

        void push_back(const std::string& value) {
            push_back(std::make_shared<JSONValue>(value));
        }

        std::string stringify(int indent = 0) const {
            std::ostringstream oss;
            switch (type) {
            case JSONType::Null:
                oss << "null";
                break;
            case JSONType::Boolean:
                oss << (std::get<bool>(data) ? "true" : "false");
                break;
            case JSONType::Number: {
                double number = std::get<double>(data);
                // 檢查是否為整數
                if (number == static_cast<int>(number)) {
                    oss << static_cast<int>(number);
                }
                else {
                    oss << number;
                }
                break;
            }
            case JSONType::String:
                oss << "\"" << escapeString(std::get<std::string>(data)) << "\"";
                break;
            case JSONType::Array: {
                const auto& array = std::get<std::vector<std::shared_ptr<JSONValue>>>(data);
                oss << "[";
                if (indent > 0 && !array.empty()) {
                    oss << "\n";
                }
                for (size_t i = 0; i < array.size(); ++i) {
                    if (indent > 0) {
                        oss << std::string(indent + 2, ' ');
                    }
                    oss << array[i]->stringify(indent > 0 ? indent + 2 : 0);
                    if (i < array.size() - 1) {
                        oss << ",";
                        if (indent > 0) {
                            oss << "\n";
                        }
                    }
                }
                if (indent > 0 && !array.empty()) {
                    oss << "\n" << std::string(indent, ' ');
                }
                oss << "]";
                break;
            }
            case JSONType::Object: {
                const auto& object = std::get<std::unordered_map<std::string, std::shared_ptr<JSONValue>>>(data);
                oss << "{";
                if (indent > 0 && !object.empty()) {
                    oss << "\n";
                }

                // 轉換為向量以確保穩定的迭代順序
                std::vector<std::pair<std::string, std::shared_ptr<JSONValue>>> sortedObject;
                for (const auto& pair : object) {
                    sortedObject.push_back(pair);
                }

                SortUtil::sort(sortedObject, [](const auto& a, const auto& b) {
                    return a.first < b.first;
                    });

                for (size_t i = 0; i < sortedObject.size(); ++i) {
                    if (indent > 0) {
                        oss << std::string(indent + 2, ' ');
                    }
                    oss << "\"" << escapeString(sortedObject[i].first) << "\":";
                    if (indent > 0) {
                        oss << " ";
                    }
                    oss << sortedObject[i].second->stringify(indent > 0 ? indent + 2 : 0);
                    if (i < sortedObject.size() - 1) {
                        oss << ",";
                        if (indent > 0) {
                            oss << "\n";
                        }
                    }
                }
                if (indent > 0 && !object.empty()) {
                    oss << "\n" << std::string(indent, ' ');
                }
                oss << "}";
                break;
            }
            }
            return oss.str();
        }

    private:
        // 跳脫字串中的特殊字元
        std::string escapeString(const std::string& str) const {
            std::ostringstream oss;
            for (char c : str) {
                switch (c) {
                case '\"': oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '\b': oss << "\\b"; break;
                case '\f': oss << "\\f"; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;
                default:
                    if (c >= 0 && c < 32) {
                        oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                    }
                    else {
                        oss << c;
                    }
                }
            }
            return oss.str();
        }
    };

    namespace detail {

        inline void skipWS(const std::string& s, size_t& i) {
            while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
        }

        inline bool match(const std::string& s, size_t& i, const char* kw) {
            size_t j = 0;
            while (kw[j] && i + j < s.size() && s[i + j] == kw[j]) ++j;
            if (kw[j] == '\0') { i += j; return true; }
            return false;
        }

        class Parser {
        public:
            explicit Parser(const std::string& src) : text(src), idx(0) {}

            std::shared_ptr<JSONValue> parse() {
                skipWS(text, idx);
                auto val = parseValue();
                skipWS(text, idx);
                if (idx != text.size())
                    throw std::runtime_error("Trailing characters after JSON");
                return val;
            }

        private:
            const std::string& text;
            size_t             idx;

            std::shared_ptr<JSONValue> parseValue() {
                if (idx >= text.size())
                    throw std::runtime_error("Unexpected end of JSON");

                char c = text[idx];
                if (c == '"')  return parseString();
                if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parseNumber();
                if (c == 't' || c == 'f') return parseBool();
                if (c == 'n') return parseNull();
                if (c == '[') return parseArray();
                if (c == '{') return parseObject();

                throw std::runtime_error("Invalid JSON syntax");
            }

            std::shared_ptr<JSONValue> parseNull() {
                if (!match(text, idx, "null"))
                    throw std::runtime_error("Invalid token (want null)");
                return std::make_shared<JSONValue>(); // default = null
            }

            std::shared_ptr<JSONValue> parseBool() {
                if (match(text, idx, "true"))  return std::make_shared<JSONValue>(true);
                if (match(text, idx, "false")) return std::make_shared<JSONValue>(false);
                throw std::runtime_error("Invalid token (want true/false)");
            }

            std::shared_ptr<JSONValue> parseNumber() {
                size_t start = idx;
                if (text[idx] == '-') ++idx;
                while (idx < text.size() && std::isdigit(static_cast<unsigned char>(text[idx]))) ++idx;
                bool isInt = true;
                if (idx < text.size() && text[idx] == '.') { // 小數
                    isInt = false;
                    ++idx;
                    while (idx < text.size() && std::isdigit(static_cast<unsigned char>(text[idx]))) ++idx;
                }
                double num = std::stod(text.substr(start, idx - start));
                if (isInt) return std::make_shared<JSONValue>(static_cast<int>(num));
                return std::make_shared<JSONValue>(num);
            }

            std::shared_ptr<JSONValue> parseString() {
                if (text[idx] != '"') throw std::runtime_error("Expect '\"'");
                ++idx;
                std::string out;
                while (idx < text.size()) {
                    char c = text[idx++];
                    if (c == '"') break;
                    if (c == '\\') { // 處理跳脫
                        if (idx >= text.size()) throw std::runtime_error("Bad escape");
                        char esc = text[idx++];
                        switch (esc) {
                        case '"':  out += '"';  break;
                        case '\\': out += '\\'; break;
                        case '/':  out += '/';  break;
                        case 'b':  out += '\b'; break;
                        case 'f':  out += '\f'; break;
                        case 'n':  out += '\n'; break;
                        case 'r':  out += '\r'; break;
                        case 't':  out += '\t'; break;
                        default: throw std::runtime_error("Unsupported escape");
                        }
                    }
                    else {
                        out += c;
                    }
                }
                return std::make_shared<JSONValue>(out);
            }

            std::shared_ptr<JSONValue> parseArray() {
                if (text[idx] != '[') throw std::runtime_error("Expect '['");
                ++idx;
                auto arr = std::vector<std::shared_ptr<JSONValue>>{};
                skipWS(text, idx);
                if (text[idx] == ']') { ++idx; return std::make_shared<JSONValue>(arr); }
                while (true) {
                    arr.push_back(parseValue());
                    skipWS(text, idx);
                    if (text[idx] == ']') { ++idx; break; }
                    if (text[idx] != ',') throw std::runtime_error("Expect ',' in array");
                    ++idx;
                    skipWS(text, idx);
                }
                return std::make_shared<JSONValue>(arr);
            }

            std::shared_ptr<JSONValue> parseObject() {
                if (text[idx] != '{') throw std::runtime_error("Expect '{'");
                ++idx;
                auto obj = std::unordered_map<std::string, std::shared_ptr<JSONValue>>{};
                skipWS(text, idx);
                if (text[idx] == '}') { ++idx; return std::make_shared<JSONValue>(obj); }
                while (true) {
                    auto keyPtr = parseString();
                    std::string key = keyPtr->getString();
                    skipWS(text, idx);
                    if (text[idx] != ':') throw std::runtime_error("Expect ':' after key");
                    ++idx;
                    skipWS(text, idx);
                    obj[key] = parseValue();
                    skipWS(text, idx);
                    if (text[idx] == '}') { ++idx; break; }
                    if (text[idx] != ',') throw std::runtime_error("Expect ',' in object");
                    ++idx;
                    skipWS(text, idx);
                }
                return std::make_shared<JSONValue>(obj);
            }
        };
    } // namespace detail

    inline std::shared_ptr<JSONValue> parseJSON(const std::string& text) {
        return detail::Parser(text).parse();
    }

    inline std::string stringifyJSON(const std::shared_ptr<JSONValue>& v, int indent = 0) {
        return v ? v->stringify(indent) : "null";
    }

}
#endif // SIMPLE_JSON_H  