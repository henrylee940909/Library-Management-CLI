#include "../include/QueryParser.h"
#include "../include/SortUtil.h"
#include "../include/SearchUtil.h"
#include <cctype>

std::string QueryMatcher::toLower(const std::string& str) {
    std::string result = str;
    SortUtil::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

bool QueryMatcher::matchString(const std::string& text, FieldOperator op, 
                              const std::string& value, bool ignoreCase) {
    std::string compareValue = ignoreCase ? toLower(text) : text;
    std::string compareQuery = ignoreCase ? toLower(value) : value;
    
    switch (op) {
        case FieldOperator::EQUALS:
            return compareValue == compareQuery;
            
        case FieldOperator::CONTAINS:
            return SearchUtil::contains(compareValue, compareQuery);
            
        case FieldOperator::GREATER:
            return compareValue > compareQuery;
            
        case FieldOperator::LESS:
            return compareValue < compareQuery;
            
        case FieldOperator::GREATER_EQ:
            return compareValue >= compareQuery;
            
        case FieldOperator::LESS_EQ:
            return compareValue <= compareQuery;
            
        default:
            return false;
    }
}

bool QueryMatcher::matchNumber(int number, FieldOperator op, const std::string& value) {
    int queryValue;
    try {
        queryValue = std::stoi(value);
    } catch (const std::exception&) {
        return false;
    }
    
    switch (op) {
        case FieldOperator::EQUALS:
            return number == queryValue;
            
        case FieldOperator::GREATER:
            return number > queryValue;
            
        case FieldOperator::LESS:
            return number < queryValue;
            
        case FieldOperator::GREATER_EQ:
            return number >= queryValue;
            
        case FieldOperator::LESS_EQ:
            return number <= queryValue;
            
        case FieldOperator::CONTAINS:
            return false; 
            
        default:
            return false;
    }
} 