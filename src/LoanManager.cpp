#include "../include/LoanManager.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include "../include/SimpleJSON.h"
#include "../include/SearchUtil.h"

using JSONValue = SimpleJSON::JSONValue;

// Helper function to format time
std::string formatDate(time_t t) {
    char buffer[20];
    struct tm* timeInfo = localtime(&t);
    strftime(buffer, 20, "%Y-%m-%d", timeInfo);
    return std::string(buffer);
}

LoanManager::LoanManager() {
}

LoanManager::LoanManager(const std::string& filename) {
    loadFromFile(filename);
}

// Loan operations
bool LoanManager::borrowBook(const std::string& username, int bookId, int graceDays) {
    // Create loan record
    time_t now = time(nullptr);
    time_t dueDate = now + (14 * 24 * 60 * 60); // Due in 14 days
    
    LoanRecord loan(username, bookId, now, dueDate, graceDays);
    
    // Add to collections
    loans.push_back(loan);
    bookLoans[bookId].push_back(&loans.back());
    userLoans[username].push_back(&loans.back());
    
    return true;
}

bool LoanManager::returnBook(const std::string& username, int bookId) {
    // Find the loan record
    LoanRecord* loan = nullptr;
    for (auto* record : userLoans[username]) {
        if (record->getBookId() == bookId && !record->isReturned()) {
            loan = record;
            break;
        }
    }
    
    if (!loan) {
        return false;
    }
    
    // Update loan record - set return date
    loan->setReturnDate(time(nullptr));
    
    return true;
}

// Get loans
std::vector<LoanRecord*> LoanManager::getLoansForUser(const std::string& username) const {
    auto it = SearchUtil::mapFind(userLoans, username);
    if (it == userLoans.end()) {
        return std::vector<LoanRecord*>();
    }
    return it->second;
}

std::vector<LoanRecord*> LoanManager::getLoansForBook(int bookId) const {
    auto it = SearchUtil::mapFind(bookLoans, bookId);
    if (it == bookLoans.end()) {
        return std::vector<LoanRecord*>();
    }
    return it->second;
}

std::vector<LoanRecord*> LoanManager::getOverdueLoans() const {
    std::vector<LoanRecord*> overdueLoans;
    
    for (const auto& loan : loans) {
        if (!loan.isReturned() && loan.isOverdue()) {
            overdueLoans.push_back(const_cast<LoanRecord*>(&loan));
        }
    }
    
    return overdueLoans;
}

std::vector<LoanRecord*> LoanManager::getAllLoans() const {
    std::vector<LoanRecord*> allLoans;
    
    for (const auto& loan : loans) {
        allLoans.push_back(const_cast<LoanRecord*>(&loan));
    }
    
    return allLoans;
}

// Fine policy
void LoanManager::setFinePolicy(const FinePolicy& policy) {
    finePolicy = policy;
}

FinePolicy LoanManager::getFinePolicy() const {
    return finePolicy;
}

double LoanManager::calculateFine(const LoanRecord& loan) const {
    int overdueDays = loan.getDaysOverdue();
    return finePolicy.calculateFine(overdueDays);
}

// File operations
bool LoanManager::loadFromFile(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        std::string jsonStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        auto j = SimpleJSON::parseJSON(jsonStr);
        
        loans.clear();
        bookLoans.clear();
        userLoans.clear();
        
        // Load fine policy
        if (j->contains("finePolicy")) {
            auto policyJson = j->at("finePolicy");
            finePolicy = FinePolicy(
                policyJson->at("graceDays")->getInt(),
                policyJson->at("fixedRate")->getNumber(),
                policyJson->at("incrementalFactor")->getNumber()
            );
        }
        
        // Load loans
        if (j->contains("loans")) {
            const auto& loansArray = j->at("loans")->getArray();
            for (const auto& loanJsonPtr : loansArray) {
                LoanRecord loan(
                    loanJsonPtr->at("username")->getString(),
                    loanJsonPtr->at("bookId")->getInt(),
                    loanJsonPtr->at("borrowDate")->getInt(),
                    loanJsonPtr->at("dueDate")->getInt(),
                    finePolicy.getGraceDays()
                );
                
                if (loanJsonPtr->contains("returnDate")) {
                    loan.setReturnDate(loanJsonPtr->at("returnDate")->getInt());
                }
                
                loans.push_back(loan);
            }
        }
        
        // Rebuild lookup maps
        for (auto& loan : loans) {
            bookLoans[loan.getBookId()].push_back(&loan);
            userLoans[loan.getUsername()].push_back(&loan);
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading loans: " << e.what() << std::endl;
        return false;
    }
}

bool LoanManager::saveToFile(const std::string& filename) const {
    try {
        auto j = SimpleJSON::JSONValue::createObject();
        
        // Save fine policy
        auto finePolicyJson = SimpleJSON::JSONValue::createObject();
        finePolicyJson->set("graceDays", finePolicy.getGraceDays());
        finePolicyJson->set("fixedRate", finePolicy.getFixedRate());
        finePolicyJson->set("incrementalFactor", finePolicy.getIncrementalFactor());
        j->set("finePolicy", finePolicyJson);
        
        // Save loans
        auto loansArray = SimpleJSON::JSONValue::createArray();
        for (const auto& loan : loans) {
            auto loanJson = SimpleJSON::JSONValue::createObject();
            loanJson->set("bookId", loan.getBookId());
            loanJson->set("username", loan.getUsername());
            loanJson->set("borrowDate", static_cast<int>(loan.getBorrowDate()));
            loanJson->set("dueDate", static_cast<int>(loan.getDueDate()));
            
            if (loan.isReturned()) {
                loanJson->set("returnDate", static_cast<int>(loan.getReturnDate()));
            }
            
            loansArray->push_back(loanJson);
        }
        j->set("loans", loansArray);
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        file << SimpleJSON::stringifyJSON(j, 4);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving loans: " << e.what() << std::endl;
        return false;
    }
}

