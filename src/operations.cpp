#include "models.hpp"

bool User::su(std::string id , std::string password, int privilege) {

}

bool User::logout() {

}

bool User::useradd(std::string id, std::string password, std::string username, int privilege) {

}

bool User::passwd(std::string id, std::string old_password, std::string new_password) {

}

bool User::Delete(std::string id) {

}

std::vector<Book::BookInfo> Book::show_isbn(std::string isbn) {

}

std::vector<Book::BookInfo> Book::show_title(std::string title) {

}

std::vector<Book::BookInfo> Book::show_author(std::string author) {

}   

std::vector<Book::BookInfo> Book::show_keyword(std::string keyword) {

}

int Book::select(std::string isbn) {

}

bool Book::modify(int book_id, std::string isbn, std::string title, std::string author, std::string keyword, Price_T price) {

}

Book::Price_T Deal::buy(std::string isbn, int quantity) {

}

Book::Price_T Deal::import(int book_id, int quantity, Book::Price_T total_cost) {

}

std::string Deal::show_finance(int count) {
    
}
