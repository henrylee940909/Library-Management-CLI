#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include <string>
#include <vector>
#include <unordered_set>
#include <memory>
#include <unordered_map>
#include "Book.h"

// Query node for boolean expression tree
enum class NodeType {
    TERM,
    AND,
    OR,
    NOT,
    FIELD_QUERY,
    KEYWORD_QUERY
};

enum class FieldOperator {
    EQUALS,      // =
    CONTAINS,    // ~
    GREATER,     // >
    LESS,        // <
    GREATER_EQ,  // >=
    LESS_EQ      // <=
};

struct QueryNode {
    NodeType type;
    std::string term;
    std::string field;
    std::string fieldValue;
    FieldOperator fieldOp;
    std::shared_ptr<QueryNode> left;
    std::shared_ptr<QueryNode> right;
    
    QueryNode(NodeType type);
    QueryNode(const std::string& term);
    QueryNode(const std::string& field, FieldOperator op, const std::string& value);
};

class QueryParser {
private:
    std::string query;
    size_t pos;
    
    std::unordered_map<std::string, std::string> fieldQueries;
    
    std::shared_ptr<QueryNode> parseExpression();
    std::shared_ptr<QueryNode> parseTerm();
    std::shared_ptr<QueryNode> parseFactor();
    std::shared_ptr<QueryNode> parseAtom();
    std::shared_ptr<QueryNode> parseFieldQuery(const std::string& field);
    
    QueryParser(const std::unordered_map<std::string, std::string>& fields) 
        : pos(0), fieldQueries(fields) {}
    
    std::shared_ptr<QueryNode> parseYearQuery(const std::string& value);
    std::shared_ptr<QueryNode> parseAuthorQuery(const std::string& value);
    std::shared_ptr<QueryNode> parseTitleQuery(const std::string& value);
    std::shared_ptr<QueryNode> parseCategoryQuery(const std::string& value);
    
    void skipWhitespace();
    bool match(const std::string& expected);
    std::string parseIdentifier();
    FieldOperator parseFieldOperator();
    std::string parseFieldValue();

public:
    QueryParser();
    
    std::shared_ptr<QueryNode> parse(const std::string& query);
    
    static std::unordered_set<int> evaluate(const std::shared_ptr<QueryNode>& node, 
                                           const std::unordered_map<std::string, std::unordered_set<int>>& invertedIndex,
                                           const std::unordered_set<int>& allBookIds);
    
    void printQueryTree(std::shared_ptr<QueryNode> node, int indent = 0);
};

namespace QueryMatcher {
    std::string toLower(const std::string& str);
    bool matchString(const std::string& text, FieldOperator op, const std::string& value, bool ignoreCase = true);
    bool matchNumber(int number, FieldOperator op, const std::string& value);
}

#endif // QUERY_PARSER_H 