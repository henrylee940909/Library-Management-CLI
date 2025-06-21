#ifndef BOOK_H
#define BOOK_H

#include <string>
#include <vector>

class Book {
private:
    int id;
    std::string title;
    std::string author;
    int year;
    int availableCopies;
    int totalCopies;
    
    // 額外欄位
    std::string isbn;
    std::string publisher;
    std::string language;
    int pageCount;
    std::string synopsis;
    std::vector<std::string> categories;

public:
    Book();
    Book(int id, const std::string& title, const std::string& author, int year, 
         int copies, const std::string& isbn = "", const std::string& publisher = "", 
         const std::string& language = "", int pageCount = 0, 
         const std::string& synopsis = "");
    
    // 取值方法
    int getId() const;
    std::string getTitle() const;
    std::string getAuthor() const;
    int getYear() const;
    int getAvailableCopies() const;
    int getTotalCopies() const;
    std::string getIsbn() const;
    std::string getPublisher() const;
    std::string getLanguage() const;
    int getPageCount() const;
    std::string getSynopsis() const;
    const std::vector<std::string>& getCategories() const;
    
    // 設值方法
    void setId(int id);
    void setTitle(const std::string& title);
    void setAuthor(const std::string& author);
    void setYear(int year);
    void setAvailableCopies(int copies);
    void setTotalCopies(int copies);
    void setIsbn(const std::string& isbn);
    void setPublisher(const std::string& publisher);
    void setLanguage(const std::string& language);
    void setPageCount(int pageCount);
    void setSynopsis(const std::string& synopsis);
    void addCategory(const std::string& category);
    void removeCategory(const std::string& category);
    
    // 圖書操作
    bool borrow();
    bool returnBook();
    
    // 顯示功能
    void display() const;
    void displaySummary() const;
    
    // 搜尋與篩選功能
    bool matchesKeyword(const std::string& keyword) const;
    bool matchesYear(int y, const std::string& op) const; // op 可以是 "=", ">", "<", ">=", "<="
    bool matchesCategory(const std::string& category) const;
};

#endif // BOOK_H 