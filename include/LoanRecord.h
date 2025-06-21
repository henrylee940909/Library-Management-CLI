#ifndef LOAN_RECORD_H
#define LOAN_RECORD_H

#include <string>
#include <ctime>

class LoanRecord {
private:
    int bookId;
    std::string username;
    time_t borrowDate;
    time_t dueDate;
    time_t returnDate; // 如果尚未歸還則為 0
    int fineAmount;

public:
    LoanRecord();
    LoanRecord(const std::string& username, int bookId, time_t borrowDate, 
               time_t dueDate, int graceDays = 0);
    
    // 取值方法
    int getBookId() const;
    std::string getUsername() const;
    time_t getBorrowDate() const;
    time_t getDueDate() const;
    time_t getReturnDate() const;
    bool isReturned() const;
    int getGraceDays() const;
    
    // 設值方法
    void setBookId(int bookId);
    void setUsername(const std::string& username);
    void setBorrowDate(time_t borrowDate);
    void setDueDate(time_t dueDate);
    void setReturnDate(time_t returnDate);
    void setReturned(bool returned);
    void setGraceDays(int graceDays);
    
    // 操作方法
    void markAsReturned();
    bool isOverdue() const;
    bool isOverdue(time_t currentTime) const;
    int getDaysOverdue() const;
    int getDaysOverdue(time_t currentTime) const;
    double calculateFine(double dailyRate, double incrementalFactor = 1.0) const;
    
    // 顯示功能
    void display() const;
};

#endif // LOAN_RECORD_H 