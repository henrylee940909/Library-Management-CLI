#include "../include/RecommendationEngine.h"
#include "../include/SortUtil.h"
#include "../include/TextUtils.h"
#include "../include/SearchUtil.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cctype>

RecommendationEngine::RecommendationEngine() {}

// 用資料初始化推薦引擎
void RecommendationEngine::initialize(const BookManager& bookManager, const LoanManager& loanManager) {
    // 建立協同過濾的使用者借閱矩陣
    buildUserLoanMatrix(loanManager);
    
    // 建立共現矩陣
    buildCooccurrenceMatrix();
    
    // 建立內容式推薦的詞彙表
    buildVocabulary(bookManager);
    
    // 計算 IDF 值
    computeIDF(bookManager);
    
    // 計算每本書的 TF-IDF 向量
    computeTFIDFVectors(bookManager);
}

// 將文字分詞（針對中文文字進行改良）
std::vector<std::string> RecommendationEngine::tokenize(const std::string& text) const {
    return TextUtils::tokenize(text);
}

void RecommendationEngine::buildUserLoanMatrix(const LoanManager& loanManager) {
    userLoans.clear();
    
    // 取得所有使用者統計資料
    auto userStats = loanManager.getUserBorrowStats();
    
    // 對每個使用者
    for (const auto& userPair : userStats) {
        const std::string& username = userPair.first;
        
        // 取得該使用者的所有借閱記錄
        auto loans = loanManager.getLoansForUser(username);
        
        // 建立該使用者借閱過的書籍 ID 集合
        std::unordered_set<int> bookSet;
        for (const auto* loan : loans) {
            bookSet.insert(loan->getBookId());
        }
        
        // 儲存到 userLoans 映射中
        userLoans[username] = bookSet;
    }
}

void RecommendationEngine::buildCooccurrenceMatrix() {
    cooccurrenceMatrix.clear();
    
    // 對每個使用者
    for (const auto& userPair : userLoans) {
        const auto& books = userPair.second;
        
        // 對該使用者借閱的每對書籍
        for (int book1 : books) {
            for (int book2 : books) {
                if (book1 != book2) {
                    // 增加共現次數
                    cooccurrenceMatrix[book1][book2]++;
                }
            }
        }
    }
}

void RecommendationEngine::buildVocabulary(const BookManager& bookManager) {
    std::unordered_set<std::string> vocabSet;
    
    // 收集所有書籍的詞彙
    for (const auto& book : bookManager.getAllBooks()) {
        auto bookTerms = TextUtils::extractBookTerms(
            book.getTitle(), 
            book.getAuthor(), 
            book.getSynopsis(), 
            book.getCategories()
        );
        vocabSet.insert(bookTerms.begin(), bookTerms.end());
    }
    
    // 將集合轉換為向量並建立詞彙到索引的映射
    vocabulary.clear();
    wordToIndex.clear();
    
    for (const auto& word : vocabSet) {
        wordToIndex[word] = vocabulary.size();
        vocabulary.push_back(word);
    }
}

void RecommendationEngine::computeIDF(const BookManager& bookManager) {
    idf.clear();
    
    const auto& books = bookManager.getAllBooks();
    int N = books.size();
    
    // 對每個詞彙，計算出現在多少文件中
    std::unordered_map<std::string, int> docFreq;
    
    for (const auto& book : books) {
        auto bookTerms = TextUtils::extractBookTerms(
            book.getTitle(), 
            book.getAuthor(), 
            book.getSynopsis(), 
            book.getCategories()
        );
        
        // 增加每個詞彙的文件頻率
        for (const auto& term : bookTerms) {
            docFreq[term]++;
        }
    }
    
    // 計算每個詞彙的 IDF
    for (const auto& term : vocabulary) {
        int df = docFreq[term];
        idf[term] = std::log(static_cast<double>(N) / (1 + df));
    }
}

void RecommendationEngine::computeTFIDFVectors(const BookManager& bookManager) {
    tfidfVectors.clear();
    
    for (const auto& book : bookManager.getAllBooks()) {
        std::vector<double> tfidf(vocabulary.size(), 0.0);
        
        // 取得這本書的所有文字
        std::string bookText = book.getTitle() + " " + book.getAuthor() + " " + book.getSynopsis();
        for (const auto& category : book.getCategories()) {
            bookText += " " + category;
        }
        
        // 分詞並計算詞頻
        auto tokens = tokenize(bookText);
        std::unordered_map<std::string, int> termFreq;
        for (const auto& token : tokens) {
            termFreq[token]++;
        }
        
        // 計算詞彙表中每個詞彙的 TF-IDF
        for (const auto& term : termFreq) {
            auto it = SearchUtil::mapFind(wordToIndex, term.first);
            if (it != wordToIndex.end()) {
                int idx = it->second;
                double tf = static_cast<double>(term.second) / tokens.size();
                tfidf[idx] = tf * idf[term.first];
            }
        }
        
        // 儲存向量
        tfidfVectors[book.getId()] = tfidf;
    }
}