// Statistics and visualization
std::unordered_map<int, int> LoanManager::getBookBorrowStats() const {
    std::unordered_map<int, int> stats;
    
    for (const auto& loan : loans) {
        stats[loan.getBookId()]++;
    }
    
    return stats;
}

std::unordered_map<std::string, int> LoanManager::getUserBorrowStats() const {
    std::unordered_map<std::string, int> stats;
    
    for (const auto& loan : loans) {
        stats[loan.getUsername()]++;
    }
    
    return stats;
}

std::vector<std::pair<std::string, int>> LoanManager::getMonthlyStats() const {
    std::unordered_map<std::string, int> statsMap;
    
    for (const auto& loan : loans) {
        time_t borrowTime = loan.getBorrowDate();
        struct tm* timeInfo = localtime(&borrowTime);
        
        char buffer[8];
        strftime(buffer, 8, "%Y-%m", timeInfo);
        std::string month(buffer);
        
        statsMap[month]++;
    }
    
    std::vector<std::pair<std::string, int>> stats;
    for (const auto& pair : statsMap) {
        stats.push_back(pair);
    }
    
    return stats;
}

// Display
void LoanManager::displayUserLoans(const std::string& username) const {
    std::cout << "===== Loans for " << username << " =====" << std::endl;
    
    auto it = SearchUtil::mapFind(userLoans, username);
    if (it == userLoans.end() || it->second.empty()) {
        std::cout << "No loans found." << std::endl;
        return;
    }
    
    for (const auto* loan : it->second) {
        std::cout << "Book ID: " << loan->getBookId() << "\n"
                  << "Borrow Date: " << formatDate(loan->getBorrowDate()) << "\n"
                  << "Due Date: " << formatDate(loan->getDueDate()) << "\n";
        
        if (loan->isReturned()) {
            std::cout << "Return Date: " << formatDate(loan->getReturnDate()) << "\n";
            
            // Calculate fine for returned books that were overdue
            if (loan->isOverdue()) {
                double fine = calculateFine(*loan);
                std::cout << "Fine: $" << std::fixed << std::setprecision(2) 
                          << fine << "\n";
            }
        } else if (loan->isOverdue()) {
            int days = loan->getDaysOverdue();
            double fine = finePolicy.calculateFine(days);
            
            std::cout << "Status: Overdue (" << days << " days)\n"
                      << "Estimated Fine: $" << std::fixed << std::setprecision(2) 
                      << fine << "\n";
        } else {
            std::cout << "Status: Borrowed\n";
        }
        
        std::cout << "------------------------------\n";
    }
}

void LoanManager::displayOverdueLoans() const {
    std::cout << "===== Overdue Loans =====" << std::endl;
    
    bool hasOverdue = false;
    
    for (const auto& loan : loans) {
        if (!loan.isReturned() && loan.isOverdue()) {
            hasOverdue = true;
            
            int days = loan.getDaysOverdue();
            double fine = finePolicy.calculateFine(days);
            
            std::cout << "Book ID: " << loan.getBookId() << "\n"
                      << "Username: " << loan.getUsername() << "\n"
                      << "Due Date: " << formatDate(loan.getDueDate()) << "\n"
                      << "Days Overdue: " << days << "\n"
                      << "Fine: $" << std::fixed << std::setprecision(2) << fine << "\n"
                      << "------------------------------\n";
        }
    }
    
    if (!hasOverdue) {
        std::cout << "No overdue loans found." << std::endl;
    }
}

void LoanManager::displayLoanHistory() const {
    std::cout << "===== Loan History =====" << std::endl;
    
    for (const auto& loan : loans) {
        std::cout << "Book ID: " << loan.getBookId() << "\n"
                  << "Username: " << loan.getUsername() << "\n"
                  << "Borrow Date: " << formatDate(loan.getBorrowDate()) << "\n"
                  << "Due Date: " << formatDate(loan.getDueDate()) << "\n";
        
        if (loan.isReturned()) {
            std::cout << "Return Date: " << formatDate(loan.getReturnDate()) << "\n";
            
            if (loan.isOverdue()) {
                std::cout << "Status: Returned Late\n";
                
                // Calculate fine for returned books that were overdue
                double fine = calculateFine(loan);
                if (fine > 0) {
                    std::cout << "Fine: $" << std::fixed << std::setprecision(2) 
                              << fine << "\n";
                }
            } else {
                std::cout << "Status: Returned On Time\n";
            }
        } else if (loan.isOverdue()) {
            int days = loan.getDaysOverdue();
            std::cout << "Status: Overdue (" << days << " days)\n";
        } else {
            std::cout << "Status: Borrowed\n";
        }
        
        std::cout << "------------------------------\n";
    }
} 