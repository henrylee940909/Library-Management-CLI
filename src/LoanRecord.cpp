#include "../include/LoanRecord.h"
#include <iostream>
#include <iomanip>
#include <ctime>

LoanRecord::LoanRecord() 
    : bookId(0), borrowDate(0), dueDate(0), returnDate(0) {}

LoanRecord::LoanRecord(const std::string& username, int bookId, time_t borrowDate, 
                       time_t dueDate, int graceDays)
    : username(username), bookId(bookId), borrowDate(borrowDate), 
      dueDate(dueDate), returnDate(0), fineAmount(0) {}

// Getters
int LoanRecord::getBookId() const { return bookId; }
std::string LoanRecord::getUsername() const { return username; }
time_t LoanRecord::getBorrowDate() const { return borrowDate; }
time_t LoanRecord::getDueDate() const { return dueDate; }
time_t LoanRecord::getReturnDate() const { return returnDate; }

// Setters
void LoanRecord::setBookId(int bookId) { this->bookId = bookId; }
void LoanRecord::setUsername(const std::string& username) { this->username = username; }
void LoanRecord::setBorrowDate(time_t borrowDate) { this->borrowDate = borrowDate; }
void LoanRecord::setDueDate(time_t dueDate) { this->dueDate = dueDate; }
void LoanRecord::setReturnDate(time_t returnDate) { this->returnDate = returnDate; }

// Operations
bool LoanRecord::isReturned() const { return returnDate != 0; }

bool LoanRecord::isOverdue() const {
    if (isReturned()) {
        return returnDate > dueDate;
    }
    return time(nullptr) > dueDate;
}

bool LoanRecord::isOverdue(time_t currentTime) const {
    if (isReturned()) {
        return returnDate > dueDate;
    }
    return currentTime > dueDate;
}

int LoanRecord::getDaysOverdue() const {
    return getDaysOverdue(time(nullptr));
}

int LoanRecord::getDaysOverdue(time_t currentTime) const {
    time_t compareTime = isReturned() ? returnDate : currentTime;
    if (compareTime <= dueDate) {
        return 0;
    }
    return static_cast<int>((compareTime - dueDate) / (24 * 60 * 60));
}

void LoanRecord::markAsReturned() {
    returnDate = time(nullptr);
}

// Format time_t to string (helper function)
std::string formatTime(time_t time) {
    struct tm* timeInfo = localtime(&time);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);
    return std::string(buffer);
}

// Display
void LoanRecord::display() const {
    std::cout << "Book ID: " << bookId << "\n"
              << "Username: " << username << "\n"
              << "Borrow Date: " << formatTime(borrowDate) << "\n"
              << "Due Date: " << formatTime(dueDate) << "\n";
    
    if (isReturned()) {
        std::cout << "Return Date: " << formatTime(returnDate) << "\n";
    } else {
        std::cout << "Status: Not returned\n";
    }
    
    if (isOverdue()) {
        std::cout << "Overdue: " << getDaysOverdue() << " days\n";
    }
} 