#include "../include/QueryParser.h"
#include "../include/SearchUtil.h"
#include <iostream>
#include <cctype>

QueryNode::QueryNode(NodeType type) : type(type), term(""), left(nullptr), right(nullptr) {}

QueryNode::QueryNode(const std::string& term) : type(NodeType::TERM), term(term), left(nullptr), right(nullptr) {}

QueryNode::QueryNode(const std::string& field, FieldOperator op, const std::string& value) 
    : type(NodeType::FIELD_QUERY), term(""), field(field), fieldValue(value), fieldOp(op), left(nullptr), right(nullptr) {}

QueryParser::QueryParser() : pos(0) {}

std::shared_ptr<QueryNode> QueryParser::parse(const std::string& query) {
    this->query = query;
    pos = 0;
    return parseExpression();
}

// Expression = Term {OR Term}
std::shared_ptr<QueryNode> QueryParser::parseExpression() {
    std::shared_ptr<QueryNode> left = parseTerm();
    
    skipWhitespace();
    while (pos < query.length() && match("OR")) {
        auto node = std::make_shared<QueryNode>(NodeType::OR);
        node->left = left;
        node->right = parseTerm();
        left = node;
        skipWhitespace();
    }
    
    return left;
}

// Term = Factor {AND Factor}
std::shared_ptr<QueryNode> QueryParser::parseTerm() {
    std::shared_ptr<QueryNode> left = parseFactor();
    
    skipWhitespace();
    while (pos < query.length() && match("AND")) {
        auto node = std::make_shared<QueryNode>(NodeType::AND);
        node->left = left;
        node->right = parseFactor();
        left = node;
        skipWhitespace();
    }
    
    return left;
}

// Factor = [NOT] Atom
std::shared_ptr<QueryNode> QueryParser::parseFactor() {
    skipWhitespace();
    
    if (match("NOT")) {
        auto node = std::make_shared<QueryNode>(NodeType::NOT);
        node->left = parseAtom();
        return node;
    }
    
    return parseAtom();
}

// Atom = ID | "(" Expression ")" | FieldQuery
std::shared_ptr<QueryNode> QueryParser::parseAtom() {
    skipWhitespace();
    
    if (pos < query.length() && query[pos] == '(') {
        pos++; // 跳過 '('
        auto node = parseExpression();
        
        skipWhitespace();
        if (pos < query.length() && query[pos] == ')') {
            pos++; // 跳過 ')'
            return node;
        } else {
            std::cerr << "Error: Missing closing parenthesis" << std::endl;
            return nullptr;
        }
    }
    
    size_t savedPos = pos;
    std::string field = parseIdentifier();
    
    if (!field.empty() && pos < query.length()) {
        // 檢查下一個字元是否為欄位運算子（=, ~, >, < 等）
        skipWhitespace();
        if (pos < query.length() && (query[pos] == '=' || query[pos] == '~' || 
                                     query[pos] == '>' || query[pos] == '<')) {
            return parseFieldQuery(field);
        }
    }
    
    // 不是欄位查詢，恢復位置並解析為一般詞彙
    pos = savedPos;
    std::string term = parseIdentifier();
    if (term.empty()) {
        std::cerr << "Error: Expected identifier" << std::endl;
        return nullptr;
    }
    
    return std::make_shared<QueryNode>(term);
}

// 解析特定欄位查詢，如 "title=value" 或 "year>=2020"
std::shared_ptr<QueryNode> QueryParser::parseFieldQuery(const std::string& field) {
    FieldOperator op = parseFieldOperator();
    
    std::string value = parseFieldValue();
    if (value.empty()) {
        std::cerr << "Error: Expected field value" << std::endl;
        return nullptr;
    }
    
    return std::make_shared<QueryNode>(field, op, value);
}

// 解析欄位運算子（=, ~, >, <, >=, <=）
FieldOperator QueryParser::parseFieldOperator() {
    skipWhitespace();
    
    if (pos + 1 < query.length()) {
        if (query[pos] == '>' && query[pos + 1] == '=') {
            pos += 2;
            return FieldOperator::GREATER_EQ;
        }
        else if (query[pos] == '<' && query[pos + 1] == '=') {
            pos += 2;
            return FieldOperator::LESS_EQ;
        }
    }
    
    if (pos < query.length()) {
        char op = query[pos++];
        switch (op) {
            case '=': return FieldOperator::EQUALS;
            case '~': return FieldOperator::CONTAINS;
            case '>': return FieldOperator::GREATER;
            case '<': return FieldOperator::LESS;
            default:
                std::cerr << "Error: Expected field operator" << std::endl;
                pos--;
                return FieldOperator::EQUALS;
        }
    }
    
    return FieldOperator::EQUALS;
}

std::string QueryParser::parseFieldValue() {
    skipWhitespace();
    
    if (pos >= query.length()) {
        return "";
    }
    
    if (query[pos] == '"') {
        pos++;
        size_t start = pos;
        
        while (pos < query.length() && query[pos] != '"') {
            pos++;
        }
        
        if (pos >= query.length()) {
            std::cerr << "Error: Unterminated string literal" << std::endl;
            return "";
        }
        
        std::string result = query.substr(start, pos - start);
        pos++;
        return result;
    }
    
    size_t start = pos;
    while (pos < query.length() && 
           !std::isspace(query[pos]) && query[pos] != ')' && 
           query[pos] != '(' && query[pos] != '&' && query[pos] != '|') {
        pos++;
    }
    
    return query.substr(start, pos - start);
}

