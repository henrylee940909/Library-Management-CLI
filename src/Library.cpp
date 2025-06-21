#include "../include/Library.h"
#include "../include/PasswordUtil.h"
#include "../include/VisualizationUtil.h"
#include "../include/SortUtil.h"
#include "../include/ConsoleUtil.h"
#include "../include/SearchUtil.h"
#include <iostream>
#include <limits>
#include <fstream>
#include <sys/stat.h>
#include <iomanip>
#include <thread>
#include <chrono>
#include <climits>   // for INT_MAX / INT_MIN

Library::Library()
    : bookFile("data/books.json"),
      userFile("data/users.json"),
      loanFile("data/loans.json") {}

Library::Library(const std::string& bookFile, const std::string& userFile, 
                 const std::string& loanFile)
    : bookFile(bookFile),
      userFile(userFile),
      loanFile(loanFile) {}

bool Library::initialize() {
    createDataDirectory();
    
    bool usersLoaded = userManager.loadFromFile(userFile);
    
    if (!usersLoaded && userManager.isFirstRun()) {
        std::cout << "首次設定 - 創建管理員帳號:" << std::endl;
        
        if (!setupAdmin()) {
            std::cerr << "創建管理員帳號失敗。" << std::endl;
            return false;
        }
        
        std::cout << "管理員帳號創建成功！" << std::endl;
        std::cout << "請重新啟動程式以登入。" << std::endl;
        return false;
    }
    
    // 無論用戶資料是否載入成功，都要載入圖書和借閱資料
    loadAllData();
    recommendationEngine.initialize(bookManager, loanManager);
    
    return true;
}

void Library::createDataDirectory() {
    struct stat info;
    if (stat("data", &info) != 0) {
#ifdef _WIN32
        system("mkdir data");
#else
        system("mkdir -p data");
#endif
    }
}

void Library::loadAllData() {
    if (!bookManager.loadFromFile(bookFile)) {
        std::cout << "未找到現有圖書資料。以空資料庫啟動。" << std::endl;
    }
    
    if (!loanManager.loadFromFile(loanFile)) {
        std::cout << "未找到現有借閱資料。" << std::endl;
    }
}

bool Library::setupAdmin() {
    std::string username, password, confirmPassword;
    
    std::cout << "管理員使用者名稱: ";
    std::getline(std::cin, username);
    
    password = PasswordUtil::getPasswordInput("管理員密碼:");
    confirmPassword = PasswordUtil::getPasswordInput("確認密碼:");
    
    if (password != confirmPassword) {
        std::cerr << "密碼不匹配！" << std::endl;
        return false;
    }
    
    if (username.empty() || password.empty()) {
        std::cerr << "使用者名稱和密碼不能為空！" << std::endl;
        return false;
    }
    
    bool success = userManager.setupAdminAccount(username, password);
    
    if (success) {
        success = userManager.saveToFile(userFile);
    }
    
    return success;
}

bool Library::saveAllData() const {
    bool success = true;
    
    if (!bookManager.saveToFile(bookFile)) {
        std::cerr << "儲存圖書資料失敗！" << std::endl;
        success = false;
    }
    
    if (!userManager.saveToFile(userFile)) {
        std::cerr << "儲存使用者資料失敗！" << std::endl;
        success = false;
    }
    
    if (!loanManager.saveToFile(loanFile)) {
        std::cerr << "儲存借閱資料失敗！" << std::endl;
        success = false;
    }
    
    return success;
}

void Library::run() {
    while (true) {
        if (!performLogin()) {
            return;
        }
        
        runMainLoop();
        saveAllData();
    }
}

