#include "../include/BookManager.h"
#include "../include/SortUtil.h"
#include "../include/QueryParser.h"
#include "../include/SearchUtil.h"
#include <fstream>
#include <iostream>
#include <cctype>
#include "../include/SimpleJSON.h"

using JSONValue = SimpleJSON::JSONValue;

BookManager::BookManager() : nextId(1) {}

bool BookManager::addBook(Book& book) {
    if (book.getId() == 0) {
        book.setId(nextId++);
    }
    else if (getBook(book.getId()) != nullptr) {
        return false;
    }
    else {
        nextId = std::max(nextId, book.getId() + 1);
    }

    books.push_back(book);
    bookIdMap[book.getId()] = books.size() - 1;

    updateBookIndex(book.getId(), book);

    return true;
}

bool BookManager::updateBook(const Book& book) {
    auto it = SearchUtil::mapFind(bookIdMap, book.getId());
    if (it == bookIdMap.end()) {
        return false;
    }

    removeFromIndex(book.getId());
    removeFromTitleIndex(book.getId());

    books[it->second] = book;
    updateBookIndex(book.getId(), book);

    return true;
}

bool BookManager::deleteBook(int bookId) {
    auto it = SearchUtil::mapFind(bookIdMap, bookId);
    if (it == bookIdMap.end()) {
        return false;
    }

    removeFromIndex(bookId);
    removeFromTitleIndex(bookId);

    int index = it->second;
    books.erase(books.begin() + index);

    // 重新建構 bookIdMap
    rebuildBookIdMap();

    return true;
}

Book* BookManager::getBook(int bookId) {
    auto it = SearchUtil::mapFind(bookIdMap, bookId);
    if (it == bookIdMap.end()) {
        return nullptr;
    }

    return &books[it->second];
}

const Book* BookManager::getBook(int bookId) const {
    auto it = SearchUtil::mapFind(bookIdMap, bookId);
    if (it == bookIdMap.end()) {
        return nullptr;
    }

    return &books[it->second];
}

bool BookManager::borrowBook(int bookId) {
    Book* book = getBook(bookId);
    if (!book) {
        return false;
    }

    return book->borrow();
}

bool BookManager::returnBook(int bookId) {
    Book* book = getBook(bookId);
    if (!book) {
        return false;
    }

    return book->returnBook();
}

// Tokenize text into words
std::vector<std::string> BookManager::tokenize(const std::string& text) const {
    std::vector<std::string> tokens;
    std::string token;

    for (char c : text) {
        if (std::isalnum(c) || c == '_') {
            token += std::tolower(c);
        }
        else if (!token.empty()) {
            tokens.push_back(token);
            token.clear();
        }
    }

    if (!token.empty()) {
        tokens.push_back(token);
    }

    return tokens;
}

void BookManager::buildInvertedIndex() {
    invertedIndex.clear();

    for (const auto& book : books) {
        auto titleTokens = tokenize(book.getTitle());
        for (const auto& token : titleTokens) {
            invertedIndex[token].insert(book.getId());
        }

        auto authorTokens = tokenize(book.getAuthor());
        for (const auto& token : authorTokens) {
            invertedIndex[token].insert(book.getId());
        }

        for (const auto& category : book.getCategories()) {
            auto categoryTokens = tokenize(category);
            for (const auto& token : categoryTokens) {
                invertedIndex[token].insert(book.getId());
            }
        }

        auto synopsisTokens = tokenize(book.getSynopsis());
        for (const auto& token : synopsisTokens) {
            invertedIndex[token].insert(book.getId());
        }
    }
}

void BookManager::buildTitleIndex() {
    titleIndex.clear();

    for (const auto& book : books) {
        // Add terms from title only
        auto titleTokens = tokenize(book.getTitle());
        for (const auto& token : titleTokens) {
            titleIndex[token].insert(book.getId());
        }
    }
}

