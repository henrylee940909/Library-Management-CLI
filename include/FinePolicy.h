#ifndef FINE_POLICY_H
#define FINE_POLICY_H

class FinePolicy {
private:
    int graceDays;    // Days before fines start
    double fixedRate;    // Fixed fine amount per day
    double incrementalFactor; // Incremental factor for increasing fines

public:
    FinePolicy();
    FinePolicy(int graceDays, double fixedRate, double incrementalFactor);
    
    // Getters
    int getGraceDays() const;
    double getFixedRate() const;
    double getIncrementalFactor() const;
    
    // Setters
    void setGraceDays(int days);
    void setFixedRate(double rate);
    void setIncrementalFactor(double factor);
    
    // Calculate fine amount
    double calculateFine(int overdueDays) const;
    
    // Display policy
    void display() const;
};

#endif // FINE_POLICY_H 