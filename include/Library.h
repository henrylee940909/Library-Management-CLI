#ifndef LIBRARY_H
#define LIBRARY_H

#include <string>
#include <ctime>
#include <vector>
#include <unordered_map>
#include "BookManager.h"
#include "UserManager.h"
#include "LoanManager.h"
#include "RecommendationEngine.h"
#include "FinePolicy.h"

class Library {
private:
    BookManager bookManager;
    UserManager userManager;
    LoanManager loanManager;
    RecommendationEngine recommendationEngine;
    
    // 檔案路徑
    std::string bookFile;
    std::string userFile;
    std::string loanFile;
    
    // 輔助結構
    struct BookInfo {
        std::string title, author, isbn, publisher, language, synopsis;
        int year, copies, pageCount;
    };
    
    // 核心初始化和運行邏輯
    void createDataDirectory();
    void loadAllData();
    bool performLogin();
    void runMainLoop();
    
    // 選單系統
    bool adminMenu();
    bool staffMenu();
    bool readerMenu();
    int getMenuChoice();
    void showInvalidChoice();
    void showUserInfo();
    bool handleLogoutChoice(int choice, int logoutOption, int exitOption);
    
    // 權限檢查
    bool requiresPermission(Role requiredRole);
    
    // 使用者操作
    void addUser();
    void changePassword();
    std::string getUserInput(const std::string& prompt);
    Role selectRole();
    
    // 圖書操作
    void addBook();
    void deleteBook();
    void editBook();
    void searchBooks();
    void viewBookDetails();
    BookInfo getBookInfoFromUser();
    void addBookCategories(Book& book);
    std::vector<Book*> performSearch(int searchType);
    std::vector<Book*> searchByYear();
    void displaySearchResults(const std::vector<Book*>& results);
    void displayBookSummary(const Book* book);
    void displayBookSummaryDetailed(const Book* book);
    void offerBookDetails(const std::vector<Book*>& results);
    
    // 書籍詳細資訊顯示
    void displayBookDetailsHeader(const Book* book);
    void displayBookBasicInfo(const Book* book);
    void displayBookInventoryStatus(const Book* book);
    void displayBookCategories(const Book* book);
    void displayBookSynopsis(const Book* book);
    void displayBookBorrowStats(const Book* book);
    
    // 熱門度計算輔助結構和方法
    struct PopularityInfo {
        int borrowCount;
        double percentile;
        double relativeToMean;
        double relativeToMedian;
        std::string level;
        std::string description;
    };
    PopularityInfo calculateRelativePopularity(int bookId, const std::unordered_map<int, int>& bookStats) const;
    void displayPopularityLevel(const PopularityInfo& info) const;
    
    // 書籍列表功能
    void viewBookList();
    void displayBookPage(const std::vector<Book>& books, int page, int booksPerPage,
                        int currentSortField, int currentSortOrder);
    bool handleBookListNavigation(int choice, int& currentPage, int totalPages, 
                                 const std::vector<Book>& books, 
                                 const std::vector<std::string>& navOptions,
                                 int& currentSortField, int& currentSortOrder);
    void jumpToPage(int& currentPage, int totalPages);
    void viewBookDetailsFromList();
    
    // 排序輔助函數
    void applySorting(std::vector<Book>& books, int sortField, int sortOrder);
    std::string getSortArrow(int currentSortField, int currentSortOrder, int fieldId);
    void showSortMenu(int& currentSortField, int& currentSortOrder);
    void setSortField(int& currentSortField, int& currentSortOrder, int newField);
    
    // 欄位寬度計算
    struct ColumnWidths {
        size_t idWidth;
        size_t titleWidth;
        size_t authorWidth;
        size_t yearWidth;
        size_t pagesWidth;
        size_t statusWidth;
    };
    ColumnWidths calculateColumnWidths(const std::vector<Book>& books, int startIndex, int endIndex);
    
    // 中文字符顯示寬度處理
    size_t getDisplayWidth(const std::string& str);
    std::string truncateToWidth(const std::string& str, size_t maxWidth);
    
    // 借閱操作
    void borrowBook();
    void returnBook();
    void displayUserLoans();
    void displayOverdueLoans();
    