void BookManager::addToIndex(int bookId, const std::string& term) {
    auto tokens = tokenize(term);
    for (const auto& token : tokens) {
        invertedIndex[token].insert(bookId);
    }
}

void BookManager::addToTitleIndex(int bookId, const std::string& term) {
    auto tokens = tokenize(term);
    for (const auto& token : tokens) {
        titleIndex[token].insert(bookId);
    }
}

void BookManager::removeFromIndex(int bookId) {
    for (auto& pair : invertedIndex) {
        pair.second.erase(bookId);
    }
}

void BookManager::removeFromTitleIndex(int bookId) {
    for (auto& pair : titleIndex) {
        pair.second.erase(bookId);
    }
}

std::vector<Book*> BookManager::searchBooks(const std::string& query) const {
    std::vector<Book*> results;

    if (query.empty()) {
        return results;
    }

    for (auto& book : books) {
        if (const_cast<Book&>(book).matchesKeyword(query)) {
            results.push_back(const_cast<Book*>(&book));
        }
    }

    return results;
}

std::vector<Book*> BookManager::filterByYear(int year, const std::string& op) const {
    std::vector<Book*> results;

    for (auto& book : books) {
        if (const_cast<Book&>(book).matchesYear(year, op)) {
            results.push_back(const_cast<Book*>(&book));
        }
    }

    return results;
}

std::vector<Book*> BookManager::filterByCategory(const std::string& category) const {
    std::vector<Book*> results;

    for (auto& book : books) {
        if (const_cast<Book&>(book).matchesCategory(category)) {
            results.push_back(const_cast<Book*>(&book));
        }
    }

    return results;
}

// Get all books
const std::vector<Book>& BookManager::getAllBooks() const {
    return books;
}

// File operations
bool BookManager::loadFromFile(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        std::string jsonStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        auto j = SimpleJSON::parseJSON(jsonStr);
        
        books.clear();
        bookIdMap.clear();
        invertedIndex.clear();
        titleIndex.clear();
        nextId = 1;
        
        if (!j->isArray()) {
            return false;
        }
        
        const auto& jsonArray = j->getArray();
        for (const auto& bookJsonPtr : jsonArray) {
            Book book;
            
            book.setId(bookJsonPtr->at("id")->getInt());
            book.setTitle(bookJsonPtr->at("title")->getString());
            book.setAuthor(bookJsonPtr->at("author")->getString());
            book.setYear(bookJsonPtr->at("year")->getInt());
            book.setTotalCopies(bookJsonPtr->at("totalCopies")->getInt());
            book.setAvailableCopies(bookJsonPtr->at("availableCopies")->getInt());
            
            if (bookJsonPtr->contains("isbn")) {
                book.setIsbn(bookJsonPtr->at("isbn")->getString());
            }
            
            if (bookJsonPtr->contains("publisher")) {
                book.setPublisher(bookJsonPtr->at("publisher")->getString());
            }
            
            if (bookJsonPtr->contains("language")) {
                book.setLanguage(bookJsonPtr->at("language")->getString());
            }
            
            if (bookJsonPtr->contains("pageCount")) {
                book.setPageCount(bookJsonPtr->at("pageCount")->getInt());
            }
            
            if (bookJsonPtr->contains("synopsis")) {
                book.setSynopsis(bookJsonPtr->at("synopsis")->getString());
            }
            
            if (bookJsonPtr->contains("categories")) {
                const auto& categories = bookJsonPtr->at("categories")->getArray();
                for (const auto& category : categories) {
                    book.addCategory(category->getString());
                }
            }
            
            books.push_back(book);
            bookIdMap[book.getId()] = books.size() - 1;
            
            // Update nextId
            nextId = std::max(nextId, book.getId() + 1);
        }
        
        // Build index
        buildInvertedIndex();
        buildTitleIndex();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading books: " << e.what() << std::endl;
        return false;
    }
}