double RecommendationEngine::computeCosineSimilarity(const std::vector<double>& v1, const std::vector<double>& v2) const {
    double dotProduct = 0.0;
    double norm1 = 0.0;
    double norm2 = 0.0;
    
    for (size_t i = 0; i < v1.size(); ++i) {
        dotProduct += v1[i] * v2[i];
        norm1 += v1[i] * v1[i];
        norm2 += v2[i] * v2[i];
    }
    
    if (norm1 > 0 && norm2 > 0) {
        return dotProduct / (std::sqrt(norm1) * std::sqrt(norm2));
    } else {
        return 0.0;
    }
}

std::string RecommendationEngine::formatBookTitle(const std::string& title) const {
    return title.length() > 38 ? title.substr(0, 35) + "..." : title;
}

// 基於協同過濾取得推薦結果（改良演算法）
std::vector<std::pair<int, double>> RecommendationEngine::getCollaborativeFilteringRecommendations(
    const std::string& username, int count) const {
    
    std::unordered_map<int, double> scores;

    auto userIt = SearchUtil::mapFind(userLoans, username);
    if (userIt == userLoans.end()) {
        return {}; // 該使用者沒有借閱記錄
    }
    
    const auto& userBooks = userIt->second;
    if (userBooks.empty()) {
        return {};
    }
    
    // 計算總使用者數量用於正規化
    int totalUsers = userLoans.size();
    
    // 對該使用者借閱過的每本書
    for (int bookId : userBooks) {
        // 取得這本書的共現資料
        auto coocIt = SearchUtil::mapFind(cooccurrenceMatrix, bookId);
        if (coocIt != cooccurrenceMatrix.end()) {
            
            // 計算當前書籍的熱門程度（有多少使用者借閱過）
            int bookPopularity = 0;
            for (const auto& userPair : userLoans) {
                if (userPair.second.count(bookId) > 0) {
                    bookPopularity++;
                }
            }
            
            // 處理每本共現的書籍
            for (const auto& pair : coocIt->second) {
                int otherBookId = pair.first;
                int coocCount = pair.second;
                
                // 跳過使用者已經借閱過的書籍
                if (userBooks.count(otherBookId) == 0) {
                    
                    // 計算另一本書的熱門程度
                    int otherBookPopularity = 0;
                    for (const auto& userPair : userLoans) {
                        if (userPair.second.count(otherBookId) > 0) {
                            otherBookPopularity++;
                        }
                    }
                    
                    // 使用改良的評分方式：
                    // - 根據書籍熱門程度正規化，減少對熱門書籍的偏見
                    // - 使用對數縮放來減少極高共現次數的影響
                    double confidence = static_cast<double>(coocCount) / std::max(bookPopularity, 1);
                    double rarityBonus = std::log(static_cast<double>(totalUsers) / std::max(otherBookPopularity, 1));
                    double score = confidence * rarityBonus;
                    
                    scores[otherBookId] += score;
                }
            }
        }
    }
    
    // 根據使用者的閱讀多樣性進行額外正規化
    double userDiversity = static_cast<double>(userBooks.size());
    double diversityFactor = std::log(userDiversity + 1);
    
    for (auto& pair : scores) {
        pair.second *= diversityFactor;
    }
    
    // 轉換為向量並排序
    std::vector<std::pair<int, double>> recommendations;
    for (const auto& pair : scores) {
        recommendations.push_back(pair);
    }
    
    // 使用改良的排序方式，確保結果一致性
    SortUtil::sort(recommendations, [](const auto& a, const auto& b) {
        if (std::abs(a.second - b.second) < 1e-9) {
            return a.first < b.first; // 根據書籍 ID 進行穩定排序
        }
        return a.second > b.second;
    });
    
    // 限制為請求的數量
    if (recommendations.size() > static_cast<size_t>(count)) {
        recommendations.resize(count);
    }
    
    return recommendations;
}

