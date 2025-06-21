#ifndef BOOK_MANAGER_H
#define BOOK_MANAGER_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include "Book.h"
#include "QueryParser.h"

class BookManager {
private:
    std::vector<Book> books;
    std::unordered_map<int, int> bookIdMap; // id -> index in vector
    std::unordered_map<std::string, std::unordered_set<int>> invertedIndex; // term -> set of book ids
    std::unordered_map<std::string, std::unordered_set<int>> titleIndex; // title term -> set of book ids
    int nextId;

    // 索引建構與維護
    void buildInvertedIndex();
    void buildTitleIndex();
    void addToIndex(int bookId, const std::string& term);
    void addToTitleIndex(int bookId, const std::string& term);
    void removeFromIndex(int bookId);
    void removeFromTitleIndex(int bookId);
    void updateBookIndex(int bookId, const Book& book);
    void rebuildBookIdMap();
    
    // 搜尋相關
    std::vector<std::string> tokenize(const std::string& text) const;
    std::unordered_set<int> searchInTitle(const std::string& query) const;
    
    // 查詢評估
    std::unordered_set<int> evaluateFieldQuery(const std::shared_ptr<QueryNode>& node) const;
    bool bookMatchesFieldQuery(const Book& book, const std::shared_ptr<QueryNode>& node) const;

public:
    BookManager();
    
    // 圖書操作
    bool addBook(Book& book);
    bool updateBook(const Book& book);
    bool deleteBook(int bookId);
    Book* getBook(int bookId);
    const Book* getBook(int bookId) const;
    bool borrowBook(int bookId);
    bool returnBook(int bookId);
    
    // 搜尋功能
    std::vector<Book*> searchBooks(const std::string& query) const;
    std::vector<Book*> filterByYear(int year, const std::string& op) const;
    std::vector<Book*> filterByCategory(const std::string& category) const;
    std::vector<Book*> advancedSearch(const std::string& query) const;
    
    // 資料取得
    const std::vector<Book>& getAllBooks() const;
    int getTotalBooks() const;
    std::unordered_map<std::string, int> getCategoryStats() const;
    
    // 檔案操作
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename) const;
    
    // 顯示功能
    void displayAllBooks() const;
    void displayBooksByCategory() const;
    void displayBooksByYear() const;
};

#endif // BOOK_MANAGER_H 