bool BookManager::saveToFile(const std::string& filename) const {
    try {
        auto j = SimpleJSON::JSONValue::createArray();
        
        for (const auto& book : books) {
            auto bookJson = SimpleJSON::JSONValue::createObject();
            
            bookJson->set("id", book.getId());
            bookJson->set("title", book.getTitle());
            bookJson->set("author", book.getAuthor());
            bookJson->set("year", book.getYear());
            bookJson->set("totalCopies", book.getTotalCopies());
            bookJson->set("availableCopies", book.getAvailableCopies());
            
            if (!book.getIsbn().empty()) {
                bookJson->set("isbn", book.getIsbn());
            }
            
            if (!book.getPublisher().empty()) {
                bookJson->set("publisher", book.getPublisher());
            }
            
            if (!book.getLanguage().empty()) {
                bookJson->set("language", book.getLanguage());
            }
            
            if (book.getPageCount() > 0) {
                bookJson->set("pageCount", book.getPageCount());
            }
            
            if (!book.getSynopsis().empty()) {
                bookJson->set("synopsis", book.getSynopsis());
            }
            
            if (!book.getCategories().empty()) {
                auto categoriesJson = SimpleJSON::JSONValue::createArray();
                for (const auto& category : book.getCategories()) {
                    categoriesJson->push_back(category);
                }
                bookJson->set("categories", categoriesJson);
            }
            
            j->push_back(bookJson);
        }
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        file << SimpleJSON::stringifyJSON(j, 4);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving books: " << e.what() << std::endl;
        return false;
    }
}

// Statistics and visualization
int BookManager::getTotalBooks() const {
    return books.size();
}

std::unordered_map<std::string, int> BookManager::getCategoryStats() const {
    std::unordered_map<std::string, int> stats;
    
    for (const auto& book : books) {
        for (const auto& category : book.getCategories()) {
            stats[category]++;
        }
    }
    
    return stats;
}

// Display
void BookManager::displayAllBooks() const {
    std::cout << "===== Book List =====" << std::endl;
    std::cout << "Total books: " << books.size() << std::endl;
    std::cout << std::string(20, '=') << std::endl;
    
    for (const auto& book : books) {
        book.displaySummary();
    }
    
    std::cout << std::string(20, '=') << std::endl;
}

void BookManager::displayBooksByCategory() const {
    auto stats = getCategoryStats();
    
    std::cout << "===== Books by Category =====" << std::endl;
    
    // Group books by category
    std::unordered_map<std::string, std::vector<const Book*>> booksByCategory;
    
    for (const auto& book : books) {
        for (const auto& category : book.getCategories()) {
            booksByCategory[category].push_back(&book);
        }
    }
    
    // Display each category
    for (const auto& pair : booksByCategory) {
        std::cout << "\n--- " << pair.first << " (" << pair.second.size() << " books) ---\n";
        
        for (const auto* book : pair.second) {
            book->displaySummary();
        }
    }
    
    std::cout << std::string(30, '=') << std::endl;
}

void BookManager::displayBooksByYear() const {
    std::cout << "===== Books by Year =====" << std::endl;
    
    // Group books by year
    std::unordered_map<int, std::vector<const Book*>> booksByYear;
    
    for (const auto& book : books) {
        booksByYear[book.getYear()].push_back(&book);
    }
    
    // Get sorted years
    std::vector<int> years;
    for (const auto& pair : booksByYear) {
        years.push_back(pair.first);
    }
    
    // Sort years
    SortUtil::sort(years);
    
    // Display each year
    for (int year : years) {
        std::cout << "\n--- " << year << " (" << booksByYear[year].size() << " books) ---\n";
        
        for (const auto* book : booksByYear[year]) {
            book->displaySummary();
        }
    }
    
    std::cout << std::string(30, '=') << std::endl;
}

