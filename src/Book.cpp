#include "../include/Book.h"
#include "../include/SearchUtil.h"
#include <iostream>
#include <iomanip>

Book::Book() : id(0), year(0), availableCopies(0), totalCopies(0), pageCount(0) {}

Book::Book(int id, const std::string& title, const std::string& author, int year,
    int copies, const std::string& isbn, const std::string& publisher,
    const std::string& language, int pageCount, const std::string& synopsis)
    : id(id), title(title), author(author), year(year),
    availableCopies(copies), totalCopies(copies), isbn(isbn),
    publisher(publisher), language(language), pageCount(pageCount),
    synopsis(synopsis) {
}

// Getters
int Book::getId() const { return id; }
std::string Book::getTitle() const { return title; }
std::string Book::getAuthor() const { return author; }
int Book::getYear() const { return year; }
int Book::getAvailableCopies() const { return availableCopies; }
int Book::getTotalCopies() const { return totalCopies; }
std::string Book::getIsbn() const { return isbn; }
std::string Book::getPublisher() const { return publisher; }
std::string Book::getLanguage() const { return language; }
int Book::getPageCount() const { return pageCount; }
std::string Book::getSynopsis() const { return synopsis; }
const std::vector<std::string>& Book::getCategories() const { return categories; }

// Setters
void Book::setId(int id) { this->id = id; }
void Book::setTitle(const std::string& title) { this->title = title; }
void Book::setAuthor(const std::string& author) { this->author = author; }
void Book::setYear(int year) { this->year = year; }
void Book::setAvailableCopies(int copies) { this->availableCopies = copies; }
void Book::setTotalCopies(int copies) { this->totalCopies = copies; }
void Book::setIsbn(const std::string& isbn) { this->isbn = isbn; }
void Book::setPublisher(const std::string& publisher) { this->publisher = publisher; }
void Book::setLanguage(const std::string& language) { this->language = language; }
void Book::setPageCount(int pageCount) { this->pageCount = pageCount; }
void Book::setSynopsis(const std::string& synopsis) { this->synopsis = synopsis; }

void Book::addCategory(const std::string& category) {
    // Check if category already exists
    for (const auto& cat : categories) {
        if (cat == category) return;
    }
    categories.push_back(category);
}

void Book::removeCategory(const std::string& category) {
    auto it = categories.begin();
    while (it != categories.end()) {
        if (*it == category) {
            it = categories.erase(it);
        }
        else {
            ++it;
        }
    }
}

// Book operations
bool Book::borrow() {
    if (availableCopies > 0) {
        --availableCopies;
        return true;
    }
    return false;
}

bool Book::returnBook() {
    if (availableCopies < totalCopies) {
        ++availableCopies;
        return true;
    }
    return false;
}

// Display
void Book::display() const {
    std::cout << "==============================\n";
    std::cout << "Book ID: " << id << "\n";
    std::cout << "Title: " << title << "\n";
    std::cout << "Author: " << author << "\n";
    std::cout << "Year: " << year << "\n";
    std::cout << "ISBN: " << isbn << "\n";
    std::cout << "Publisher: " << publisher << "\n";
    std::cout << "Language: " << language << "\n";
    std::cout << "Page Count: " << pageCount << "\n";
    std::cout << "Availability: " << availableCopies << "/" << totalCopies << "\n";

    std::cout << "Categories: ";
    for (size_t i = 0; i < categories.size(); ++i) {
        std::cout << categories[i];
        if (i < categories.size() - 1) std::cout << ", ";
    }
    std::cout << "\n";

    std::cout << "Synopsis: " << synopsis << "\n";
    std::cout << "==============================\n";
}

void Book::displaySummary() const {
    std::cout << std::left << "[" << std::setw(4) << id << "] "
        << std::setw(30) << (title.length() > 28 ? title.substr(0, 28) + "..." : title)
        << " (Author: " << std::setw(20) << (author.length() > 18 ? author.substr(0, 18) + "..." : author)
        << ", Year: " << std::setw(4) << year
        << ", Available: " << availableCopies << "/" << totalCopies << ")\n";
}

// Search/filter functions
bool Book::matchesKeyword(const std::string& keyword) const {
    if (keyword.empty()) {
        return false;
    }

    if (SearchUtil::contains(title, keyword)) {
        return true;
    }

    if (SearchUtil::contains(author, keyword)) {
        return true;
    }

    if (SearchUtil::contains(synopsis, keyword)) {
        return true;
    }

    for (const auto& category : categories) {
        if (SearchUtil::contains(category, keyword)) {
            return true;
        }
    }

    if (SearchUtil::contains(publisher, keyword)) {
        return true;
    }

    if (SearchUtil::contains(isbn, keyword)) {
        return true;
    }

    return false;
}

bool Book::matchesYear(int y, const std::string& op) const {
    if (op == "=") {
        return year == y;
    }
    else if (op == ">") {
        return year > y;
    }
    else if (op == "<") {
        return year < y;
    }
    else if (op == ">=") {
        return year >= y;
    }
    else if (op == "<=") {
        return year <= y;
    }
    else {
        return false;
    }
}

bool Book::matchesCategory(const std::string& category) const {
    for (const auto& cat : categories) {
        if (cat == category) return true;
    }
    return false;
}
