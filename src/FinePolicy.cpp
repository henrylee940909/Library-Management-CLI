#include "../include/FinePolicy.h"
#include <iostream>
#include <iomanip>
#include <cmath>

FinePolicy::FinePolicy() : graceDays(0), fixedRate(0), incrementalFactor(1.0) {}

FinePolicy::FinePolicy(int graceDays, double fixedRate, double incrementalFactor)
    : graceDays(graceDays), fixedRate(fixedRate), incrementalFactor(incrementalFactor) {}

// Getters
int FinePolicy::getGraceDays() const { return graceDays; }
double FinePolicy::getFixedRate() const { return fixedRate; }
double FinePolicy::getIncrementalFactor() const { return incrementalFactor; }

// Setters
void FinePolicy::setGraceDays(int days) { graceDays = days; }
void FinePolicy::setFixedRate(double rate) { fixedRate = rate; }
void FinePolicy::setIncrementalFactor(double factor) { incrementalFactor = factor; }

// Calculate fine amount
double FinePolicy::calculateFine(int overdueDays) const {
    if (overdueDays <= graceDays) {
        return 0.0; // Within grace period
    }
    
    double fine = 0.0;
    int daysToCharge = overdueDays - graceDays;
    
    if (incrementalFactor <= 1.0) {
        // Fixed rate fine
        fine = daysToCharge * fixedRate;
    } else {
        // Incremental fine
        for (int i = 0; i < daysToCharge; ++i) {
            fine += fixedRate * std::pow(incrementalFactor, i);
        }
    }
    
    return fine;
}

// Display policy
void FinePolicy::display() const {
    std::cout << "===== 罰款政策 =====" << std::endl;
    std::cout << "寬限期: " << graceDays << " 天" << std::endl;
    std::cout << "基本費率: $" << std::fixed << std::setprecision(0) << fixedRate << " 每天" << std::endl;
    
    if (incrementalFactor > 1.0) {
        std::cout << "遞增因子: " << incrementalFactor << " (每日遞增)" << std::endl;
    } else {
        std::cout << "費率類型: 固定費率" << std::endl;
    }
    
    std::cout << "範例: 逾期 10 天罰款 $" 
              << std::fixed << std::setprecision(0) 
              << calculateFine(10) << std::endl;
    std::cout << "====================" << std::endl;
} 