// Advanced search (parse and evaluate boolean expressions)
std::vector<Book*> BookManager::advancedSearch(const std::string& query) const {
    QueryParser parser;
    std::vector<Book*> results;
    
    // Parse the query
    auto root = parser.parse(query);
    if (!root) {
        std::cerr << "Error parsing query" << std::endl;
        return results;
    }
    
    // Create a set of all book IDs
    std::unordered_set<int> allBookIds;
    for (const auto& pair : bookIdMap) {
        allBookIds.insert(pair.first);
    }
    
    // Create a custom evaluate function that uses our field-specific query handler
    std::function<std::unordered_set<int>(std::shared_ptr<QueryNode>)> evaluateNode = 
        [this, &evaluateNode, &allBookIds](std::shared_ptr<QueryNode> node) -> std::unordered_set<int> {
            if (!node) {
                return {};
            }
            
            switch (node->type) {
                case NodeType::TERM: {
                    // 沒有指定欄位的查詢，當作標題包含搜尋處理
                    return searchInTitle(node->term);
                }
                
                case NodeType::AND: {
                    auto leftResults = evaluateNode(node->left);
                    auto rightResults = evaluateNode(node->right);
                    
                    std::unordered_set<int> result;
                    for (int id : leftResults) {
                        if (rightResults.count(id) > 0) {
                            result.insert(id);
                        }
                    }
                    return result;
                }
                
                case NodeType::OR: {
                    auto leftResults = evaluateNode(node->left);
                    auto rightResults = evaluateNode(node->right);
                    
                    std::unordered_set<int> result = leftResults;
                    for (int id : rightResults) {
                        result.insert(id);
                    }
                    return result;
                }
                
                case NodeType::NOT: {
                    auto childResults = evaluateNode(node->left);
                    
                    std::unordered_set<int> result;
                    for (int id : allBookIds) {
                        if (childResults.count(id) == 0) {
                            result.insert(id);
                        }
                    }
                    return result;
                }
                
                case NodeType::FIELD_QUERY: {
                    return evaluateFieldQuery(node);
                }
                
                case NodeType::KEYWORD_QUERY: {
                    // 關鍵字查詢，類似於 TERM 的處理
                    return searchInTitle(node->term);
                }
            }
            
            return {};
        };
    
    // Evaluate the query
    auto resultIds = evaluateNode(root);
    
    // Convert IDs to Book pointers
    for (int id : resultIds) {
        auto bookIt = SearchUtil::mapFind(bookIdMap, id);
        if (bookIt != bookIdMap.end()) {
            results.push_back(const_cast<Book*>(&books[bookIt->second]));
        }
    }
    
    return results;
}

// Evaluate a field-specific query against all books
std::unordered_set<int> BookManager::evaluateFieldQuery(const std::shared_ptr<QueryNode>& node) const {
    if (!node || node->type != NodeType::FIELD_QUERY) {
        return {};
    }
    
    std::unordered_set<int> result;
    
    // Iterate through all books and check if they match the field query
    for (const auto& book : books) {
        if (bookMatchesFieldQuery(book, node)) {
            result.insert(book.getId());
        }
    }
    
    return result;
}

