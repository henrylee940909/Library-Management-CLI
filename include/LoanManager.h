#ifndef LOAN_MANAGER_H
#define LOAN_MANAGER_H

#include <vector>
#include <unordered_map>
#include <string>
#include "LoanRecord.h"
#include "FinePolicy.h"
#include "Book.h"
#include "BookManager.h"

class LoanManager {
private:
    std::vector<LoanRecord> loans;
    std::unordered_map<int, std::vector<LoanRecord*>> bookLoans;    // bookId -> loans
    std::unordered_map<std::string, std::vector<LoanRecord*>> userLoans; // username -> loans
    FinePolicy finePolicy;

public:
    LoanManager();
    LoanManager(const std::string& filename);

    void setFilename(const std::string& filename);

    // 借閱操作
    bool borrowBook(const std::string& username, int bookId, int graceDays = 0);
    bool returnBook(const std::string& username, int bookId);
    bool extendLoan(const std::string& username, int bookId, int days);

    // 取得借閱記錄
    std::vector<LoanRecord*> getLoansForUser(const std::string& username) const;
    std::vector<LoanRecord*> getLoansForBook(int bookId) const;
    std::vector<LoanRecord*> getAllLoans() const;
    std::vector<LoanRecord*> getOverdueLoans() const;
    LoanRecord* findActiveLoan(const std::string& username, int bookId) const;

    // 罰款政策
    void setFinePolicy(const FinePolicy& policy);
    FinePolicy getFinePolicy() const;
    double calculateFine(const LoanRecord& loan) const;
    double calculateUserFines(const std::string& username) const;

    // 檔案操作
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename) const;

    // 統計與視覺化
    std::unordered_map<int, int> getBookBorrowStats() const;
    std::unordered_map<std::string, int> getUserBorrowStats() const;
    std::vector<std::pair<std::string, int>> getMonthlyStats() const;

    // 顯示功能
    void displayUserLoans(const std::string& username) const;
    void displayOverdueLoans() const;
    void displayLoanHistory() const;
};

#endif // LOAN_MANAGER_H 