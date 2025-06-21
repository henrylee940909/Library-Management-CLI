#include <iostream>
#include "../include/Library.h"

int main() {
    try {
        Library library("data/books.json", "data/users.json", 
                       "data/loans.json");
        
        if (library.initialize()) {
            library.run();
        }
    } catch (const std::exception& e) {
        std::cerr << "錯誤: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 