// Check if a book matches a field-specific query
bool BookManager::bookMatchesFieldQuery(const Book& book, const std::shared_ptr<QueryNode>& node) const {
    if (!node || node->type != NodeType::FIELD_QUERY) {
        return false;
    }
    
    const std::string& field = node->field;
    FieldOperator op = node->fieldOp;
    const std::string& value = node->fieldValue;
    
    std::string fieldValue;
    int numericValue = 0;
    bool isNumeric = false;
    
    std::string lowerField = field;
    for (char& c : lowerField) c = std::tolower(c);
    
    if (lowerField == "title" || lowerField == "標題") {
        fieldValue = book.getTitle();
    } 
    else if (lowerField == "author" || lowerField == "作者") {
        fieldValue = book.getAuthor();
    } 
    else if (lowerField == "year" || lowerField == "年份") {
        numericValue = book.getYear();
        isNumeric = true;
    } 
    else if (lowerField == "isbn") {
        fieldValue = book.getIsbn();
    } 
    else if (lowerField == "publisher" || lowerField == "出版社") {
        fieldValue = book.getPublisher();
    } 
    else if (lowerField == "language" || lowerField == "語言") {
        fieldValue = book.getLanguage();
    } 
    else if (lowerField == "pagecount" || lowerField == "頁數") {
        numericValue = book.getPageCount();
        isNumeric = true;
    } 
    else if (lowerField == "category" || lowerField == "類別" || lowerField == "標籤") {
        const auto& categories = book.getCategories();
        
        for (const auto& category : categories) {
            if (QueryMatcher::matchString(category, op, value, false)) {
                return true;
            }
        }
        
        return false;
    } 
    else if (lowerField == "synopsis" || lowerField == "簡介" || lowerField == "概要") {
        fieldValue = book.getSynopsis();
    } 
    else if (lowerField == "copies" || lowerField == "totalcopies" || lowerField == "總數量") {
        numericValue = book.getTotalCopies();
        isNumeric = true;
    } 
    else if (lowerField == "availablecopies" || lowerField == "可用數量") {
        numericValue = book.getAvailableCopies();
        isNumeric = true;
    } 
    else {
        return false;
    }
    
    return isNumeric ? 
        QueryMatcher::matchNumber(numericValue, op, value) : 
        QueryMatcher::matchString(fieldValue, op, value, true);
}

std::unordered_set<int> BookManager::searchInTitle(const std::string& query) const {
    // 如果查詢字串為空，回傳空結果
    if (query.empty()) {
        return {};
    }
    
    // 對於中文或包含特殊字元的查詢，使用子字串匹配
    // 對於英文詞彙，使用分詞匹配
    auto queryTokens = tokenize(query);
    std::unordered_set<int> resultIds;
    
    // 先嘗試使用分詞進行精確匹配（適用於英文）
    if (!queryTokens.empty()) {
        // 取得第一個 token 的結果
        auto it = SearchUtil::mapFind(titleIndex, queryTokens[0]);
        if (it != titleIndex.end()) {
            resultIds = it->second;
        }
        
        // 對每個額外的 token 進行交集操作
        for (size_t i = 1; i < queryTokens.size(); ++i) {
            std::unordered_set<int> tokenResults;
            it = SearchUtil::mapFind(titleIndex, queryTokens[i]);
            if (it != titleIndex.end()) {
                tokenResults = it->second;
            }
            
            std::unordered_set<int> intersection;
            for (int id : resultIds) {
                if (tokenResults.count(id) > 0) {
                    intersection.insert(id);
                }
            }
            
            resultIds = intersection;
        }
    }
    
    // 如果分詞匹配沒有結果，或查詢包含中文字元，使用子字串匹配
    if (resultIds.empty()) {
        for (const auto& book : books) {
            std::string title = book.getTitle();
            std::string searchQuery = query;
            
            // 轉換為小寫進行不區分大小寫的比較
            for (char& c : title) c = std::tolower(c);
            for (char& c : searchQuery) c = std::tolower(c);
            
            if (SearchUtil::contains(title, searchQuery)) {
                resultIds.insert(book.getId());
            }
        }
    }
    
    return resultIds;
}

void BookManager::updateBookIndex(int bookId, const Book& book) {
    addToIndex(bookId, book.getTitle());
    addToIndex(bookId, book.getAuthor());
    for (const auto& category : book.getCategories()) {
        addToIndex(bookId, category);
    }
    addToTitleIndex(bookId, book.getTitle());
}

void BookManager::rebuildBookIdMap() {
    bookIdMap.clear();
    for (size_t i = 0; i < books.size(); ++i) {
        bookIdMap[books[i].getId()] = i;
    }
} 