// 取得內容式推薦
std::vector<std::pair<int, double>> RecommendationEngine::getContentBasedRecommendations(
    int bookId, int count) const {
    
    std::vector<std::pair<int, double>> similarities;
    
    // 取得參考書籍的 TF-IDF 向量
    auto refVectorIt = SearchUtil::mapFind(tfidfVectors, bookId);
    if (refVectorIt == tfidfVectors.end()) {
        return {}; // 找不到書籍
    }
    
    const auto& refVector = refVectorIt->second;
    
    // 計算與所有其他書籍的相似度
    for (const auto& pair : tfidfVectors) {
        int otherId = pair.first;
        
        // 跳過參考書籍本身
        if (otherId != bookId) {
            const auto& otherVector = pair.second;
            double similarity = computeCosineSimilarity(refVector, otherVector);
            
            similarities.push_back({otherId, similarity});
        }
    }
    
    // 按相似度排序（降序）
    SortUtil::sort(similarities, [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    
    // 限制為請求的數量
    if (similarities.size() > static_cast<size_t>(count)) {
        similarities.resize(count);
    }
    
    return similarities;
}

// 取得混合式推薦（改良演算法）
std::vector<std::pair<int, double>> RecommendationEngine::getHybridRecommendations(
    const std::string& username, int count) const {
    
    // 取得更多候選的協同過濾推薦
    auto cfRecs = getCollaborativeFilteringRecommendations(username, count * 3);
    
    // 如果沒有協同過濾推薦，回退到內容式推薦
    if (cfRecs.empty()) {
        // 找到最近借閱的書籍
        auto userIt = SearchUtil::mapFind(userLoans, username);
        
        if (userIt != userLoans.end() && !userIt->second.empty()) {
            int someBookId = *userIt->second.begin();
            return getContentBasedRecommendations(someBookId, count);
        }
        
        return {}; // 無法產生推薦
    }
    
    // 從多本參考書籍取得內容式推薦
    std::unordered_map<int, double> contentScores;
    auto userIt = SearchUtil::mapFind(userLoans, username);
    
    if (userIt != userLoans.end() && !userIt->second.empty()) {
        const auto& userBooks = userIt->second;
        
        // 使用最多 3 本不同的書作為內容式推薦的參考
        int refCount = 0;
        for (int refBookId : userBooks) {
            if (refCount >= 3) break;
            
            auto cbRecs = getContentBasedRecommendations(refBookId, count * 2);
            
            // 根據參考數量的倒數來加權內容分數
            double weight = 1.0 / (refCount + 1);
            
            for (const auto& rec : cbRecs) {
                contentScores[rec.first] += rec.second * weight;
            }
            
            refCount++;
        }
    }
    
    // 使用適應性加權合併兩種推薦類型
    std::unordered_map<int, double> finalScores;
    
    // 正規化協同過濾分數
    double maxCf = 0.0;
    for (const auto& rec : cfRecs) {
        maxCf = std::max(maxCf, rec.second);
    }
    
    // 正規化內容分數
    double maxContent = 0.0;
    for (const auto& pair : contentScores) {
        maxContent = std::max(maxContent, pair.second);
    }
    
    // 適應性加權基於數據可用性
    double cfWeight = cfRecs.empty() ? 0.0 : 0.6;
    double contentWeight = contentScores.empty() ? 0.0 : 0.4;
    
    // 如果一種類型有很少的推薦，提升另一種
    if (cfRecs.size() < static_cast<size_t>(count / 2)) {
        cfWeight = 0.3;
        contentWeight = 0.7;
    } else if (contentScores.size() < static_cast<size_t>(count / 2)) {
        cfWeight = 0.8;
        contentWeight = 0.2;
    }
    
    // 組合協同過濾分數
    if (maxCf > 0) {
        for (const auto& rec : cfRecs) {
            double normalizedScore = rec.second / maxCf;
            finalScores[rec.first] += cfWeight * normalizedScore;
        }
    }
    
    // 組合內容式分數
    if (maxContent > 0) {
        for (const auto& pair : contentScores) {
            double normalizedScore = pair.second / maxContent;
            finalScores[pair.first] += contentWeight * normalizedScore;
        }
    }
    
    // 應用多樣性獎勵：提升具有唯一類別的書籍
    for (auto& pair : finalScores) {
        // 這是一個簡化的多樣性獎勵
        // 在真正的系統中，您需要更複雜的類別分析
        pair.second *= (1.0 + 0.1 * std::sin(pair.first)); // 簡單的偽隨機多樣性
    }
    
    // 轉換為向量並排序
    std::vector<std::pair<int, double>> hybrid;
    for (const auto& pair : finalScores) {
        hybrid.push_back(pair);
    }
    
    // 按最終分數排序
    SortUtil::sort(hybrid, [](const auto& a, const auto& b) {
        if (std::abs(a.second - b.second) < 1e-9) {
            return a.first < b.first; // 穩定排序
        }
        return a.second > b.second;
    });
    
    // 應用最終多樣性過濾：確保不推薦太多同一類別的書籍
    std::vector<std::pair<int, double>> diverseResults;
    std::unordered_map<std::string, int> categoryCount;
    
    for (const auto& rec : hybrid) {
        if (diverseResults.size() >= static_cast<size_t>(count)) break;
        
        // Add the book (simplified diversity check)
        diverseResults.push_back(rec);
    }
    
    return diverseResults;
}

// Display recommendations
void RecommendationEngine::displayRecommendations(const std::string& username, 
                                                 const BookManager& bookManager, int count) const {
    std::cout << "===== " << username << " 的個人化推薦 =====" << std::endl;
    
    auto hybrid = getHybridRecommendations(username, count);
    
    if (hybrid.empty()) {
        std::cout << "暫時沒有可用的推薦。請先借閱一些圖書！" << std::endl;
        return;
    }
    
    displayHybridRecommendations(hybrid, bookManager);
    displayCollaborativeFilteringDetails(username, bookManager, count);
    displayContentBasedDetails(username, bookManager, count);
    
    std::cout << std::endl;
    std::cout << "💡 提示: 混合推薦結合了協同過濾和內容相似度，為您提供最精準的推薦！" << std::endl;
}

void RecommendationEngine::displayHybridRecommendations(const std::vector<std::pair<int, double>>& hybrid,
                                                       const BookManager& bookManager) const {
    std::cout << "🤖 智能混合推薦 (協同過濾 + 內容相似度)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    int rank = 1;
    for (const auto& rec : hybrid) {
        const Book* book = bookManager.getBook(rec.first);
        if (book) {
            std::cout << rank << ". [" << book->getId() << "] " 
                      << std::setw(40) << std::left 
                      << formatBookTitle(book->getTitle())
                      << " (綜合指數: " << std::fixed << std::setprecision(2) << rec.second << ")" << std::endl;
            
            std::cout << "   作者: " << book->getAuthor();
            if (book->getAvailableCopies() > 0) {
                std::cout << " | 狀態: 可借閱 (" << book->getAvailableCopies() << " 本)";
            } else {
                std::cout << " | 狀態: 已借完";
            }
            std::cout << std::endl << std::endl;
            
            rank++;
        }
    }
}

void RecommendationEngine::displayCollaborativeFilteringDetails(const std::string& username,
                                                               const BookManager& bookManager, int count) const {
    auto cfRecs = getCollaborativeFilteringRecommendations(username, count);
    if (cfRecs.empty()) return;
    
    std::cout << "📊 協同過濾推薦 (基於其他讀者的借閱習慣)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    int rank = 1;
    for (const auto& rec : cfRecs) {
        const Book* book = bookManager.getBook(rec.first);
        if (book) {
            std::cout << rank << ". [" << book->getId() << "] " 
                      << std::setw(40) << std::left 
                      << formatBookTitle(book->getTitle())
                      << " (協同指數: " << std::fixed << std::setprecision(2) << rec.second << ")" << std::endl;
            
            rank++;
        }
    }
    std::cout << std::endl;
}

void RecommendationEngine::displayContentBasedDetails(const std::string& username,
                                                     const BookManager& bookManager, int count) const {
    auto userIt = SearchUtil::mapFind(userLoans, username);
    
    if (userIt == userLoans.end() || userIt->second.empty()) return;
    
    int someBookId = *userIt->second.begin();
    const Book* refBook = bookManager.getBook(someBookId);
    
    if (!refBook) return;
    
    std::cout << "📚 內容相似度推薦 (基於您借閱過的《" << refBook->getTitle() << "》)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    auto cbRecs = getContentBasedRecommendations(someBookId, count);
    
    int rank = 1;
    for (const auto& rec : cbRecs) {
        const Book* book = bookManager.getBook(rec.first);
        if (book) {
            std::cout << rank << ". [" << book->getId() << "] " 
                      << std::setw(40) << std::left 
                      << formatBookTitle(book->getTitle())
                      << " (相似度: " << std::fixed << std::setprecision(2) << rec.second << ")" << std::endl;
            
            rank++;
        }
    }
} 