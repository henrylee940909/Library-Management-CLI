#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <string>
#include <vector>
#include <ctime>
#include "Book.h"
#include "LoanRecord.h"
#include "FinePolicy.h"

class DisplayManager {
public:
    // 書籍顯示
    static void showBookSummary(const Book* book);
    static void showBookDetails(const Book* book);
    static void showSearchResults(const std::vector<Book*>& books);
    static void showAvailableBooks(const std::vector<Book>& books);
    
    // 借閱記錄顯示
    static void showLoanRecord(const LoanRecord* loan, const Book* book, bool showUsername = false);
    static void showUserLoans(const std::vector<LoanRecord*>& loans, const std::string& username);
    static void showOverdueLoans(const std::vector<LoanRecord*>& loans, bool isAdmin = false);
    
    // 罰款政策顯示
    static void showFinePolicy(const FinePolicy& policy);
    static void showFineInfo(const std::string& username, int bookId, double amount);
    
    // 推薦系統顯示
    static void showRecommendations(const std::vector<std::pair<int, double>>& recs, const std::string& title);
    static void showWelcomeMessage(const std::string& username);
    
    // 統計資料顯示
    static void showStatsSummary(int totalBooks, int totalUsers, int activeLoans);
    
    // 輔助格式化
    static std::string formatTime(time_t timestamp);
    static std::string formatCurrency(double amount);
};

#endif // DISPLAYMANAGER_H 