bool Library::performLogin() {
    bool loggedIn = false;
    int attempts = 0;
    const int maxAttempts = 3;
    
    while (!loggedIn && attempts < maxAttempts) {
        ConsoleUtil::printTitle("圖書館管理系統");
        
        std::string username, password;
        
        ConsoleUtil::printColored("使用者名稱: ", ConsoleUtil::Color::BRIGHT_CYAN);
        std::getline(std::cin, username);
        
        password = PasswordUtil::getPasswordInput("密碼:");
        
        if (userManager.login(username, password)) {
            loggedIn = true;
            ConsoleUtil::printSuccess("登入成功，您的角色: " + userManager.getCurrentUser()->getRoleName());
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
        } else {
            ConsoleUtil::printError("使用者名稱或密碼無效，請重試");
            attempts++;
            
            if (attempts >= maxAttempts) {
                ConsoleUtil::printError("嘗試次數過多，正在退出系統...");
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
    
    return loggedIn;
}

void Library::runMainLoop() {
    Role userRole = userManager.getCurrentUser()->getRole();
    bool stayLoggedIn = true;
    
    while (stayLoggedIn && userManager.isLoggedIn()) {
        switch (userRole) {
            case Role::Admin:
                stayLoggedIn = adminMenu();
                break;
            case Role::Staff:
                stayLoggedIn = staffMenu();
                break;
            case Role::Reader:
                stayLoggedIn = readerMenu();
                break;
        }
    }
}

void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

bool Library::requiresPermission(Role requiredRole) {
    Role currentRole = userManager.getCurrentUser()->getRole();
    return static_cast<int>(currentRole) >= static_cast<int>(requiredRole);
}

void Library::showUserInfo() {
    std::string username = userManager.getCurrentUser()->getUsername();
    std::string role = userManager.getCurrentUser()->getRoleName();
    ConsoleUtil::printInfo("目前登入：" + username + " (" + role + ")");
    std::cout << "\n";
}

bool Library::handleLogoutChoice(int choice, int logoutOption, int exitOption) {
    if (choice == logoutOption) {
        userManager.logout();
        ConsoleUtil::printSuccess("已登出");
        return false; // 返回登入畫面
    } else if (choice == exitOption) {
        userManager.logout();
        saveAllData();
        ConsoleUtil::printSuccess("資料已儲存，系統即將退出");
        ConsoleUtil::pauseAndWait();
        exit(0);
    }
    return true; // 繼續目前選單
}

bool Library::adminMenu() {
    while (true) {
        std::vector<std::string> options = {
            "新增使用者", "設置罰款政策", "新增圖書", "刪除圖書", "編輯圖書",
            "搜尋圖書", "檢視書籍", "書籍列表", "借閱圖書", "歸還圖書", "修改密碼", 
            "檢視統計資料", "檢視逾期圖書", "登出", "退出系統"
        };
        
        ConsoleUtil::printTitleWithSubtitle("圖書館管理系統", "管理員主選單");
        showUserInfo();
        ConsoleUtil::printMenuOptions(options);
        
        int choice = getMenuChoice();
        
        switch (choice) {
            case 1: addUser(); break;
            case 2: setFinePolicy(); break;
            case 3: addBook(); break;
            case 4: deleteBook(); break;
            case 5: editBook(); break;
            case 6: searchBooks(); break;
            case 7: viewBookDetails(); break;
            case 8: viewBookList(); break;
            case 9: borrowBook(); break;
            case 10: returnBook(); break;
            case 11: changePassword(); break;
            case 12: showStatistics(); break;
            case 13: displayOverdueLoans(); break;
            case 14: case 15: 
                return !handleLogoutChoice(choice, 14, 15);
            default:
                showInvalidChoice();
        }
    }
}

bool Library::staffMenu() {
    while (true) {
        std::vector<std::string> options = {
            "新增圖書", "刪除圖書", "編輯圖書", "搜尋圖書", "檢視書籍", "書籍列表", 
            "借閱圖書", "歸還圖書", "修改密碼", "檢視逾期圖書", "登出", "退出系統"
        };
        
        ConsoleUtil::printTitleWithSubtitle("圖書館管理系統", "館員主選單");
        showUserInfo();
        ConsoleUtil::printMenuOptions(options);
        
        int choice = getMenuChoice();
        
        switch (choice) {
            case 1: addBook(); break;
            case 2: deleteBook(); break;
            case 3: editBook(); break;
            case 4: searchBooks(); break;
            case 5: viewBookDetails(); break;
            case 6: viewBookList(); break;
            case 7: borrowBook(); break;
            case 8: returnBook(); break;
            case 9: changePassword(); break;
            case 10: displayOverdueLoans(); break;
            case 11: case 12:
                return !handleLogoutChoice(choice, 11, 12);
            default:
                showInvalidChoice();
        }
    }
}

bool Library::readerMenu() {
    // 在讀者登入後先顯示歡迎推薦
    std::string currentUsername = userManager.getCurrentUser()->getUsername();
    showWelcomeRecommendations(currentUsername);
    
    while (true) {
        std::vector<std::string> options = {
            "搜尋圖書", "檢視書籍", "書籍列表", "借閱圖書", "歸還圖書", "檢視我的借閱",
            "修改密碼", "檢視推薦", "登出", "退出系統"
        };
        
        ConsoleUtil::printTitleWithSubtitle("圖書館管理系統", "讀者主選單");
        showUserInfo();
        ConsoleUtil::printMenuOptions(options);
        
        int choice = getMenuChoice();
        
        switch (choice) {
            case 1: searchBooks(); break;
            case 2: viewBookDetails(); break;
            case 3: viewBookList(); break;
            case 4: borrowBook(); break;
            case 5: returnBook(); break;
            case 6: displayUserLoans(); break;
            case 7: changePassword(); break;
            case 8: showRecommendations(); break;
            case 9: case 10:
                return !handleLogoutChoice(choice, 9, 10);
            default:
                showInvalidChoice();
        }
    }
}

int Library::getMenuChoice() {
    int choice;
    std::cin >> choice;
    clearInputBuffer();
    return choice;
}

void Library::showInvalidChoice() {
    ConsoleUtil::printError("無效的選擇，請重試");
    ConsoleUtil::pauseAndWait();
}

void Library::addUser() {
    if (!userManager.hasPermission(Role::Admin)) {
        ConsoleUtil::printError("權限拒絕：只有管理員可以新增使用者");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    ConsoleUtil::printTitle("新增使用者");
    
    std::string username = getUserInput("使用者名稱");
    std::string password = PasswordUtil::getPasswordInput("密碼:");
    std::string confirmPassword = PasswordUtil::getPasswordInput("確認密碼:");
    
    if (password != confirmPassword) {
        ConsoleUtil::printError("密碼不匹配");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    Role role = selectRole();
    
    if (userManager.addUser(username, password, role)) {
        ConsoleUtil::printSuccess("使用者 " + username + " 成功新增");
        userManager.saveToFile(userFile);
    } else {
        ConsoleUtil::printError("新增使用者失敗，使用者名稱可能已存在");
    }
    
    ConsoleUtil::pauseAndWait();
}

std::string Library::getUserInput(const std::string& prompt) {
    std::string input;
    std::cout << prompt << ": ";
    std::getline(std::cin, input);
    return input;
}

Role Library::selectRole() {
    ConsoleUtil::printInfo("選擇角色 (1=館員, 2=讀者): ");
    int choice = getMenuChoice();
    return (choice == 1) ? Role::Staff : Role::Reader;
}

void Library::changePassword() {
    ConsoleUtil::printTitle("修改密碼");
    
    std::string oldPassword = PasswordUtil::getPasswordInput("當前密碼:");
    std::string newPassword = PasswordUtil::getPasswordInput("新密碼:");
    std::string confirmPassword = PasswordUtil::getPasswordInput("確認新密碼:");
    
    if (newPassword != confirmPassword) {
        ConsoleUtil::printError("密碼不匹配");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    std::string username = userManager.getCurrentUser()->getUsername();
    if (userManager.changePassword(username, oldPassword, newPassword)) {
        ConsoleUtil::printSuccess("密碼修改成功");
        userManager.saveToFile(userFile);
    } else {
        ConsoleUtil::printError("修改密碼失敗，請檢查當前密碼");
    }
    
    ConsoleUtil::pauseAndWait();
}

void Library::addBook() {
    if (!userManager.hasPermission(Role::Staff)) {
        ConsoleUtil::printError("權限拒絕：只有館員和管理員可以新增圖書");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    ConsoleUtil::printTitle("新增圖書");
    
    BookInfo info = getBookInfoFromUser();
    
    Book book(0, info.title, info.author, info.year, info.copies, 
              info.isbn, info.publisher, info.language, info.pageCount, info.synopsis);
    
    addBookCategories(book);
    
    if (bookManager.addBook(book)) {
        ConsoleUtil::printSuccess("圖書成功新增，ID: " + std::to_string(book.getId()));
        bookManager.saveToFile(bookFile);
    } else {
        ConsoleUtil::printError("新增圖書失敗");
    }
    
    ConsoleUtil::pauseAndWait();
}

Library::BookInfo Library::getBookInfoFromUser() {
    BookInfo info;
    
    info.title = getUserInput("書名");
    info.author = getUserInput("作者");
    
    std::cout << "年份: ";
    std::cin >> info.year;
    clearInputBuffer();
    
    info.isbn = getUserInput("ISBN");
    info.publisher = getUserInput("出版社");
    info.language = getUserInput("語言");
    
    std::cout << "頁數: ";
    std::cin >> info.pageCount;
    clearInputBuffer();
    
    std::cout << "複本數量: ";
    std::cin >> info.copies;
    clearInputBuffer();
    
    info.synopsis = getUserInput("摘要");
    
    return info;
}

void Library::addBookCategories(Book& book) {
    ConsoleUtil::printInfo("新增分類 (輸入 'done' 完成):");
    
    std::string category;
    while (true) {
        category = getUserInput("分類");
        if (category == "done") break;
        book.addCategory(category);
    }
}

void Library::deleteBook() {
    if (!userManager.hasPermission(Role::Staff)) {
        ConsoleUtil::printError("權限拒絕：只有館員和管理員可以刪除圖書");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    ConsoleUtil::printTitle("刪除圖書");
    
    int bookId = getBookIdChoice("請輸入要刪除的圖書 ID");
    
    if (bookId <= 0) {
        ConsoleUtil::printWarning("取消刪除");
        return;
    }
    
    const Book* book = bookManager.getBook(bookId);
    
    if (!book) {
        ConsoleUtil::printError("找不到 ID 為 " + std::to_string(bookId) + " 的書籍");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    // 檢查是否有正在借閱的複本
    int borrowedCopies = book->getTotalCopies() - book->getAvailableCopies();
    if (borrowedCopies > 0) {
        ConsoleUtil::printError("無法刪除：該書籍目前有 " + std::to_string(borrowedCopies) + " 本正在借閱中");
        ConsoleUtil::printInfo("請等待所有複本歸還後再進行刪除操作");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    // 顯示要刪除的書籍資訊
    ConsoleUtil::printSubtitle("即將刪除的圖書資訊");
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    std::cout << ConsoleUtil::colorText("📖 書名: ", ConsoleUtil::Color::BRIGHT_CYAN) 
              << ConsoleUtil::colorText(book->getTitle(), ConsoleUtil::Color::BRIGHT_WHITE) << std::endl;
    
    std::cout << ConsoleUtil::colorText("✍️  作者: ", ConsoleUtil::Color::BRIGHT_CYAN) 
              << book->getAuthor() << std::endl;
    
    std::cout << ConsoleUtil::colorText("📅 出版年份: ", ConsoleUtil::Color::BRIGHT_CYAN) 
              << book->getYear() << std::endl;
    
    std::cout << ConsoleUtil::colorText("📚 ISBN: ", ConsoleUtil::Color::BRIGHT_CYAN) 
              << book->getIsbn() << std::endl;
    
    std::cout << ConsoleUtil::colorText("📦 總複本數: ", ConsoleUtil::Color::BRIGHT_CYAN) 
              << book->getTotalCopies() << " 本" << std::endl;
    
    // 顯示分類
    const auto& categories = book->getCategories();
    std::cout << ConsoleUtil::colorText("🏷️  分類: ", ConsoleUtil::Color::BRIGHT_CYAN);
    if (categories.empty()) {
        std::cout << "無分類標籤" << std::endl;
    } else {
        for (size_t i = 0; i < categories.size(); ++i) {
            std::cout << ConsoleUtil::colorText("[" + categories[i] + "]", ConsoleUtil::Color::BRIGHT_MAGENTA);
            if (i < categories.size() - 1) {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    }
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    // 檢查是否有歷史借閱記錄
    auto bookLoans = loanManager.getLoansForBook(bookId);
    if (!bookLoans.empty()) {
        std::cout << std::endl;
        ConsoleUtil::printWarning("注意：此書籍有 " + std::to_string(bookLoans.size()) + " 筆歷史借閱記錄");
        ConsoleUtil::printInfo("刪除書籍不會影響歷史借閱記錄，但該書籍將無法再被借閱");
    }
    
    std::cout << std::endl;
    
    // 二次確認
    ConsoleUtil::printWarning("⚠️  警告：刪除操作無法復原！");
    ConsoleUtil::printInfo("確認要刪除這本書嗎？(輸入 'DELETE' 確認，其他任意鍵取消): ");
    
    std::string confirmation;
    std::getline(std::cin, confirmation);
    
    if (confirmation != "DELETE") {
        ConsoleUtil::printInfo("已取消刪除操作");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    // 最後確認
    ConsoleUtil::printWarning("最後確認：您真的要刪除「" + book->getTitle() + "」嗎？(y/N): ");
    char finalConfirmation;
    std::cin >> finalConfirmation;
    clearInputBuffer();
    
    if (finalConfirmation != 'y' && finalConfirmation != 'Y') {
        ConsoleUtil::printInfo("已取消刪除操作");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    // 執行刪除
    std::string bookTitle = book->getTitle(); // 保存書名用於顯示
    
    if (bookManager.deleteBook(bookId)) {
        ConsoleUtil::printSuccess("圖書「" + bookTitle + "」已成功刪除");
        
        // 保存變更到文件
        if (bookManager.saveToFile(bookFile)) {
            ConsoleUtil::printSuccess("資料已成功保存");
        } else {
            ConsoleUtil::printError("保存資料時發生錯誤，但圖書已刪除");
        }
        
        // 重新初始化推薦引擎（因為圖書資料發生變化）
        recommendationEngine.initialize(bookManager, loanManager);
        
    } else {
        ConsoleUtil::printError("刪除圖書失敗，請稍後再試");
    }
    
    ConsoleUtil::pauseAndWait();
}

void Library::viewBookDetails() {
    ConsoleUtil::printTitle("檢視書籍詳細資訊");
    
    int bookId = getBookIdChoice("請輸入要查看的書籍 ID");
    
    if (bookId <= 0) {
        ConsoleUtil::printWarning("取消檢視");
        return;
    }
    
    const Book* book = bookManager.getBook(bookId);
    
    if (!book) {
        ConsoleUtil::printError("找不到 ID 為 " + std::to_string(bookId) + " 的書籍");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    displayBookDetailsHeader(book);
    displayBookBasicInfo(book);
    displayBookInventoryStatus(book);
    displayBookCategories(book);
    displayBookSynopsis(book);
    displayBookBorrowStats(book);
    
    ConsoleUtil::pauseAndWait();
}

void Library::displayBookDetailsHeader(const Book* book) {
    ConsoleUtil::printTitleWithSubtitle("書籍詳細資訊", "ID: " + std::to_string(book->getId()));
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

void Library::displayBookBasicInfo(const Book* book) {
    std::cout << ConsoleUtil::colorText("📖 書名: ", ConsoleUtil::Color::BRIGHT_CYAN) 
              << ConsoleUtil::colorText(book->getTitle(), ConsoleUtil::Color::BRIGHT_WHITE) << std::endl;
    
    std::cout << ConsoleUtil::colorText("✍️  作者: ", ConsoleUtil::Color::BRIGHT_CYAN) 
              << book->getAuthor() << std::endl;
    
    std::cout << ConsoleUtil::colorText("📅 出版年份: ", ConsoleUtil::Color::BRIGHT_CYAN) 
              << book->getYear() << std::endl;
    
    std::cout << ConsoleUtil::colorText("🏢 出版社: ", ConsoleUtil::Color::BRIGHT_CYAN) 
              << book->getPublisher() << std::endl;
    
    std::cout << ConsoleUtil::colorText("📄 頁數: ", ConsoleUtil::Color::BRIGHT_CYAN) 
              << book->getPageCount() << " 頁" << std::endl;
    
    std::cout << ConsoleUtil::colorText("🌐 語言: ", ConsoleUtil::Color::BRIGHT_CYAN) 
              << book->getLanguage() << std::endl;
    
    std::cout << ConsoleUtil::colorText("📚 ISBN: ", ConsoleUtil::Color::BRIGHT_CYAN) 
              << book->getIsbn() << std::endl;
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

void Library::displayBookInventoryStatus(const Book* book) {
    std::cout << ConsoleUtil::colorText("📊 館藏狀況", ConsoleUtil::Color::BRIGHT_YELLOW) << std::endl;
    std::cout << "   總複本數量: " << book->getTotalCopies() << " 本" << std::endl;
    
    if (book->getAvailableCopies() > 0) {
        std::cout << "   可借閱數量: " 
                  << ConsoleUtil::colorText(std::to_string(book->getAvailableCopies()) + " 本", 
                                          ConsoleUtil::Color::BRIGHT_GREEN) << std::endl;
        std::cout << "   狀態: " << ConsoleUtil::colorText("可借閱", ConsoleUtil::Color::BRIGHT_GREEN) << std::endl;
    } else {
        std::cout << "   可借閱數量: " 
                  << ConsoleUtil::colorText("0 本", ConsoleUtil::Color::BRIGHT_RED) << std::endl;
        std::cout << "   狀態: " << ConsoleUtil::colorText("已借完", ConsoleUtil::Color::BRIGHT_RED) << std::endl;
    }
    
    int borrowedCopies = book->getTotalCopies() - book->getAvailableCopies();
    std::cout << "   已借出數量: " << borrowedCopies << " 本" << std::endl;
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

void Library::displayBookCategories(const Book* book) {
    std::cout << ConsoleUtil::colorText("🏷️  分類標籤", ConsoleUtil::Color::BRIGHT_YELLOW) << std::endl;
    const auto& categories = book->getCategories();
    if (categories.empty()) {
        std::cout << "   無分類標籤" << std::endl;
    } else {
        std::cout << "   ";
        for (size_t i = 0; i < categories.size(); ++i) {
            std::cout << ConsoleUtil::colorText("[" + categories[i] + "]", ConsoleUtil::Color::BRIGHT_MAGENTA);
            if (i < categories.size() - 1) {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    }
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

void Library::displayBookSynopsis(const Book* book) {
    std::cout << ConsoleUtil::colorText("📝 內容簡介", ConsoleUtil::Color::BRIGHT_YELLOW) << std::endl;
    if (book->getSynopsis().empty()) {
        std::cout << "   暫無簡介" << std::endl;
    } else {
        std::cout << "   " << book->getSynopsis() << std::endl;
    }
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

void Library::displayBookBorrowStats(const Book* book) {
    auto bookStats = loanManager.getBookBorrowStats();
    auto it = SearchUtil::mapFind(bookStats, book->getId());
    if (it != bookStats.end() && it->second > 0) {
        std::cout << ConsoleUtil::colorText("📈 借閱統計", ConsoleUtil::Color::BRIGHT_YELLOW) << std::endl;
        std::cout << "   歷史借閱次數: " << it->second << " 次" << std::endl;
        
        // 計算相對熱門度
        auto popularityInfo = calculateRelativePopularity(book->getId(), bookStats);
        std::cout << "   熱門程度: ";
        displayPopularityLevel(popularityInfo);
        
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    }
    
    // 提供相關操作選項
    if (userManager.getCurrentUser()->getRole() == Role::Reader) {
        if (book->getAvailableCopies() > 0) {
            std::cout << std::endl;
            ConsoleUtil::printInfo("此書可借閱，是否要立即借閱？(y/n): ");
            char choice;
            std::cin >> choice;
            clearInputBuffer();
            
            if (choice == 'y' || choice == 'Y') {
                std::string username = userManager.getCurrentUser()->getUsername();
                if (loanManager.borrowBook(username, book->getId())) {
                    ConsoleUtil::printSuccess("圖書借閱成功！");
                } else {
                    ConsoleUtil::printError("圖書借閱失敗");
                }
            }
        }
    }
}

void Library::searchBooks() {
    ConsoleUtil::printTitle("搜尋圖書");
    
    std::vector<std::string> searchOptions = {
        "簡單搜尋", "多條件智慧搜尋 (AND/OR/NOT)", "依年份篩選", "依分類篩選", "互動式搜尋教學"
    };
    
    ConsoleUtil::printSubtitle("搜尋選項");
    ConsoleUtil::printMenuOptions(searchOptions);
    int choice = getMenuChoice();
    
    std::vector<Book*> results = performSearch(choice);
    displaySearchResults(results);
}

std::vector<Book*> Library::performSearch(int searchType) {
    switch (searchType) {
        case 1: {
            std::string query = getUserInput("請輸入搜尋關鍵字");
            return bookManager.searchBooks(query);
        }
        case 2: {
            showAdvancedSearchHelp();
            std::string query = getUserInput("請輸入搜尋條件（支援 AND/OR/NOT、書名/作者/年份/標籤）");
            
            if (query.empty()) {
                ConsoleUtil::printWarning("搜尋條件不能為空");
                return {};
            }
            
            ConsoleUtil::printInfo("搜尋中...");
            auto results = bookManager.advancedSearch(query);
            
            // If no results and query seems complex, suggest checking syntax
            if (results.empty() && (SearchUtil::contains(query, "AND") ||
                SearchUtil::contains(query, "OR") ||
                SearchUtil::contains(query, "NOT")  ||
                SearchUtil::contains(query, "=") ||
                SearchUtil::contains(query, "~") )) {
                ConsoleUtil::printWarning("複雜查詢未找到結果。請檢查語法是否正確。");
                ConsoleUtil::printInfo("提示: 使用選項 5 查看搜尋教學");
            }
            
            return results;
        }
        case 3: {
            return searchByYear();
        }
        case 4: {
            std::string category = getUserInput("請輸入分類");
            return bookManager.filterByCategory(category);
        }
        case 5: {
            showSearchTutorial();
            return {};
        }
        default:
            ConsoleUtil::printError("無效的選擇");
            return {};
    }
}

std::vector<Book*> Library::searchByYear() {
    ConsoleUtil::printInfo("請輸入年份: ");
    int year;
    std::cin >> year;
    clearInputBuffer();

    ConsoleUtil::printInfo("請輸入運算符 (=, >, <, >=, <=): ");
    std::string op;
    std::getline(std::cin, op);

    std::vector<Book*> results = bookManager.filterByYear(year, op);

    SortUtil::sort(results, [](Book* a, Book* b) {
        return a->getTitle() < b->getTitle();
        });

    return results;
}

void Library::displaySearchResults(const std::vector<Book*>& results) {
    if (results.empty()) {
        ConsoleUtil::printWarning("未找到符合條件的圖書");
        std::cout << std::endl;
        ConsoleUtil::printInfo("建議:");
        std::cout << "  • 檢查搜尋條件是否正確" << std::endl;
        std::cout << "  • 嘗試使用更簡單的關鍵字" << std::endl;
        std::cout << "  • 使用 OR 運算符擴大搜尋範圍" << std::endl;
        std::cout << "  • 檢查欄位名稱拼寫是否正確" << std::endl;
        std::cout << std::endl;
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    ConsoleUtil::printSuccess("找到了 " + std::to_string(results.size()) + " 本書：");
    std::cout << std::endl;
    
    // 顯示搜尋結果列表
    for (const auto* book : results) {
        displayBookSummaryDetailed(book);
    }
    
    std::cout << std::endl;
    // 提供查看詳情選項
    offerBookDetails(results);
}

void Library::displayBookSummaryDetailed(const Book* book) {
    std::cout << ConsoleUtil::colorText("[" + std::to_string(book->getId()) + "] ", 
                                      ConsoleUtil::Color::BRIGHT_YELLOW);
    std::cout << ConsoleUtil::colorText(book->getTitle(), ConsoleUtil::Color::BRIGHT_WHITE);
    std::cout << "       (作者: " << ConsoleUtil::colorText(book->getAuthor(), ConsoleUtil::Color::BRIGHT_CYAN);
    std::cout << ", 年份: " << ConsoleUtil::colorText(std::to_string(book->getYear()), ConsoleUtil::Color::BRIGHT_CYAN);
    std::cout << ")" << std::endl;
}

void Library::offerBookDetails(const std::vector<Book*>&) {
    ConsoleUtil::printInfo("\n請輸入圖書 ID 以查看詳情，或輸入 0 返回: ");
    int bookId = getMenuChoice();
    
    if (bookId > 0) {
        const Book* book = bookManager.getBook(bookId);
        if (book) {
            ConsoleUtil::printTitleWithSubtitle("搜尋圖書", "圖書詳情");
            book->display();
        } else {
            ConsoleUtil::printError("圖書未找到");
        }
    }
}

std::string Library::formatTime(time_t time) {
    char buffer[80];
    struct tm* timeInfo = localtime(&time);
    strftime(buffer, 80, "%Y-%m-%d", timeInfo);
    return std::string(buffer);
}

void Library::borrowBook() {
    auto availableBooks = getAvailableBooks();
    
    if (availableBooks.empty()) {
        ConsoleUtil::printTitle("借閱圖書");
        ConsoleUtil::printWarning("目前沒有可借閱的圖書");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    ConsoleUtil::printTitleWithSubtitle("借閱圖書", "可借閱的圖書");
    displayAvailableBooks(availableBooks);
    
    int bookId = getBookIdChoice("請輸入圖書 ID");
    std::string username = getBorrowerUsername();
    
    if (loanManager.borrowBook(username, bookId)) {
        ConsoleUtil::printSuccess("圖書借閱成功！");
    } else {
        ConsoleUtil::printError("圖書借閱失敗，請檢查圖書 ID 和使用者名稱");
    }
    
    ConsoleUtil::pauseAndWait();
}

std::vector<Book> Library::getAvailableBooks() {
    std::vector<Book> available;
    for (const auto& book : bookManager.getAllBooks()) {
        if (book.getAvailableCopies() > 0) {
            available.push_back(book);
        }
    }
    return available;
}

void Library::displayAvailableBooks(const std::vector<Book>& books) {
    for (const auto& book : books) {
        std::cout << ConsoleUtil::colorText("[ID: " + std::to_string(book.getId()) + "]", 
                                          ConsoleUtil::Color::BRIGHT_YELLOW);
        std::cout << " " << book.getTitle();
        std::cout << ConsoleUtil::colorText(" (" + std::to_string(book.getAvailableCopies()) + " 本可借)", 
                                          ConsoleUtil::Color::BRIGHT_GREEN);
        std::cout << std::endl;
    }
}

int Library::getBookIdChoice(const std::string& prompt) {
    ConsoleUtil::printInfo("\n" + prompt + ": ");
    return getMenuChoice();
}

std::string Library::getBorrowerUsername() {
    if (userManager.getCurrentUser()->getRole() == Role::Reader) {
        return userManager.getCurrentUser()->getUsername();
    } else {
        return getUserInput("請輸入讀者使用者名稱");
    }
}

void Library::returnBook() {
    std::string targetUser = getTargetUserForReturn();
    auto activeLoans = getActiveLoansForUser(targetUser);
    
    if (activeLoans.empty()) {
        showNoActiveLoansMessage(targetUser);
        return;
    }
    
    displayActiveLoans(activeLoans, targetUser);
    
    int bookId = getBookIdChoice("請輸入要歸還的圖書 ID");
    
    if (loanManager.returnBook(targetUser, bookId)) {
        ConsoleUtil::printSuccess("圖書歸還成功！");
        showFineIfAny(targetUser, bookId);
    } else {
        ConsoleUtil::printError("圖書歸還失敗");
    }
    
    ConsoleUtil::pauseAndWait();
}

std::string Library::getTargetUserForReturn() {
    if (userManager.getCurrentUser()->getRole() == Role::Reader) {
        return userManager.getCurrentUser()->getUsername();
    } else {
        ConsoleUtil::printTitle("歸還圖書");
        return getUserInput("請輸入讀者使用者名稱來查看其借閱記錄");
    }
}

std::vector<LoanRecord*> Library::getActiveLoansForUser(const std::string& username) {
    auto userLoans = loanManager.getLoansForUser(username);
    std::vector<LoanRecord*> activeLoans;
    
    for (auto* loan : userLoans) {
        if (!loan->isReturned()) {
            activeLoans.push_back(loan);
        }
    }
    
    return activeLoans;
}

void Library::showNoActiveLoansMessage(const std::string& username) {
    ConsoleUtil::printTitle("歸還圖書");
    if (username == userManager.getCurrentUser()->getUsername()) {
        ConsoleUtil::printWarning("您沒有需要歸還的圖書");
    } else {
        ConsoleUtil::printWarning("該使用者沒有需要歸還的圖書");
    }
    ConsoleUtil::pauseAndWait();
}

void Library::displayActiveLoans(const std::vector<LoanRecord*>& loans, const std::string& username) {
    ConsoleUtil::printTitleWithSubtitle("歸還圖書", "目前借閱記錄");
    
    for (const auto* loan : loans) {
        const Book* book = bookManager.getBook(loan->getBookId());
        if (book) {
            displayLoanRecord(loan, book, username != userManager.getCurrentUser()->getUsername());
        }
    }
}

void Library::displayLoanRecord(const LoanRecord* loan, const Book* book, bool showUsername) {
    std::string status = loan->isOverdue() ? 
        ConsoleUtil::colorText(" [逾期]", ConsoleUtil::Color::BRIGHT_RED) : 
        ConsoleUtil::colorText(" [正常]", ConsoleUtil::Color::BRIGHT_GREEN);
    
    std::cout << ConsoleUtil::colorText("[ID: " + std::to_string(book->getId()) + "]", 
                                      ConsoleUtil::Color::BRIGHT_YELLOW);
    std::cout << " " << book->getTitle();
    
    if (showUsername) {
        std::cout << " - " << loan->getUsername();
    }
    
    std::cout << status << std::endl;
}

void Library::showFineIfAny(const std::string& username, int bookId) {
    auto userLoans = loanManager.getLoansForUser(username);
    for (auto* loan : userLoans) {
        if (loan->getBookId() == bookId && loan->isReturned()) {
            double fine = loanManager.calculateFine(*loan);
            if (fine > 0) {
                ConsoleUtil::printWarning("此書逾期歸還，產生罰款: $" + std::to_string(static_cast<int>(fine)));
            }
            break;
        }
    }
}

// 編輯圖書
void Library::editBook() {
    if (!userManager.hasPermission(Role::Staff)) {
        ConsoleUtil::printError("權限拒絕：只有館員和管理員可以編輯圖書");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    ConsoleUtil::printTitle("編輯圖書");
    
    int bookId = getBookIdChoice("請輸入圖書 ID");
    Book* book = bookManager.getBook(bookId);
    
    if (!book) {
        ConsoleUtil::printError("圖書未找到");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    ConsoleUtil::printSubtitle("目前的圖書資訊");
    book->display();
    
    Book editedBook = *book;
    runEditMenu(editedBook);
}

void Library::runEditMenu(Book& book) {
    while (true) {
        auto options = createEditOptions(book);
        
        ConsoleUtil::printTitle("編輯選項");
        ConsoleUtil::printMenuOptions(options);
        
        int choice = getMenuChoice();
        
        if (choice >= 1 && choice <= 9) {
            editBookField(book, choice);
        } else if (choice == 10) {
            manageBookCategories(book);
        } else if (choice == 11) {
            if (saveBookChanges(book)) return;
        } else if (choice == 12) {
            ConsoleUtil::printWarning("已取消編輯");
            ConsoleUtil::pauseAndWait();
            return;
        } else {
            showInvalidChoice();
        }
    }
}

std::vector<std::string> Library::createEditOptions(const Book& book) {
    return {
        "書名 (目前: " + book.getTitle() + ")",
        "作者 (目前: " + book.getAuthor() + ")",
        "年份 (目前: " + std::to_string(book.getYear()) + ")",
        "ISBN (目前: " + book.getIsbn() + ")",
        "出版社 (目前: " + book.getPublisher() + ")",
        "語言 (目前: " + book.getLanguage() + ")",
        "頁數 (目前: " + std::to_string(book.getPageCount()) + ")",
        "複本數量 (目前: " + std::to_string(book.getTotalCopies()) + ")",
        "摘要",
        "管理分類",
        "保存更改並退出",
        "取消編輯"
    };
}

void Library::editBookField(Book& book, int field) {
    switch (field) {
        case 1: 
            book.setTitle(getUserInput("新書名"));
            break;
        case 2: 
            book.setAuthor(getUserInput("新作者"));
            break;
        case 3: {
            std::cout << "新年份: ";
            int year;
            std::cin >> year;
            clearInputBuffer();
            book.setYear(year);
            break;
        }
        case 4: 
            book.setIsbn(getUserInput("新 ISBN"));
            break;
        case 5: 
            book.setPublisher(getUserInput("新出版社"));
            break;
        case 6: 
            book.setLanguage(getUserInput("新語言"));
            break;
        case 7: {
            std::cout << "新頁數: ";
            int pages;
            std::cin >> pages;
            clearInputBuffer();
            book.setPageCount(pages);
            break;
        }
        case 8:
            updateBookCopies(book);
            break;
        case 9: 
            book.setSynopsis(getUserInput("新摘要"));
            break;
    }
}

void Library::updateBookCopies(Book& book) {
    std::cout << "新複本數量: ";
    int newCopies;
    std::cin >> newCopies;
    clearInputBuffer();
    
    int borrowedCopies = book.getTotalCopies() - book.getAvailableCopies();
    
    if (newCopies < borrowedCopies) {
        ConsoleUtil::printError("複本數量不能少於已借出的數量 (" + std::to_string(borrowedCopies) + ")");
    } else {
        int newAvailable = book.getAvailableCopies() + (newCopies - book.getTotalCopies());
        book.setTotalCopies(newCopies);
        book.setAvailableCopies(newAvailable);
    }
}

void Library::manageBookCategories(Book& book) {
    while (true) {
        ConsoleUtil::printTitle("管理分類");
        displayCurrentCategories(book);
        
        std::vector<std::string> options = {"新增分類", "刪除分類", "返回主編輯選單"};
        ConsoleUtil::printMenuOptions(options);
        
        int choice = getMenuChoice();
        
        if (choice == 1) {
            std::string category = getUserInput("新分類");
            book.addCategory(category);
        } else if (choice == 2) {
            removeCategoryFromBook(book);
        } else if (choice == 3) {
            break;
        } else {
            showInvalidChoice();
        }
    }
}

void Library::displayCurrentCategories(const Book& book) {
    const auto& categories = book.getCategories();
    std::cout << "目前分類: ";
    
    if (categories.empty()) {
        std::cout << "(無)" << std::endl;
    } else {
        for (size_t i = 0; i < categories.size(); ++i) {
            std::cout << categories[i];
            if (i < categories.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
}

void Library::removeCategoryFromBook(Book& book) {
    const auto& categories = book.getCategories();
    
    if (categories.empty()) {
        ConsoleUtil::printWarning("沒有分類可以刪除");
    } else {
        std::string category = getUserInput("要刪除的分類");
        book.removeCategory(category);
    }
}

bool Library::saveBookChanges(Book& book) {
    if (bookManager.updateBook(book)) {
        ConsoleUtil::printSuccess("圖書資訊更新成功");
        bookManager.saveToFile(bookFile);
        ConsoleUtil::pauseAndWait();
        return true;
    } else {
        ConsoleUtil::printError("更新圖書資訊失敗");
        return false;
    }
}

// 用戶借閱記錄的詳細實現
void Library::displayUserLoans() {
    std::string username = userManager.getCurrentUser()->getUsername();
    ConsoleUtil::printTitle("我的借閱記錄");
    
    auto userLoans = loanManager.getLoansForUser(username);
    
    if (userLoans.empty()) {
        ConsoleUtil::printWarning("您沒有借閱記錄");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    sortLoansByStatus(userLoans);
    displayCurrentAndReturnedLoans(userLoans);
    
    ConsoleUtil::pauseAndWait();
}

void Library::sortLoansByStatus(std::vector<LoanRecord*>& loans) {
    SortUtil::sort(loans, [](const LoanRecord* a, const LoanRecord* b) {
        // 目前借閱的排在前面
        if (a->isReturned() != b->isReturned()) {
            return !a->isReturned();
        }
        
        // 目前借閱的按到期日排序（最早的在前）
        if (!a->isReturned()) {
            return a->getDueDate() < b->getDueDate();
        }
        
        // 已歸還的按歸還日期排序（最近的在前）
        return a->getReturnDate() > b->getReturnDate();
    });
}

void Library::displayCurrentAndReturnedLoans(const std::vector<LoanRecord*>& loans) {
    bool hasCurrentLoans = displayLoanSection(loans, false, "目前借閱中的圖書");
    bool hasReturnedLoans = displayLoanSection(loans, true, "已歸還的圖書");
    
    if (!hasCurrentLoans) {
        std::cout << "沒有目前借閱中的圖書。" << std::endl;
    }
    
    if (!hasReturnedLoans) {
        std::cout << "沒有已歸還的圖書記錄。" << std::endl;
    }
}

bool Library::displayLoanSection(const std::vector<LoanRecord*>& loans, bool showReturned, const std::string& title) {
    bool hasLoans = false;
    ConsoleUtil::printSubtitle(title);
    
    for (const auto* loan : loans) {
        if (loan->isReturned() == showReturned) {
            hasLoans = true;
            displayDetailedLoanRecord(loan);
        }
    }
    
    return hasLoans;
}

void Library::displayDetailedLoanRecord(const LoanRecord* loan) {
    const Book* book = bookManager.getBook(loan->getBookId());
    if (!book) return;
    
    std::cout << "[" << loan->getBookId() << "] " << book->getTitle() 
              << " (" << book->getAuthor() << ")" << std::endl;
    std::cout << "   借閱日期: " << formatTime(loan->getBorrowDate()) << std::endl;
    
    if (loan->isReturned()) {
        std::cout << "   歸還日期: " << formatTime(loan->getReturnDate()) << std::endl;
        showFineForReturnedBook(loan);
    } else {
        std::cout << "   到期日: " << formatTime(loan->getDueDate()) << std::endl;
        showLoanStatus(loan);
    }
    
    std::cout << "------------------------------" << std::endl;
}

void Library::showFineForReturnedBook(const LoanRecord* loan) {
    if (loan->isOverdue()) {
        double fine = loanManager.calculateFine(*loan);
        if (fine > 0) {
            std::cout << "   罰款: $" << std::fixed << std::setprecision(2) << fine << std::endl;
        }
    }
}

void Library::showLoanStatus(const LoanRecord* loan) {
    if (loan->isOverdue()) {
        int overdueDays = loan->getDaysOverdue();
        double fine = loanManager.calculateFine(*loan);
        std::cout << "   逾期 " << overdueDays << " 天，預估罰款: $" 
                  << std::fixed << std::setprecision(2) << fine << std::endl;
    } else {
        time_t now = time(nullptr);
        int daysLeft = (loan->getDueDate() - now) / (60 * 60 * 24);
        std::cout << "   剩餘 " << daysLeft << " 天" << std::endl;
    }
}

// 智能推薦系統
void Library::showRecommendations() {
    ConsoleUtil::printTitle("智能推薦系統");
    
    std::string currentUsername = userManager.getCurrentUser()->getUsername();
    
    // 檢查使用者是否有借閱歷史
    auto userLoans = loanManager.getLoansForUser(currentUsername);
    
    if (userLoans.empty()) {
        ConsoleUtil::printWarning("您尚未有借閱記錄，無法提供個人化推薦");
        showPopularBooksRecommendation();
        return;
    }
    
    ConsoleUtil::printSuccess("正在為您生成個人化推薦...");
    std::cout << std::endl;
    
    // 顯示互動式推薦介面
    showInteractiveRecommendations(currentUsername);
}

void Library::showInteractiveRecommendations(const std::string& username) {
    ConsoleUtil::printSubtitle("===== 歡迎，" + username + "！以下是為您精選推薦 =====");
    
    // 獲取混合推薦
    auto hybridRecs = recommendationEngine.getHybridRecommendations(username, 5);
    
    if (hybridRecs.empty()) {
        ConsoleUtil::printWarning("暫時無法生成推薦，請稍後再試");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    // 顯示推薦列表
    displayRecommendationList(hybridRecs, username);
    
    // 提供互動選項
    handleRecommendationInteraction(hybridRecs);
}

void Library::displayRecommendationList(const std::vector<std::pair<int, double>>& recommendations, 
                                       const std::string& username) {
    auto cfRecs = recommendationEngine.getCollaborativeFilteringRecommendations(username, 10);
    
    std::unordered_map<int, double> cfScores;
    for (const auto& rec : cfRecs) {
        cfScores[rec.first] = rec.second;
    }
    
    int rank = 1;
    for (const auto& rec : recommendations) {
        const Book* book = bookManager.getBook(rec.first);
        if (!book) continue;
        
        // 計算內容相似度分數（如果有的話）
        double contentScore = 0.0;
        auto userLoans = loanManager.getLoansForUser(username);
        if (!userLoans.empty()) {
            // 使用用戶最近借閱的書籍作為參考
            int refBookId = userLoans.back()->getBookId();
            auto contentRecs = recommendationEngine.getContentBasedRecommendations(refBookId, 10);
            
            for (const auto& cRec : contentRecs) {
                if (cRec.first == rec.first) {
                    contentScore = cRec.second;
                    break;
                }
            }
        }
        
        double cfScore = cfScores.count(rec.first) ? cfScores[rec.first] : 0.0;
        
        // 格式化顯示
        std::cout << rank << ". ";
        std::cout << ConsoleUtil::colorText("[" + std::to_string(book->getId()) + "]", 
                                          ConsoleUtil::Color::BRIGHT_YELLOW);
        std::cout << " " << ConsoleUtil::colorText(book->getTitle(), ConsoleUtil::Color::BRIGHT_WHITE);
        
        // 截斷過長的標題
        std::string displayTitle = book->getTitle();
        if (displayTitle.length() > 25) {
            displayTitle = displayTitle.substr(0, 22) + "...";
        }
        
        // 顯示推薦原因
        if (cfScore > 0.0 && contentScore > 0.0) {
            std::cout << std::string(35 - displayTitle.length(), ' ');
            std::cout << "(混合推薦指數：" << std::fixed << std::setprecision(2) << rec.second << ")" << std::endl;
        } else if (cfScore > 0.0) {
            std::cout << std::string(35 - displayTitle.length(), ' ');
            std::cout << "(基於協同過濾推薦指數：" << std::fixed << std::setprecision(2) << cfScore << ")" << std::endl;
        } else if (contentScore > 0.0) {
            std::cout << std::string(35 - displayTitle.length(), ' ');
            std::cout << "(內容相似度：" << std::fixed << std::setprecision(2) << contentScore << ")" << std::endl;
        } else {
            std::cout << std::string(35 - displayTitle.length(), ' ');
            std::cout << "(推薦指數：" << std::fixed << std::setprecision(2) << rec.second << ")" << std::endl;
        }
        
        // 顯示作者和可借狀態
        std::cout << "   作者：" << ConsoleUtil::colorText(book->getAuthor(), ConsoleUtil::Color::BRIGHT_CYAN);
        
        if (book->getAvailableCopies() > 0) {
            std::cout << " | " << ConsoleUtil::colorText("可借閱", ConsoleUtil::Color::BRIGHT_GREEN);
            std::cout << " (" << book->getAvailableCopies() << " 本)";
        } else {
            std::cout << " | " << ConsoleUtil::colorText("已借完", ConsoleUtil::Color::BRIGHT_RED);
        }
        std::cout << std::endl;
        
        rank++;
    }
}

void Library::handleRecommendationInteraction(const std::vector<std::pair<int, double>>& recommendations) {
    std::cout << std::endl;
    ConsoleUtil::printInfo("輸入書號即可借閱，或按 Enter 返回主選單...");
    
    std::string input;
    std::getline(std::cin, input);
    
    if (input.empty()) {
        return; // 返回主選單
    }
    
    try {
        int bookId = std::stoi(input);
        
        bool found = false;
        for (const auto& rec : recommendations) {
            if (rec.first == bookId) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            ConsoleUtil::printWarning("請輸入推薦列表中的書號");
            ConsoleUtil::pauseAndWait();
            return;
        }
        
        std::string username = userManager.getCurrentUser()->getUsername();
        
        if (loanManager.borrowBook(username, bookId)) {
            ConsoleUtil::printSuccess("圖書借閱成功！");
            
            recommendationEngine.initialize(bookManager, loanManager);
        } else {
            ConsoleUtil::printError("圖書借閱失敗，請檢查圖書狀態");
        }
        
    } catch (const std::exception&) {
        ConsoleUtil::printError("無效的書號格式");
    }
    
    ConsoleUtil::pauseAndWait();
}

void Library::showWelcomeRecommendations(const std::string& username) {
    std::cout << std::endl;
    
    auto userLoans = loanManager.getLoansForUser(username);
    
    if (userLoans.empty()) {
        showPopularBooksForNewUser(username);
    } else {
        showPersonalizedRecommendations(username, userLoans);
    }
    
    std::cout << std::endl;
    std::cout << "按任意鍵繼續..." << std::endl;
    std::cin.get();
}

void Library::showPopularBooksForNewUser(const std::string& username) {
    std::cout << "===== 歡迎，" << username << "！以下是熱門圖書推薦 =====" << std::endl;
    
    auto bookStats = loanManager.getBookBorrowStats();
    std::vector<std::pair<int, int>> popularBooks;
    
    for (const auto& stat : bookStats) {
        popularBooks.push_back(stat);
    }
    
    SortUtil::sort(popularBooks, [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    
    int count = 0;
    for (const auto& bookStat : popularBooks) {
        if (count >= 5) break;
        
        const Book* book = bookManager.getBook(bookStat.first);
        if (book && book->getAvailableCopies() > 0) {
            std::cout << (count + 1) << ". [" << book->getId() << "] " 
                      << book->getTitle() 
                      << std::string(35 - std::min(book->getTitle().length(), size_t(34)), ' ')
                      << "(熱門借閱：" << bookStat.second << " 次)" << std::endl;
            count++;
        }
    }
    
    if (count == 0) {
        std::cout << "暫時沒有可推薦的圖書" << std::endl;
    }
}

void Library::showPersonalizedRecommendations(const std::string& username, 
                                            const std::vector<LoanRecord*>& userLoans) {
    std::cout << "===== 歡迎，" << username << "！以下是為您精選推薦 =====" << std::endl;
    
    auto hybridRecs = recommendationEngine.getHybridRecommendations(username, 5);
    
    if (hybridRecs.empty()) {
        std::cout << "暫時無法生成推薦，請稍後再試" << std::endl;
        return;
    }
    
    auto cfRecs = recommendationEngine.getCollaborativeFilteringRecommendations(username, 10);
    std::unordered_map<int, double> cfScores;
    for (const auto& rec : cfRecs) {
        cfScores[rec.first] = rec.second;
    }
    
    int rank = 1;
    for (const auto& rec : hybridRecs) {
        const Book* book = bookManager.getBook(rec.first);
        if (!book) continue;
        
        double contentScore = calculateContentScore(rec.first, userLoans);
        double cfScore = cfScores.count(rec.first) ? cfScores[rec.first] : 0.0;
        
        displayRecommendationItem(book, rec.second, cfScore, contentScore, rank);
        rank++;
    }
}

double Library::calculateContentScore(int bookId, const std::vector<LoanRecord*>& userLoans) {
    if (userLoans.empty()) return 0.0;
    
    int refBookId = userLoans.back()->getBookId();
    auto contentRecs = recommendationEngine.getContentBasedRecommendations(refBookId, 10);
    
    for (const auto& cRec : contentRecs) {
        if (cRec.first == bookId) {
            return cRec.second;
        }
    }
    
    return 0.0;
}

void Library::displayRecommendationItem(const Book* book, double hybridScore, 
                                       double cfScore, double contentScore, int rank) {
    std::cout << rank << ". [" << book->getId() << "] " << book->getTitle();
    
    size_t titleLen = book->getTitle().length();
    size_t padding = titleLen < 30 ? 30 - titleLen : 1;
    std::cout << std::string(padding, ' ');
    
    if (cfScore > 0.0 && contentScore > 0.0) {
        std::cout << "(混合推薦：" << std::fixed << std::setprecision(2) << hybridScore << ")";
    } else if (cfScore > 0.0) {
        std::cout << "(協同過濾：" << std::fixed << std::setprecision(2) << cfScore << ")";
    } else if (contentScore > 0.0) {
        std::cout << "(內容相似度：" << std::fixed << std::setprecision(2) << contentScore << ")";
    } else {
        std::cout << "(推薦指數：" << std::fixed << std::setprecision(2) << hybridScore << ")";
    }
    std::cout << std::endl;
}

void Library::showPopularBooksRecommendation() {
    ConsoleUtil::printSubtitle("熱門圖書推薦");
    
    auto bookStats = loanManager.getBookBorrowStats();
    std::vector<std::pair<int, int>> popularBooks;
    
    for (const auto& stat : bookStats) {
        popularBooks.push_back(stat);
    }
    
    SortUtil::sort(popularBooks, [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    
    int count = 0;
    for (const auto& bookStat : popularBooks) {
        if (count >= 5) break; // 只顯示前5本
        
        const Book* book = bookManager.getBook(bookStat.first);
        if (book && book->getAvailableCopies() > 0) {
            std::cout << (count + 1) << ". ";
            std::cout << ConsoleUtil::colorText("[" + std::to_string(book->getId()) + "]", 
                                              ConsoleUtil::Color::BRIGHT_YELLOW);
            std::cout << " " << ConsoleUtil::colorText(book->getTitle(), ConsoleUtil::Color::BRIGHT_WHITE);
            std::cout << " - " << book->getAuthor();
            std::cout << " (借閱次數: " << bookStat.second << ")" << std::endl;
            count++;
        }
    }
    
    if (count == 0) {
        ConsoleUtil::printWarning("暫時沒有可推薦的圖書");
    } else {
        std::cout << std::endl;
        ConsoleUtil::printInfo("開始借閱圖書以獲得個人化推薦！");
    }
    
    ConsoleUtil::pauseAndWait();
}

void Library::displayOverdueLoans() {
    ConsoleUtil::printTitle("逾期圖書");
    
    auto overdueLoans = loanManager.getOverdueLoans();
    
    if (overdueLoans.empty()) {
        ConsoleUtil::printSuccess("目前沒有逾期圖書");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    Role userRole = userManager.getCurrentUser()->getRole();
    
    if (userRole == Role::Reader) {
        // 讀者只能看自己的逾期記錄
        displayReaderOverdueLoans(overdueLoans);
    } else {
        // 管理員和館員可以看所有逾期記錄
        displayAllOverdueLoans(overdueLoans);
    }
    
    ConsoleUtil::pauseAndWait();
}

void Library::displayReaderOverdueLoans(const std::vector<LoanRecord*>& overdueLoans) {
    std::string username = userManager.getCurrentUser()->getUsername();
    std::vector<LoanRecord*> userOverdueLoans;
    
    for (auto* loan : overdueLoans) {
        if (loan->getUsername() == username) {
            userOverdueLoans.push_back(loan);
        }
    }
    
    if (userOverdueLoans.empty()) {
        ConsoleUtil::printSuccess("您沒有逾期圖書");
        return;
    }
    
    ConsoleUtil::printWarning("您有 " + std::to_string(userOverdueLoans.size()) + " 本逾期圖書:");
    
    double totalFine = 0.0;
    for (const auto* loan : userOverdueLoans) {
        const Book* book = bookManager.getBook(loan->getBookId());
        if (book) {
            std::cout << "[" << loan->getBookId() << "] " << book->getTitle() << std::endl;
            std::cout << "   逾期: " << loan->getDaysOverdue() << " 天" << std::endl;
            
            double fine = loanManager.calculateFine(*loan);
            totalFine += fine;
            std::cout << "   罰款: $" << std::fixed << std::setprecision(2) << fine << std::endl;
            std::cout << "---" << std::endl;
        }
    }
    
    ConsoleUtil::printWarning("總計罰款: $" + std::to_string(static_cast<int>(totalFine)));
}

void Library::displayAllOverdueLoans(const std::vector<LoanRecord*>& overdueLoans) {
    // 按使用者分組
    std::unordered_map<std::string, std::vector<LoanRecord*>> userOverdueLoans;
    
    for (auto* loan : overdueLoans) {
        userOverdueLoans[loan->getUsername()].push_back(loan);
    }
    
    ConsoleUtil::printInfo("總計 " + std::to_string(overdueLoans.size()) + " 本逾期圖書，" + 
                          std::to_string(userOverdueLoans.size()) + " 位讀者");
    
    for (const auto& userLoan : userOverdueLoans) {
        std::cout << "\n=== " << userLoan.first << " (" << userLoan.second.size() << " 本逾期) ===" << std::endl;
        
        double userTotalFine = 0.0;
        for (const auto* loan : userLoan.second) {
            const Book* book = bookManager.getBook(loan->getBookId());
            if (book) {
                std::cout << "[" << loan->getBookId() << "] " << book->getTitle() << std::endl;
                std::cout << "   逾期: " << loan->getDaysOverdue() << " 天" << std::endl;
                
                double fine = loanManager.calculateFine(*loan);
                userTotalFine += fine;
                std::cout << "   罰款: $" << std::fixed << std::setprecision(2) << fine << std::endl;
            }
        }
        
        std::cout << "小計罰款: $" << std::fixed << std::setprecision(2) << userTotalFine << std::endl;
    }
}

// 罰款政策設置
void Library::setFinePolicy() {
    if (!userManager.hasPermission(Role::Admin)) {
        ConsoleUtil::printError("權限拒絕：只有管理員可以設置罰款政策");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    const FinePolicy& currentPolicy = loanManager.getFinePolicy();
    showCurrentPolicy(currentPolicy);
    
    runPolicyMenu(currentPolicy);
}

void Library::showCurrentPolicy(const FinePolicy& policy) {
    ConsoleUtil::printTitleWithSubtitle("設置罰款政策", "目前政策");
    policy.display();
    ConsoleUtil::pauseAndWait();
}

void Library::runPolicyMenu(const FinePolicy& currentPolicy) {
    while (true) {
        auto options = createPolicyOptions(currentPolicy);
        
        ConsoleUtil::printTitleWithSubtitle("設置罰款政策", "設置選項");
        ConsoleUtil::printMenuOptions(options);
        
        int choice = getMenuChoice();
        
        if (handlePolicyChoice(choice, currentPolicy)) {
            return;
        }
    }
}

std::vector<std::string> Library::createPolicyOptions(const FinePolicy& policy) {
    return {
        "修改寬限期 (目前: " + std::to_string(policy.getGraceDays()) + " 天)",
        "修改固定費率 (目前: $" + std::to_string(policy.getFixedRate()) + " 每天)",
        "修改遞增因子 (目前: " + std::to_string(policy.getIncrementalFactor()) + ")",
        "使用預設政策",
        "儲存並退出",
        "取消設置"
    };
}

bool Library::handlePolicyChoice(int choice, const FinePolicy& currentPolicy) {
    FinePolicy newPolicy = currentPolicy;
    
    switch (choice) {
        case 1:
            updateGraceDays(newPolicy);
            break;
        case 2:
            updateFixedRate(newPolicy);
            break;
        case 3:
            updateIncFactor(newPolicy);
            break;
        case 4:
            useDefaultPolicy();
            break;
        case 5:
            return savePolicyAndExit();
        case 6:
            ConsoleUtil::printWarning("已取消罰款政策設置");
            ConsoleUtil::pauseAndWait();
            return true;
        default:
            showInvalidChoice();
            break;
    }
    
    return false;
}

void Library::updateGraceDays(FinePolicy& policy) {
    ConsoleUtil::printTitle("修改寬限期");
    ConsoleUtil::printInfo("請輸入新的寬限期 (天數, 0-30): ");
    
    int graceDays;
    std::cin >> graceDays;
    clearInputBuffer();
    
    if (graceDays < 0 || graceDays > 30) {
        ConsoleUtil::printError("無效的寬限期！必須在 0-30 天之間");
    } else {
        policy.setGraceDays(graceDays);
        loanManager.setFinePolicy(policy);
        ConsoleUtil::printSuccess("寬限期已更新為 " + std::to_string(graceDays) + " 天");
    }
    
    ConsoleUtil::pauseAndWait();
}

void Library::updateFixedRate(FinePolicy& policy) {
    ConsoleUtil::printTitle("修改固定費率");
    ConsoleUtil::printInfo("請輸入新的固定費率 (美元, 1-100): ");
    
    double fixedRate;
    std::cin >> fixedRate;
    clearInputBuffer();
    
    if (fixedRate < 1 || fixedRate > 100) {
        ConsoleUtil::printError("無效的固定費率！必須在 $1-$100 之間");
    } else {
        policy.setFixedRate(fixedRate);
        loanManager.setFinePolicy(policy);
        ConsoleUtil::printSuccess("固定費率已更新為 $" + std::to_string(fixedRate) + " 每天");
    }
    
    ConsoleUtil::pauseAndWait();
}

void Library::updateIncFactor(FinePolicy& policy) {
    ConsoleUtil::printTitle("修改遞增因子");
    ConsoleUtil::printInfo("請輸入新的遞增因子 (1.0-2.0): ");
    
    double incrementalFactor;
    std::cin >> incrementalFactor;
    clearInputBuffer();
    
    if (incrementalFactor < 1.0 || incrementalFactor > 2.0) {
        ConsoleUtil::printError("無效的遞增因子！必須在 1.0-2.0 之間");
    } else {
        policy.setIncrementalFactor(incrementalFactor);
        loanManager.setFinePolicy(policy);
        ConsoleUtil::printSuccess("遞增因子已更新為 " + std::to_string(incrementalFactor));
    }
    
    ConsoleUtil::pauseAndWait();
}

void Library::useDefaultPolicy() {
    ConsoleUtil::printTitle("使用預設政策");
    ConsoleUtil::printWarning("確認使用預設政策嗎？ (y/n): ");
    
    char confirm;
    std::cin >> confirm;
    clearInputBuffer();
    
    if (confirm == 'y' || confirm == 'Y') {
        FinePolicy defaultPolicy(2, 20, 1.0);
        loanManager.setFinePolicy(defaultPolicy);
        ConsoleUtil::printSuccess("已設定為預設罰款政策");
    }
    
    ConsoleUtil::pauseAndWait();
}

bool Library::savePolicyAndExit() {
    ConsoleUtil::printTitle("儲存設定");
    
    if (loanManager.saveToFile(loanFile)) {
        ConsoleUtil::printSuccess("罰款政策已成功儲存");
    } else {
        ConsoleUtil::printError("儲存罰款政策時發生錯誤");
    }
    
    ConsoleUtil::pauseAndWait();
    return true;
}

void Library::showStatistics() {
    if (!userManager.hasPermission(Role::Admin)) {
        ConsoleUtil::printError("權限拒絕：只有管理員可以查看統計數據");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    while (true) {
        ConsoleUtil::clearScreen();
        
        // 顯示統計總覽
        std::vector<std::string> options = {
            "📊 借閱次數統計", "📚 圖書分類統計", "📈 月度借閱統計", 
            "📋 系統總覽", "🔙 返回主選單"
        };
        
        ConsoleUtil::printTitleWithSubtitle("圖書館管理系統", "統計數據中心");
        
        // 快速統計摘要
        showQuickStatsSummary();
        
        ConsoleUtil::printSubtitle("統計選項");
        ConsoleUtil::printMenuOptions(options);
        
        int choice = getMenuChoice();
        
        switch (choice) {
            case 1: showBorrowStats(); break;
            case 2: showCategoryStats(); break; 
            case 3: showMonthlyStats(); break;
            case 4: showSystemOverview(); break;
            case 5: return;
            default: showInvalidChoice();
        }
    }
}

void Library::showAdvancedSearchHelp() {
    ConsoleUtil::printSubtitle("多條件智慧搜尋說明");
    
    ConsoleUtil::printInfo("支援的運算符:");
    std::cout << "  " << ConsoleUtil::colorText("AND", ConsoleUtil::Color::BRIGHT_GREEN) << " - 交集（同時滿足）" << std::endl;
    std::cout << "  " << ConsoleUtil::colorText("OR", ConsoleUtil::Color::BRIGHT_YELLOW) << " - 聯集（滿足其一）" << std::endl;
    std::cout << "  " << ConsoleUtil::colorText("NOT", ConsoleUtil::Color::BRIGHT_RED) << " - 差集（排除）" << std::endl;
    std::cout << "  " << ConsoleUtil::colorText("( )", ConsoleUtil::Color::BRIGHT_CYAN) << " - 括號（優先運算）" << std::endl;
    
    std::cout << std::endl;
    ConsoleUtil::printInfo("支援的欄位查詢:");
    std::cout << "  " << ConsoleUtil::colorText("作者=\"張三\"", ConsoleUtil::Color::BRIGHT_WHITE) << " - 精確匹配作者" << std::endl;
    std::cout << "  " << ConsoleUtil::colorText("title~\"程式\"", ConsoleUtil::Color::BRIGHT_WHITE) << " - 書名包含關鍵字" << std::endl;
    std::cout << "  " << ConsoleUtil::colorText("年份>=2020", ConsoleUtil::Color::BRIGHT_WHITE) << " - 年份條件" << std::endl;
    std::cout << "  " << ConsoleUtil::colorText("標籤~\"入門\"", ConsoleUtil::Color::BRIGHT_WHITE) << " - 標籤包含" << std::endl;
    
    std::cout << std::endl;
    ConsoleUtil::printInfo("運算符說明:");
    std::cout << "  = (等於)  ~ (包含)  > (大於)  < (小於)  >= (大於等於)  <= (小於等於)" << std::endl;
    
    std::cout << std::endl;
    ConsoleUtil::printSuccess("範例: 程式設計 AND (作者=\"陳鍾誠\" OR 年份>=2020) NOT 標籤~\"入門\"");
    std::cout << std::endl;
}

void Library::showSearchTutorial() {
    ConsoleUtil::printTitle("互動式搜尋教學");
    
    ConsoleUtil::printSubtitle("基本搜尋範例");
    
    // Example 1: Simple search
    ConsoleUtil::printInfo("範例 1: 簡單關鍵字搜尋");
    std::cout << "  輸入: " << ConsoleUtil::colorText("程式設計", ConsoleUtil::Color::BRIGHT_CYAN) << std::endl;
    std::cout << "  說明: 搜尋標題、作者或標籤中包含「程式設計」的圖書" << std::endl;
    std::cout << std::endl;
    
    // Example 2: Field-specific search
    ConsoleUtil::printInfo("範例 2: 欄位指定搜尋");
    std::cout << "  輸入: " << ConsoleUtil::colorText("作者=\"王大明\"", ConsoleUtil::Color::BRIGHT_CYAN) << std::endl;
    std::cout << "  說明: 搜尋作者名字完全等於「王大明」的圖書" << std::endl;
    std::cout << std::endl;
    
    // Example 3: Numeric conditions
    ConsoleUtil::printInfo("範例 3: 數值條件搜尋");
    std::cout << "  輸入: " << ConsoleUtil::colorText("年份>=2020", ConsoleUtil::Color::BRIGHT_CYAN) << std::endl;
    std::cout << "  說明: 搜尋 2020 年以後出版的圖書" << std::endl;
    std::cout << std::endl;
    
    // Example 4: Boolean operations
    ConsoleUtil::printInfo("範例 4: 布林運算組合");
    std::cout << "  輸入: " << ConsoleUtil::colorText("程式設計 AND 年份>=2020", ConsoleUtil::Color::BRIGHT_CYAN) << std::endl;
    std::cout << "  說明: 搜尋包含「程式設計」且 2020 年後出版的圖書" << std::endl;
    std::cout << std::endl;
    
    // Example 5: Complex query
    ConsoleUtil::printInfo("範例 5: 複雜組合查詢");
    std::cout << "  輸入: " << ConsoleUtil::colorText("(Java OR Python) AND 作者~\"陳\" NOT 標籤~\"入門\"", ConsoleUtil::Color::BRIGHT_CYAN) << std::endl;
    std::cout << "  說明: 搜尋關於 Java 或 Python，作者姓名包含「陳」，但標籤不包含「入門」的圖書" << std::endl;
    std::cout << std::endl;
    
    ConsoleUtil::printSubtitle("支援的欄位名稱");
    std::cout << "  " << ConsoleUtil::colorText("中文", ConsoleUtil::Color::BRIGHT_YELLOW) << ": 作者、標題、年份、標籤、出版社、語言、頁數、簡介" << std::endl;
    std::cout << "  " << ConsoleUtil::colorText("英文", ConsoleUtil::Color::BRIGHT_YELLOW) << ": author、title、year、category、publisher、language、pagecount、synopsis" << std::endl;
    std::cout << std::endl;
    
    ConsoleUtil::printSuccess("提示: 使用括號來控制運算優先順序，使用雙引號來包含空格的值");
    
    ConsoleUtil::pauseAndWait();
}

void Library::showQuickStatsSummary() {
    auto allBooks = bookManager.getAllBooks();
    auto overdueLoans = loanManager.getOverdueLoans();
    auto bookStats = loanManager.getBookBorrowStats();
    auto userStats = loanManager.getUserBorrowStats();
    
    int totalBooks = allBooks.size();
    int totalBorrows = 0;
    int activeLoans = 0;
    int availableBooks = 0;
    
    for (const auto& book : allBooks) {
        availableBooks += book.getAvailableCopies();
    }
    
    for (const auto& stat : bookStats) {
        totalBorrows += stat.second;
    }
    
    for (const auto& book : allBooks) {
        activeLoans += (book.getTotalCopies() - book.getAvailableCopies());
    }
    
    ConsoleUtil::printInfo("📊 系統概況");
    std::cout << "┌─────────────────────────────────────────────────────────────┐" << std::endl;
    std::cout << "│ " << ConsoleUtil::colorText("總圖書數量", ConsoleUtil::Color::BRIGHT_CYAN) 
              << ": " << std::setw(10) << totalBooks
              << " │ " << ConsoleUtil::colorText("可借圖書", ConsoleUtil::Color::BRIGHT_GREEN)
              << ": " << std::setw(10) << availableBooks << " │" << std::endl;
    std::cout << "│ " << ConsoleUtil::colorText("總借閱次數", ConsoleUtil::Color::BRIGHT_BLUE) 
              << ": " << std::setw(10) << totalBorrows
              << " │ " << ConsoleUtil::colorText("目前借出", ConsoleUtil::Color::BRIGHT_YELLOW)
              << ": " << std::setw(10) << activeLoans << " │" << std::endl;
    std::cout << "│ " << ConsoleUtil::colorText("逾期圖書", ConsoleUtil::Color::BRIGHT_RED) 
              << ": " << std::setw(10) << overdueLoans.size()
              << " │ " << ConsoleUtil::colorText("活躍用戶", ConsoleUtil::Color::BRIGHT_MAGENTA)
              << ": " << std::setw(10) << userStats.size() << " │" << std::endl;
    std::cout << "└─────────────────────────────────────────────────────────────┘" << std::endl << std::endl;
}

void Library::showBorrowStats() {
    ConsoleUtil::clearScreen();
    ConsoleUtil::printTitle("借閱統計分析");
    
    auto bookStats = loanManager.getBookBorrowStats();
    
    if (bookStats.empty()) {
        ConsoleUtil::printWarning("暫無借閱數據");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    // 圖書借閱排行榜
    std::vector<std::pair<std::string, int>> topBooks;
    
    for (const auto& stat : bookStats) {
        const Book* book = bookManager.getBook(stat.first);
        if (book) {
            std::string displayTitle = book->getTitle();
            if (displayTitle.length() > 25) {
                displayTitle = displayTitle.substr(0, 22) + "...";
            }
            topBooks.push_back({displayTitle, stat.second});
        }
    }
    
    SortUtil::sort(topBooks, [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    
    if (topBooks.size() > 15) {
        topBooks.resize(15);
    }
    
    VisualizationUtil::drawBarChart(topBooks, "📚 熱門圖書排行榜 (Top 15)", 40);
    
    // 用戶活躍度排行榜
    auto userStats = loanManager.getUserBorrowStats();
    std::vector<std::pair<std::string, int>> topUsers;
    
    for (const auto& stat : userStats) {
        topUsers.push_back({stat.first, stat.second});
    }
    
    SortUtil::sort(topUsers, [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    
    if (topUsers.size() > 10) {
        topUsers.resize(10);
    }
    
    VisualizationUtil::drawBarChart(topUsers, "👤 活躍讀者排行榜 (Top 10)", 40);
    
    // 借閱數據摘要
    showBorrowingSummary(bookStats, userStats);
    
    ConsoleUtil::pauseAndWait();
}

void Library::showBorrowingSummary(const std::unordered_map<int, int>& bookStats, 
                                  const std::unordered_map<std::string, int>& userStats) {
    int totalBorrows = 0;
    int maxBookBorrows = 0;
    int maxUserBorrows = 0;
    
    for (const auto& stat : bookStats) {
        totalBorrows += stat.second;
        maxBookBorrows = std::max(maxBookBorrows, stat.second);
    }
    
    for (const auto& stat : userStats) {
        maxUserBorrows = std::max(maxUserBorrows, stat.second);
    }
    
    double avgBorrowsPerBook = bookStats.empty() ? 0 : (double)totalBorrows / bookStats.size();
    double avgBorrowsPerUser = userStats.empty() ? 0 : (double)totalBorrows / userStats.size();
    
    std::cout << "📈 " << ConsoleUtil::colorText("借閱數據摘要", ConsoleUtil::Color::BRIGHT_CYAN) << std::endl;
    std::cout << "┌─────────────────────────────────────────────────────────────┐" << std::endl;
    std::cout << "│ 總借閱次數: " << std::setw(10) << totalBorrows 
              << " │ 平均每本圖書: " << std::setw(8) << std::fixed << std::setprecision(1) 
              << avgBorrowsPerBook << " 次 │" << std::endl;
    std::cout << "│ 最熱門圖書: " << std::setw(10) << maxBookBorrows << " 次"
              << " │ 平均每位讀者: " << std::setw(8) << std::fixed << std::setprecision(1) 
              << avgBorrowsPerUser << " 次 │" << std::endl;
    std::cout << "│ 最活躍讀者: " << std::setw(10) << maxUserBorrows << " 次"
              << " │ 參與圖書數量: " << std::setw(10) << bookStats.size() << " 本 │" << std::endl;
    std::cout << "└─────────────────────────────────────────────────────────────┘" << std::endl << std::endl;
}

void Library::showCategoryStats() {
    ConsoleUtil::clearScreen();
    ConsoleUtil::printTitle("圖書分類統計");
    
    std::unordered_map<std::string, int> categoryCount;
    std::unordered_map<std::string, int> categoryBorrows;
    
    // 統計圖書數量分佈
    for (const auto& book : bookManager.getAllBooks()) {
        const auto& categories = book.getCategories();
        if (categories.empty()) {
            categoryCount["未分類"]++;
        } else {
            for (const auto& category : categories) {
                categoryCount[category]++;
            }
        }
    }
    
    VisualizationUtil::drawPieChart(categoryCount, "📚 圖書類別分佈");
    
    // 統計借閱次數分佈
    auto bookStats = loanManager.getBookBorrowStats();
    
    for (const auto& stat : bookStats) {
        const Book* book = bookManager.getBook(stat.first);
        if (book) {
            const auto& categories = book->getCategories();
            if (categories.empty()) {
                categoryBorrows["未分類"] += stat.second;
            } else {
                for (const auto& category : categories) {
                    categoryBorrows[category] += stat.second;
                }
            }
        }
    }
    
    VisualizationUtil::drawBarChart(categoryBorrows, "📊 各類別借閱熱度", 40);
    
    // 分類效率分析
    showCategoryEfficiency(categoryCount, categoryBorrows);
    
    ConsoleUtil::pauseAndWait();
}

void Library::showCategoryEfficiency(const std::unordered_map<std::string, int>& categoryCount,
                                    const std::unordered_map<std::string, int>& categoryBorrows) {
    std::vector<std::pair<std::string, double>> efficiency;
    
    for (const auto& count : categoryCount) {
        auto borrowIt = SearchUtil::mapFind(categoryBorrows, count.first);
        int borrows = (borrowIt != categoryBorrows.end()) ? borrowIt->second : 0;
        double eff = count.second > 0 ? (double)borrows / count.second : 0;
        efficiency.push_back({count.first, eff});
    }
    
    SortUtil::sort(efficiency, [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    
    std::cout << "🎯 " << ConsoleUtil::colorText("分類效率分析 (平均每本借閱次數)", ConsoleUtil::Color::BRIGHT_CYAN) << std::endl;
    std::cout << "┌─────────────────────────────────────────────────────────────┐" << std::endl;
    
    for (size_t i = 0; i < std::min(efficiency.size(), size_t(5)); i++) {
        ConsoleUtil::Color color = ConsoleUtil::Color::BRIGHT_GREEN;
        if (i >= 3) color = ConsoleUtil::Color::BRIGHT_YELLOW;
        
        std::cout << "│ " << std::left << std::setw(15) << efficiency[i].first
                  << ": " << ConsoleUtil::colorText(
                      std::to_string((int)(efficiency[i].second * 10) / 10.0), color)
                  << " 次/本" << std::string(35, ' ') << " │" << std::endl;
    }
    
    std::cout << "└─────────────────────────────────────────────────────────────┘" << std::endl << std::endl;
}

void Library::showMonthlyStats() {
    ConsoleUtil::clearScreen();
    ConsoleUtil::printTitle("月度借閱趨勢分析");
    
    auto monthlyStats = loanManager.getMonthlyStats();
    std::vector<std::pair<std::string, int>> monthlyData;
    
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    int currentYear = timeinfo->tm_year + 1900;
    int currentMonth = timeinfo->tm_mon + 1;
    
    // 生成最近12個月的數據
    for (int i = 11; i >= 0; i--) {
        int month = currentMonth - i;
        int year = currentYear;
        
        if (month <= 0) {
            month += 12;
            year -= 1;
        }
        
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%04d-%02d", year, month);
        std::string monthKey = buffer;
        
        // 格式化月份顯示
        char displayBuffer[10];
        snprintf(displayBuffer, sizeof(displayBuffer), "%02d月", month);
        
        int count = 0;
        for (const auto& stat : monthlyStats) {
            if (stat.first == monthKey) {
                count = stat.second;
                break;
            }
        }
        
        monthlyData.push_back({displayBuffer, count});
    }
    
    VisualizationUtil::drawLineChart(monthlyData, "📈 月度借閱趨勢 (近12個月)", 50);
    
    // 月度統計摘要
    showMonthlyStatsSummary(monthlyData);
    
    ConsoleUtil::pauseAndWait();
}

void Library::showMonthlyStatsSummary(const std::vector<std::pair<std::string, int>>& monthlyData) {
    if (monthlyData.empty()) return;
    
    int total = 0;
    int maxMonth = 0;
    int minMonth = INT_MAX;
    std::string maxMonthName, minMonthName;
    
    for (const auto& data : monthlyData) {
        total += data.second;
        if (data.second > maxMonth) {
            maxMonth = data.second;
            maxMonthName = data.first;
        }
        if (data.second < minMonth) {
            minMonth = data.second;
            minMonthName = data.first;
        }
    }
    
    double average = (double)total / monthlyData.size();
    
    std::cout << "📊 " << ConsoleUtil::colorText("月度統計摘要", ConsoleUtil::Color::BRIGHT_CYAN) << std::endl;
    std::cout << "┌─────────────────────────────────────────────────────────────┐" << std::endl;
    std::cout << "│ 12個月總計: " << std::setw(8) << total << " 次"
              << " │ 月平均借閱: " << std::setw(8) << std::fixed << std::setprecision(1) 
              << average << " 次 │" << std::endl;
    std::cout << "│ 最高月份: " << std::setw(6) << maxMonthName << " (" << maxMonth << " 次)"
              << " │ 最低月份: " << std::setw(6) << minMonthName << " (" << minMonth << " 次) │" << std::endl;
    std::cout << "└─────────────────────────────────────────────────────────────┘" << std::endl << std::endl;
}

void Library::showSystemOverview() {
    ConsoleUtil::clearScreen();
    ConsoleUtil::printTitle("系統全面概覽");
    
    // 系統狀態總覽
    showDetailedSystemStatus();
    
    // 近期活動摘要
    showRecentActivitySummary();
    
    ConsoleUtil::pauseAndWait();
}

void Library::showDetailedSystemStatus() {
    auto allBooks = bookManager.getAllBooks();
    auto allUsers = userManager.getAllUsers();
    auto allLoans = loanManager.getAllLoans();
    auto overdueLoans = loanManager.getOverdueLoans();
    
    int totalCopies = 0;
    int availableCopies = 0;
    int adminCount = 0, staffCount = 0, readerCount = 0;
    
    for (const auto& book : allBooks) {
        totalCopies += book.getTotalCopies();
        availableCopies += book.getAvailableCopies();
    }
    
    for (const auto& user : allUsers) {
        switch (user.getRole()) {
            case Role::Admin: adminCount++; break;
            case Role::Staff: staffCount++; break;
            case Role::Reader: readerCount++; break;
        }
    }
    
    std::cout << "🏛️ " << ConsoleUtil::colorText("圖書館系統狀態", ConsoleUtil::Color::BRIGHT_CYAN) << std::endl;
    std::cout << "╔═════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║ 📚 圖書館藏: " << std::setw(8) << allBooks.size() << " 種"
              << " │ 📦 館藏複本: " << std::setw(8) << totalCopies << " 本 ║" << std::endl;
    std::cout << "║ 📖 可借圖書: " << std::setw(8) << availableCopies << " 本"
              << " │ 📊 借出比例: " << std::setw(6) << std::fixed << std::setprecision(1)
              << (totalCopies > 0 ? (double)(totalCopies - availableCopies) / totalCopies * 100 : 0) << "% ║" << std::endl;
    std::cout << "╠═════════════════════════════════════════════════════════════╣" << std::endl;
    std::cout << "║ 👥 系統用戶: " << std::setw(8) << allUsers.size() << " 人"
              << " │ 📈 借閱記錄: " << std::setw(8) << allLoans.size() << " 筆 ║" << std::endl;
    std::cout << "║ 👑 管理員: " << std::setw(10) << adminCount << " 人"
              << " │ 👨‍💼 館員: " << std::setw(12) << staffCount << " 人 ║" << std::endl;
    std::cout << "║ 👤 讀者: " << std::setw(12) << readerCount << " 人"
              << " │ ⚠️  逾期: " << std::setw(12) << overdueLoans.size() << " 筆 ║" << std::endl;
    std::cout << "╚═════════════════════════════════════════════════════════════╝" << std::endl << std::endl;
}

void Library::showRecentActivitySummary() {
    auto allLoans = loanManager.getAllLoans();
    
    // 計算最近7天和30天的活動
    time_t now = time(nullptr);
    time_t week_ago = now - (7 * 24 * 60 * 60);
    time_t month_ago = now - (30 * 24 * 60 * 60);
    
    int recent_borrows_7d = 0, recent_returns_7d = 0;
    int recent_borrows_30d = 0, recent_returns_30d = 0;
    
    for (const auto* loan : allLoans) {
        if (loan->getBorrowDate() >= week_ago) {
            recent_borrows_7d++;
        }
        if (loan->getBorrowDate() >= month_ago) {
            recent_borrows_30d++;
        }
        if (loan->isReturned()) {
            if (loan->getReturnDate() >= week_ago) {
                recent_returns_7d++;
            }
            if (loan->getReturnDate() >= month_ago) {
                recent_returns_30d++;
            }
        }
    }
    
    std::cout << "⏰ " << ConsoleUtil::colorText("近期活動摘要", ConsoleUtil::Color::BRIGHT_CYAN) << std::endl;
    std::cout << "┌─────────────────────────────────────────────────────────────┐" << std::endl;
    std::cout << "│ 最近7天:  借出 " << std::setw(3) << recent_borrows_7d << " 本"
              << " │ 歸還 " << std::setw(3) << recent_returns_7d << " 本           │" << std::endl;
    std::cout << "│ 最近30天: 借出 " << std::setw(3) << recent_borrows_30d << " 本"
              << " │ 歸還 " << std::setw(3) << recent_returns_30d << " 本           │" << std::endl;
    std::cout << "│ 日均借閱: " << std::setw(5) << std::fixed << std::setprecision(1) 
              << (recent_borrows_30d / 30.0) << " 本/天"
              << " │ 日均歸還: " << std::setw(5) << std::fixed << std::setprecision(1) 
              << (recent_returns_30d / 30.0) << " 本/天 │" << std::endl;
    std::cout << "└─────────────────────────────────────────────────────────────┘" << std::endl << std::endl;
}

// 書籍列表功能實現
void Library::viewBookList() {
    ConsoleUtil::printTitle("書籍列表瀏覽");
    
    auto allBooks = bookManager.getAllBooks();
    
    if (allBooks.empty()) {
        ConsoleUtil::printWarning("目前沒有任何圖書");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    const int booksPerPage = 20;
    int currentPage = 1;
    
    // 0=NONE, 1=TITLE, 2=AUTHOR, 3=YEAR, 4=PAGES
    int currentSortField = 0;
    // 0=ASC, 1=DESC
    int currentSortOrder = 0;
    
    while (true) {
        auto sortedBooks = allBooks;
        applySorting(sortedBooks, currentSortField, currentSortOrder);
        
        int totalPages = (sortedBooks.size() + booksPerPage - 1) / booksPerPage;
        
        ConsoleUtil::clearScreen();
        ConsoleUtil::printTitleWithSubtitle("書籍列表", 
            "第 " + std::to_string(currentPage) + " / " + std::to_string(totalPages) + " 頁");
        
        displayBookPage(sortedBooks, currentPage, booksPerPage, currentSortField, currentSortOrder);
        
        std::vector<std::string> navOptions;
        
        if (currentPage > 1) {
            navOptions.push_back("上一頁");
        }
        if (currentPage < totalPages) {
            navOptions.push_back("下一頁");
        }
        navOptions.push_back("跳到指定頁");
        navOptions.push_back("重新排列");
        navOptions.push_back("檢視書籍詳情");
        navOptions.push_back("回到主選單");
        
        ConsoleUtil::printSubtitle("導航選項");
        ConsoleUtil::printMenuOptions(navOptions);
        
        int choice = getMenuChoice();
        
        if (!handleBookListNavigation(choice, currentPage, totalPages, sortedBooks, navOptions,
                                     currentSortField, currentSortOrder)) {
            break;
        }
    }
}

void Library::displayBookPage(const std::vector<Book>& books, int page, int booksPerPage,
                             int currentSortField, int currentSortOrder) {
    int startIndex = (page - 1) * booksPerPage;
    int endIndex = std::min(startIndex + booksPerPage, static_cast<int>(books.size()));
    
    ConsoleUtil::printInfo("顯示第 " + std::to_string(startIndex + 1) + 
                          " - " + std::to_string(endIndex) + " 本圖書 (共 " + 
                          std::to_string(books.size()) + " 本)");
    std::cout << std::endl;
    
    // 計算每欄的最大寬度
    auto columnWidths = calculateColumnWidths(books, startIndex, endIndex);
    
    // 排序箭頭符號
    std::string titleArrow = getSortArrow(currentSortField, currentSortOrder, 1);
    std::string authorArrow = getSortArrow(currentSortField, currentSortOrder, 2);
    std::string yearArrow = getSortArrow(currentSortField, currentSortOrder, 3);
    std::string pagesArrow = getSortArrow(currentSortField, currentSortOrder, 4);
    
    // 表頭
    std::cout << ConsoleUtil::colorText("ID", ConsoleUtil::Color::BRIGHT_YELLOW);
    std::cout << std::string(columnWidths.idWidth - getDisplayWidth("ID"), ' ');
    
    std::string titleHeader = "書名" + titleArrow;
    std::cout << ConsoleUtil::colorText(titleHeader, ConsoleUtil::Color::BRIGHT_CYAN);
    std::cout << std::string(columnWidths.titleWidth - getDisplayWidth(titleHeader), ' ');
    
    std::string authorHeader = "作者" + authorArrow;
    std::cout << ConsoleUtil::colorText(authorHeader, ConsoleUtil::Color::BRIGHT_GREEN);
    std::cout << std::string(columnWidths.authorWidth - getDisplayWidth(authorHeader), ' ');
    
    std::string yearHeader = "年份" + yearArrow;
    std::cout << ConsoleUtil::colorText(yearHeader, ConsoleUtil::Color::BRIGHT_BLUE);
    std::cout << std::string(columnWidths.yearWidth - getDisplayWidth(yearHeader), ' ');
    
    std::string pagesHeader = "頁數" + pagesArrow;
    std::cout << ConsoleUtil::colorText(pagesHeader, ConsoleUtil::Color::BRIGHT_MAGENTA);
    std::cout << std::string(columnWidths.pagesWidth - getDisplayWidth(pagesHeader), ' ');
    
    std::cout << ConsoleUtil::colorText("狀態", ConsoleUtil::Color::BRIGHT_WHITE) << std::endl;
    
    // 分隔線
    int totalWidth = columnWidths.idWidth + columnWidths.titleWidth + columnWidths.authorWidth + 
                     columnWidths.yearWidth + columnWidths.pagesWidth + columnWidths.statusWidth;
    std::cout << ConsoleUtil::colorText(std::string(totalWidth, '='), 
                                      ConsoleUtil::Color::BRIGHT_BLACK) << std::endl;
    
    // 顯示書籍列表
    for (int i = startIndex; i < endIndex; i++) {
        const Book& book = books[i];
        
        // 交替行顏色
        ConsoleUtil::Color rowColor = (i % 2 == 0) ? ConsoleUtil::Color::BRIGHT_WHITE : ConsoleUtil::Color::WHITE;
        
        // ID欄位
        std::string idStr = std::to_string(book.getId());
        std::cout << ConsoleUtil::colorText(idStr, ConsoleUtil::Color::BRIGHT_YELLOW);
        std::cout << std::string(columnWidths.idWidth - getDisplayWidth(idStr), ' ');
        
        // 書名欄位
        std::string title = book.getTitle();
        if (getDisplayWidth(title) > columnWidths.titleWidth - 2) {
            title = truncateToWidth(title, columnWidths.titleWidth - 2);
        }
        std::cout << ConsoleUtil::colorText(title, rowColor);
        std::cout << std::string(columnWidths.titleWidth - getDisplayWidth(title), ' ');
        
        // 作者欄位
        std::string author = book.getAuthor();
        if (getDisplayWidth(author) > columnWidths.authorWidth - 2) {
            author = truncateToWidth(author, columnWidths.authorWidth - 2);
        }
        std::cout << ConsoleUtil::colorText(author, rowColor);
        std::cout << std::string(columnWidths.authorWidth - getDisplayWidth(author), ' ');
        
        // 年份欄位
        std::string yearStr = std::to_string(book.getYear());
        std::cout << ConsoleUtil::colorText(yearStr, rowColor);
        std::cout << std::string(columnWidths.yearWidth - getDisplayWidth(yearStr), ' ');
        
        // 頁數欄位
        std::string pagesStr = std::to_string(book.getPageCount());
        std::cout << ConsoleUtil::colorText(pagesStr, rowColor);
        std::cout << std::string(columnWidths.pagesWidth - getDisplayWidth(pagesStr), ' ');
        
        // 狀態欄位
        std::string statusStr;
        if (book.getAvailableCopies() > 0) {
            statusStr = "可借(" + std::to_string(book.getAvailableCopies()) + ")";
            std::cout << ConsoleUtil::colorText(statusStr, ConsoleUtil::Color::BRIGHT_GREEN);
        } else {
            statusStr = "借完";
            std::cout << ConsoleUtil::colorText(statusStr, ConsoleUtil::Color::BRIGHT_RED);
        }
        
        std::cout << std::endl;
    }
    
    // 底部分隔線
    std::cout << ConsoleUtil::colorText(std::string(totalWidth, '='), 
                                      ConsoleUtil::Color::BRIGHT_BLACK) << std::endl;
}

bool Library::handleBookListNavigation(int choice, int& currentPage, int totalPages, 
                                       const std::vector<Book>&, 
                                       const std::vector<std::string>&,
                                       int& currentSortField, int& currentSortOrder) {
    int optionIndex = 1;
    
    // 上一頁選項
    if (currentPage > 1) {
        if (choice == optionIndex) {
            currentPage--;
            return true;
        }
        optionIndex++;
    }
    
    // 下一頁選項
    if (currentPage < totalPages) {
        if (choice == optionIndex) {
            currentPage++;
            return true;
        }
        optionIndex++;
    }
    
    // 跳到指定頁
    if (choice == optionIndex) {
        jumpToPage(currentPage, totalPages);
        return true;
    }
    optionIndex++;
    
    // 重新排列
    if (choice == optionIndex) {
        showSortMenu(currentSortField, currentSortOrder);
        currentPage = 1; // 重新排序後回到第一頁
        return true;
    }
    optionIndex++;
    
    // 檢視書籍詳情
    if (choice == optionIndex) {
        viewBookDetailsFromList();
        return true;
    }
    optionIndex++;
    
    // 回到主選單
    if (choice == optionIndex) {
        return false;
    }
    
    showInvalidChoice();
    return true;
}

void Library::jumpToPage(int& currentPage, int totalPages) {
    ConsoleUtil::printInfo("請輸入要跳轉的頁碼 (1-" + std::to_string(totalPages) + "): ");
    int targetPage;
    std::cin >> targetPage;
    clearInputBuffer();
    
    if (targetPage >= 1 && targetPage <= totalPages) {
        currentPage = targetPage;
        ConsoleUtil::printSuccess("已跳轉到第 " + std::to_string(targetPage) + " 頁");
    } else {
        ConsoleUtil::printError("無效的頁碼！請輸入 1-" + std::to_string(totalPages) + " 之間的數字");
        ConsoleUtil::pauseAndWait();
    }
}

void Library::viewBookDetailsFromList() {
    int bookId = getBookIdChoice("請輸入要查看詳情的書籍 ID");
    
    if (bookId <= 0) {
        return;
    }
    
    const Book* book = bookManager.getBook(bookId);
    
    if (!book) {
        ConsoleUtil::printError("找不到 ID 為 " + std::to_string(bookId) + " 的書籍");
        ConsoleUtil::pauseAndWait();
        return;
    }
    
    ConsoleUtil::clearScreen();
    displayBookDetailsHeader(book);
    displayBookBasicInfo(book);
    displayBookInventoryStatus(book);
    displayBookCategories(book);
    displayBookSynopsis(book);
    displayBookBorrowStats(book);
    
        ConsoleUtil::pauseAndWait();
}

// 排序輔助函數
void Library::applySorting(std::vector<Book>& books, int sortField, int sortOrder) {
    if (sortField == 0) return; // NONE
    
    bool ascending = (sortOrder == 0); // ASC = 0, DESC = 1
    
    SortUtil::sort(books, [sortField, ascending](const Book& a, const Book& b) {
        bool result = false;
        
        switch (sortField) {
            case 1: // TITLE
                result = a.getTitle() < b.getTitle();
                break;
            case 2: // AUTHOR
                result = a.getAuthor() < b.getAuthor();
                break;
            case 3: // YEAR
                result = a.getYear() < b.getYear();
                break;
            case 4: // PAGES
                result = a.getPageCount() < b.getPageCount();
                break;
            default:
                return false;
        }
        
        return ascending ? result : !result;
    });
}

std::string Library::getSortArrow(int currentSortField, int currentSortOrder, int fieldId) {
    if (currentSortField != fieldId) {
        return "";
    }
    
    return (currentSortOrder == 0) ? " ▲" : " ▼"; // ASC = 0 (▲), DESC = 1 (▼)
}

void Library::showSortMenu(int& currentSortField, int& currentSortOrder) {
    ConsoleUtil::printTitle("重新排列選項");
    
    std::vector<std::string> sortOptions = {
        "依書名排序", "依作者排序", "依出版年份排序", "依頁數排序", "取消排序", "返回"
    };
    
    // 顯示當前排序狀態
    if (currentSortField != 0) {
        std::string fieldName;
        switch (currentSortField) {
            case 1: fieldName = "書名"; break;
            case 2: fieldName = "作者"; break;
            case 3: fieldName = "出版年份"; break;
            case 4: fieldName = "頁數"; break;
        }
        
        std::string orderName = (currentSortOrder == 0) ? "升序" : "降序";
        ConsoleUtil::printInfo("目前排序：依 " + fieldName + " " + orderName);
    } else {
        ConsoleUtil::printInfo("目前排序：無排序");
    }
    
    std::cout << std::endl;
    ConsoleUtil::printMenuOptions(sortOptions);
    
    int choice = getMenuChoice();
    
    switch (choice) {
        case 1: // 書名
            setSortField(currentSortField, currentSortOrder, 1);
            break;
        case 2: // 作者
            setSortField(currentSortField, currentSortOrder, 2);
            break;
        case 3: // 年份
            setSortField(currentSortField, currentSortOrder, 3);
            break;
        case 4: // 頁數
            setSortField(currentSortField, currentSortOrder, 4);
            break;
        case 5: // 取消排序
            currentSortField = 0; // NONE
            currentSortOrder = 0; // ASC
            ConsoleUtil::printSuccess("已取消排序");
            break;
        case 6: // 返回
            return;
        default:
            showInvalidChoice();
        return;
    }
    
    ConsoleUtil::pauseAndWait();
}

void Library::setSortField(int& currentSortField, int& currentSortOrder, int newField) {
    if (currentSortField == newField) {
        // 如果是同一欄位，切換排序順序
        currentSortOrder = 1 - currentSortOrder; // 0 <-> 1
    } else {
        // 如果是新欄位，設為升序
        currentSortField = newField;
        currentSortOrder = 0; // ASC
    }
    
    std::string fieldName;
    switch (newField) {
        case 1: fieldName = "書名"; break;
        case 2: fieldName = "作者"; break;
        case 3: fieldName = "出版年份"; break;
        case 4: fieldName = "頁數"; break;
    }
    
    std::string orderName = (currentSortOrder == 0) ? "升序" : "降序";
    ConsoleUtil::printSuccess("已設定排序：依 " + fieldName + " " + orderName);
}

// 計算每欄的最大寬度
Library::ColumnWidths Library::calculateColumnWidths(const std::vector<Book>& books, int startIndex, int endIndex) {
    ColumnWidths widths = {0, 0, 0, 0, 0, 0};
    
    // 最小寬度（考慮顯示寬度）
    widths.idWidth = std::max(widths.idWidth, size_t(4)); // "ID" + 空格
    widths.titleWidth = std::max(widths.titleWidth, getDisplayWidth("書名") + 4); // "書名" + 箭頭 + 空格
    widths.authorWidth = std::max(widths.authorWidth, getDisplayWidth("作者") + 4); // "作者" + 箭頭 + 空格
    widths.yearWidth = std::max(widths.yearWidth, getDisplayWidth("年份") + 4); // "年份" + 箭頭 + 空格
    widths.pagesWidth = std::max(widths.pagesWidth, getDisplayWidth("頁數") + 4); // "頁數" + 箭頭 + 空格
    widths.statusWidth = std::max(widths.statusWidth, getDisplayWidth("狀態") + 2); // "狀態" + 空格
    
    // 計算實際內容的最大顯示寬度
    for (int i = startIndex; i < endIndex; i++) {
        const Book& book = books[i];
        
        // ID欄位
        std::string idStr = std::to_string(book.getId());
        widths.idWidth = std::max(widths.idWidth, getDisplayWidth(idStr) + 2);
        
        // 書名欄位
        std::string title = book.getTitle();
        widths.titleWidth = std::max(widths.titleWidth, getDisplayWidth(title) + 2);
        
        // 作者欄位
        std::string author = book.getAuthor();
        widths.authorWidth = std::max(widths.authorWidth, getDisplayWidth(author) + 2);
        
        // 年份欄位
        std::string yearStr = std::to_string(book.getYear());
        widths.yearWidth = std::max(widths.yearWidth, getDisplayWidth(yearStr) + 2);
        
        // 頁數欄位
        std::string pagesStr = std::to_string(book.getPageCount());
        widths.pagesWidth = std::max(widths.pagesWidth, getDisplayWidth(pagesStr) + 2);
        
        // 狀態欄位
        std::string statusStr;
        if (book.getAvailableCopies() > 0) {
            statusStr = "可借(" + std::to_string(book.getAvailableCopies()) + ")";
        } else {
            statusStr = "借完";
        }
        widths.statusWidth = std::max(widths.statusWidth, getDisplayWidth(statusStr) + 2);
    }
    
    return widths;
}

// 計算字符串的顯示寬度（中文字符占2個寬度）
size_t Library::getDisplayWidth(const std::string& str) {
    size_t width = 0;
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = str[i];
        if (c < 0x80) {
            // ASCII字符
            width += 1;
            i += 1;
        } else if ((c & 0xE0) == 0xC0) {
            // 2字節UTF-8字符
            width += 2;
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // 3字節UTF-8字符（包含大部分中文）
            width += 2;
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            // 4字節UTF-8字符
            width += 2;
            i += 4;
        } else {
            // 無效字符，跳過
            i += 1;
        }
    }
    return width;
}

std::string Library::truncateToWidth(const std::string& str, size_t maxWidth) {
    if (maxWidth <= 3) return "...";
    
    std::string result;
    size_t currentWidth = 0;
    
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = str[i];
        size_t charWidth = 0;
        size_t charBytes = 0;
        
        if (c < 0x80) {
            charWidth = 1;
            charBytes = 1;
        } else if ((c & 0xE0) == 0xC0) {
            charWidth = 2;
            charBytes = 2;
        } else if ((c & 0xF0) == 0xE0) {
            charWidth = 2;
            charBytes = 3;
        } else if ((c & 0xF8) == 0xF0) {
            charWidth = 2;
            charBytes = 4;
        } else {
            i += 1;
            continue;
        }
        
        if (currentWidth + charWidth > maxWidth - 3) {
            result += "...";
            break;
        }
        
        result += str.substr(i, charBytes);
        currentWidth += charWidth;
        i += charBytes;
    }
    
    return result;
}

Library::PopularityInfo Library::calculateRelativePopularity(int bookId, const std::unordered_map<int, int>& bookStats) const {
    PopularityInfo info;
    
    auto it = SearchUtil::mapFind(bookStats, bookId);
    info.borrowCount = (it != bookStats.end()) ? it->second : 0;
    
    if (bookStats.empty() || info.borrowCount == 0) {
        info.percentile = 0.0;
        info.relativeToMean = 0.0;
        info.relativeToMedian = 0.0;
        info.level = "📚 新書/無借閱";
        info.description = "尚未有人借閱";
        return info;
    }
    
    std::vector<int> allCounts;
    int totalBorrows = 0;
    for (const auto& stat : bookStats) {
        allCounts.push_back(stat.second);
        totalBorrows += stat.second;
    }
    
    SortUtil::sort(allCounts, [](const int& a, const int& b) {
        return a < b;
    });
    
    double mean = static_cast<double>(totalBorrows) / allCounts.size();
    double median = 0.0;
    
    size_t n = allCounts.size();
    if (n % 2 == 0) {
        median = (allCounts[n/2 - 1] + allCounts[n/2]) / 2.0;
    } else {
        median = allCounts[n/2];
    }
    
    int lowerCount = 0;
    for (int count : allCounts) {
        if (count < info.borrowCount) {
            lowerCount++;
        }
    }
    info.percentile = (static_cast<double>(lowerCount) / allCounts.size()) * 100.0;
    
    // 計算相對於平均值和中位數的比例
    info.relativeToMean = mean > 0 ? (static_cast<double>(info.borrowCount) / mean) : 0.0;
    info.relativeToMedian = median > 0 ? (static_cast<double>(info.borrowCount) / median) : 0.0;
    
    if (info.percentile >= 90.0 && info.relativeToMean >= 2.0) {
        info.level = "🔥 極度熱門";
        info.description = "館內最熱門的 10% 圖書，借閱次數遠超平均";
    } else if (info.percentile >= 80.0 && info.relativeToMean >= 1.5) {
        info.level = "⭐ 非常熱門";
        info.description = "館內最熱門的 20% 圖書，深受讀者喜愛";
    } else if (info.percentile >= 60.0 || info.relativeToMean >= 1.2) {
        info.level = "👍 頗受歡迎";
        info.description = "借閱次數超過大部分圖書，頗受歡迎";
    } else if (info.percentile >= 30.0 || info.relativeToMedian >= 0.8) {
        info.level = "📖 普通受歡迎";
        info.description = "借閱次數中等，有一定讀者群";
    } else if (info.borrowCount >= 1) {
        info.level = "🆕 較冷門";
        info.description = "借閱次數較少，可能是新書或小眾圖書";
    } else {
        info.level = "📚 新書/無借閱";
        info.description = "尚未有人借閱";
    }
    
    return info;
}

void Library::displayPopularityLevel(const PopularityInfo& info) const {
    ConsoleUtil::Color levelColor;
    
    if (SearchUtil::contains(info.level, "極度熱門")) {
        levelColor = ConsoleUtil::Color::BRIGHT_RED;
    } else if (SearchUtil::contains(info.level, "非常熱門")) {
        levelColor = ConsoleUtil::Color::BRIGHT_YELLOW;
    } else if (SearchUtil::contains(info.level, "頗受歡迎")) {
        levelColor = ConsoleUtil::Color::BRIGHT_GREEN;
    } else if (SearchUtil::contains(info.level, "普通受歡迎")) {
        levelColor = ConsoleUtil::Color::BRIGHT_CYAN;
    } else {
        levelColor = ConsoleUtil::Color::BRIGHT_BLUE;
    }
    
    std::cout << ConsoleUtil::colorText(info.level, levelColor) << std::endl;
    std::cout << "   " << info.description << std::endl;
    
    if (info.borrowCount > 0) {
        std::cout << "   統計指標: ";
        std::cout << "排名前 " << std::fixed << std::setprecision(1) << (100.0 - info.percentile) << "%";
        
        if (info.relativeToMean > 0) {
            std::cout << " | 為平均值的 " << std::fixed << std::setprecision(1) << info.relativeToMean << " 倍";
        }
        
        if (info.relativeToMedian > 0) {
            std::cout << " | 為中位數的 " << std::fixed << std::setprecision(1) << info.relativeToMedian << " 倍";
        }
        
        std::cout << std::endl;
    }
}
