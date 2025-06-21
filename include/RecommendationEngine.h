#ifndef RECOMMENDATION_ENGINE_H
#define RECOMMENDATION_ENGINE_H

#include <vector>
#include <unordered_map>
#include <string>
#include <utility>
#include "Book.h"
#include "LoanManager.h"
#include "BookManager.h"

class RecommendationEngine {
private:
    // 協同過濾資料
    std::unordered_map<std::string, std::unordered_set<int>> userLoans; // userId -> set of bookIds
    std::unordered_map<int, std::unordered_map<int, int>> cooccurrenceMatrix; // bookId -> (bookId -> count)
    
    // 內容式推薦資料
    std::vector<std::string> vocabulary; // 所有不重複的詞彙
    std::unordered_map<std::string, int> wordToIndex; // 詞彙 -> 向量中的索引
    std::unordered_map<int, std::vector<double>> tfidfVectors; // bookId -> tfidf 向量
    std::unordered_map<std::string, double> idf; // 詞彙 -> IDF 值
    
    // 建立矩陣和向量
    void buildUserLoanMatrix(const LoanManager& loanManager);
    void buildCooccurrenceMatrix();
    void buildVocabulary(const BookManager& bookManager);
    void computeIDF(const BookManager& bookManager);
    void computeTFIDFVectors(const BookManager& bookManager);
    
    // 輔助函數
    std::vector<std::string> tokenize(const std::string& text) const;
    double computeCosineSimilarity(const std::vector<double>& v1, const std::vector<double>& v2) const;
    std::string formatBookTitle(const std::string& title) const;
    
    // 顯示輔助工具
    void displayHybridRecommendations(const std::vector<std::pair<int, double>>& hybrid,
                                    const BookManager& bookManager) const;
    void displayCollaborativeFilteringDetails(const std::string& username,
                                             const BookManager& bookManager, int count) const;
    void displayContentBasedDetails(const std::string& username,
                                   const BookManager& bookManager, int count) const;

public:
    RecommendationEngine();
    
    // 用資料初始化推薦引擎
    void initialize(const BookManager& bookManager, const LoanManager& loanManager);
    
    // 取得推薦結果
    std::vector<std::pair<int, double>> getCollaborativeFilteringRecommendations(
        const std::string& username, int count) const;
    std::vector<std::pair<int, double>> getContentBasedRecommendations(
        int bookId, int count) const;
    std::vector<std::pair<int, double>> getHybridRecommendations(
        const std::string& username, int count) const;
    
    // 顯示功能
    void displayRecommendations(const std::string& username, 
                                const BookManager& bookManager, int count) const;
};

#endif // RECOMMENDATION_ENGINE_H 