    // 借閱輔助方法
    std::vector<Book> getAvailableBooks();
    void displayAvailableBooks(const std::vector<Book>& books);
    int getBookIdChoice(const std::string& prompt);
    std::string getBorrowerUsername();
    std::string getTargetUserForReturn();
    std::vector<LoanRecord*> getActiveLoansForUser(const std::string& username);
    void showNoActiveLoansMessage(const std::string& username);
    void displayActiveLoans(const std::vector<LoanRecord*>& loans, const std::string& username);
    void displayLoanRecord(const LoanRecord* loan, const Book* book, bool showUsername);
    void showFineIfAny(const std::string& username, int bookId);
    
    // 編輯圖書輔助方法
    void runEditMenu(Book& book);
    std::vector<std::string> createEditOptions(const Book& book);
    void editBookField(Book& book, int field);
    void updateBookCopies(Book& book);
    void manageBookCategories(Book& book);
    void displayCurrentCategories(const Book& book);
    void removeCategoryFromBook(Book& book);
    bool saveBookChanges(Book& book);
    
    // 用戶借閱記錄輔助方法
    void sortLoansByStatus(std::vector<LoanRecord*>& loans);
    void displayCurrentAndReturnedLoans(const std::vector<LoanRecord*>& loans);
    bool displayLoanSection(const std::vector<LoanRecord*>& loans, bool showReturned, const std::string& title);
    void displayDetailedLoanRecord(const LoanRecord* loan);
    void showFineForReturnedBook(const LoanRecord* loan);
    void showLoanStatus(const LoanRecord* loan);
    
    // 罰款政策
    void setFinePolicy();
    void displayFinePolicy();
    void showCurrentPolicy(const FinePolicy& policy);
    void runPolicyMenu(const FinePolicy& currentPolicy);
    std::vector<std::string> createPolicyOptions(const FinePolicy& policy);
    bool handlePolicyChoice(int choice, const FinePolicy& currentPolicy);
    void updateGraceDays(FinePolicy& policy);
    void updateFixedRate(FinePolicy& policy);
    void updateIncFactor(FinePolicy& policy);
    void useDefaultPolicy();
    bool savePolicyAndExit();
    
    // 推薦系統
    void showRecommendations();
    void showInteractiveRecommendations(const std::string& username);
    void displayRecommendationList(const std::vector<std::pair<int, double>>& recommendations, 
                                  const std::string& username);
    void handleRecommendationInteraction(const std::vector<std::pair<int, double>>& recommendations);
    void showPopularBooksRecommendation();
    void showWelcomeRecommendations(const std::string& username);
    
    // 推薦系統輔助方法
    void showPopularBooksForNewUser(const std::string& username);
    void showPersonalizedRecommendations(const std::string& username, 
                                       const std::vector<LoanRecord*>& userLoans);
    double calculateContentScore(int bookId, const std::vector<LoanRecord*>& userLoans);
    void displayRecommendationItem(const Book* book, double hybridScore, 
                                 double cfScore, double contentScore, int rank);
    
    // 逾期圖書輔助方法
    void displayReaderOverdueLoans(const std::vector<LoanRecord*>& overdueLoans);
    void displayAllOverdueLoans(const std::vector<LoanRecord*>& overdueLoans);
    
    // 統計功能
    void showStatistics();
    void showBorrowStats();
    void showCategoryStats();
    void showMonthlyStats();
    
    // 統計輔助方法
    void showQuickStatsSummary();
    void showBorrowingSummary(const std::unordered_map<int, int>& bookStats, 
                             const std::unordered_map<std::string, int>& userStats);
    void showCategoryEfficiency(const std::unordered_map<std::string, int>& categoryCount,
                               const std::unordered_map<std::string, int>& categoryBorrows);
    void showMonthlyStatsSummary(const std::vector<std::pair<std::string, int>>& monthlyData);
    void showSystemOverview();
    void showDetailedSystemStatus();
    void showRecentActivitySummary();
    
    // 輔助函數
    std::string formatTime(time_t time);

    // 進階搜尋輔助方法
    void showAdvancedSearchHelp();
    void showSearchTutorial();

public:
    Library();
    Library(const std::string& bookFile, const std::string& userFile, 
            const std::string& loanFile);
    
    // 初始化
    bool initialize();
    bool setupAdmin();
    
    // 主要進入點
    void run();
    
    // 儲存所有資料
    bool saveAllData() const;
};

#endif // LIBRARY_H 