void QueryParser::skipWhitespace() {
    while (pos < query.length() && std::isspace(query[pos])) {
        pos++;
    }
}

std::string QueryParser::parseIdentifier() {
    skipWhitespace();
    
    if (pos >= query.length()) {
        return "";
    }
    
    if (query[pos] == '"') {
        pos++;
        size_t start = pos;

        while (pos < query.length() && query[pos] != '"') {
            pos++;
        }
        
        if (pos >= query.length()) {
            std::cerr << "Error: Unterminated string literal" << std::endl;
            return "";
        }
        
        std::string result = query.substr(start, pos - start);
        pos++;
        return result;
    }
    
    size_t start = pos;
    while (pos < query.length() && 
           (std::isalnum(query[pos]) || query[pos] == '_' || query[pos] == '-' || 
            query[pos] == '.' || 
            // Allow non-ASCII characters for Chinese/other languages
            (static_cast<unsigned char>(query[pos]) > 127))) {
        pos++;
    }
    
    return query.substr(start, pos - start);
}

bool QueryParser::match(const std::string& s) {
    skipWhitespace();
    
    if (pos + s.length() <= query.length()) {
        bool matches = true;
        for (size_t i = 0; i < s.length(); i++) {
            if (std::toupper(query[pos + i]) != std::toupper(s[i])) {
                matches = false;
                break;
            }
        }
        
        if (matches) {
            // Check that the keyword is followed by whitespace or end of string
            if (pos + s.length() == query.length() || 
                std::isspace(query[pos + s.length()])) {
                pos += s.length();
                skipWhitespace();
                return true;
            }
        }
    }
    
    return false;
}

std::unordered_set<int> QueryParser::evaluate(
    const std::shared_ptr<QueryNode>& node, 
    const std::unordered_map<std::string, std::unordered_set<int>>& invertedIndex,
    const std::unordered_set<int>& allBookIds) {
    
    if (!node) {
        return {};
    }
    
    switch (node->type) {
        case NodeType::TERM: {
            auto it = SearchUtil::mapFind(invertedIndex, node->term);
            if (it != invertedIndex.end()) {
                return it->second;
            }
            return {};
        }
        
        case NodeType::AND: {
            auto leftResults = evaluate(node->left, invertedIndex, allBookIds);
            auto rightResults = evaluate(node->right, invertedIndex, allBookIds);
            
            std::unordered_set<int> result;
            for (int id : leftResults) {
                if (rightResults.count(id) > 0) {
                    result.insert(id);
                }
            }
            return result;
        }
        
        case NodeType::OR: {
            auto leftResults = evaluate(node->left, invertedIndex, allBookIds);
            auto rightResults = evaluate(node->right, invertedIndex, allBookIds);
            
            std::unordered_set<int> result = leftResults;
            for (int id : rightResults) {
                result.insert(id);
            }
            return result;
        }
        
        case NodeType::NOT: {
            auto childResults = evaluate(node->left, invertedIndex, allBookIds);
            
            std::unordered_set<int> result;
            for (int id : allBookIds) {
                if (childResults.count(id) == 0) {
                    result.insert(id);
                }
            }
            return result;
        }
        
        case NodeType::FIELD_QUERY: {
            return {};
        }
        
        case NodeType::KEYWORD_QUERY: {
            return {};
        }
    }
    
    return {};
}

void QueryParser::printQueryTree(std::shared_ptr<QueryNode> node, int indent) {
    if (!node) return;
    
    std::string indentStr(indent * 2, ' ');
    
    switch (node->type) {
        case NodeType::TERM:
            std::cout << indentStr << "TERM: " << node->term << std::endl;
            break;
        case NodeType::AND:
            std::cout << indentStr << "AND" << std::endl;
            printQueryTree(node->left, indent + 1);
            printQueryTree(node->right, indent + 1);
            break;
        case NodeType::OR:
            std::cout << indentStr << "OR" << std::endl;
            printQueryTree(node->left, indent + 1);
            printQueryTree(node->right, indent + 1);
            break;
        case NodeType::NOT:
            std::cout << indentStr << "NOT" << std::endl;
            printQueryTree(node->left, indent + 1);
            break;
        case NodeType::FIELD_QUERY:
            std::cout << indentStr << "FIELD QUERY: " << node->field << " ";
            
            switch (node->fieldOp) {
                case FieldOperator::EQUALS: std::cout << "="; break;
                case FieldOperator::CONTAINS: std::cout << "~"; break;
                case FieldOperator::GREATER: std::cout << ">"; break;
                case FieldOperator::LESS: std::cout << "<"; break;
                case FieldOperator::GREATER_EQ: std::cout << ">="; break;
                case FieldOperator::LESS_EQ: std::cout << "<="; break;
            }
            
            std::cout << " \"" << node->fieldValue << "\"" << std::endl;
            break;
        case NodeType::KEYWORD_QUERY:
            std::cout << indentStr << "KEYWORD QUERY: " << node->term << std::endl;
            break;
    }
} 