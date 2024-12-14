#include "operations.cpp"
#include "models.hpp"
#include <ostream>
#include <stack>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

extern const int INSTANCE_ID;

bool check_rest(std::stringstream &ss) {
    std::string rest;
    ss >> rest;
    if (rest != "") {
        std::cout << "Invalid" << std::endl;
        return true; //注意！：写反了
    }
    return false;
}

int check_price(const std::string &price) {
    if (price == "") return false;
    std::istringstream iss(price);
    double p;
    char c;
    if (!(iss >> p) || iss.get(c)) {
        return -1;
    }
    std::string::size_type dot_pos = price.find('.');
    if (dot_pos == std::string::npos) {
        return 0;
    }
    if (price.size() - dot_pos - 1 == 2) {
        return 2;
    }
    if (price.size() - dot_pos - 1 == 1) {
        return 1;
    }
    return -1;
}

int main() {
    // freopen("data.in", "r", stdin);
    if (User::db.init("user", INSTANCE_ID, "", false)) {
        User::useradd("root", "sjtu", "admin", 7, 8, "root");
    }
    std::string cmd;
    std::stack<std::string> c_users;
    c_users.push("guest");
    std::stack<int> c_selected_books;
    c_selected_books.push(-1);
    std::stack<int> c_privileges;
    c_privileges.push(0);
    while (std::getline(std::cin, cmd)) {
        std::stringstream ss(cmd);
        std::string op;
        ss >> op;
        if (op == "exit" || op == "quit") {
            if (check_rest(ss)) { continue;}
            break;
        }
        else if (op == "su") {
            std::string id, password;
            ss >> id >> password;
            if (check_rest(ss)) { continue;}
            int res = User::su(id, password, c_privileges.top());
            if (res != -1) {
                c_users.push(id);
                c_privileges.push(res);
                c_selected_books.push(-1);
            }
            else {
                std::cout << "Invalid" << std::endl;
                continue;
            }
        }
        else if (op == "logout") {
            if (check_rest(ss)) { continue;}
            if (c_users.size() > 1) {
                Log::logout(c_users.top());
                c_users.pop();
                c_privileges.pop();
                c_selected_books.pop();
            }
            else {
                std::cout << "Invalid" << std::endl;
                continue;
            }
        }
        else if (op == "passwd") {
            std::string id, old_password, new_password;
            ss >> id >> old_password >> new_password;
            if (check_rest(ss)) { continue;}
            if (new_password=="") {
                new_password = old_password;
                old_password = "";
            }
            if (!User::passwd(id, new_password, c_users.top(), c_privileges.top(), old_password)) {
                std::cout << "Invalid" << std::endl;
                continue;
            }
        }
        else if (op == "useradd") {
            std::string id, password, username;
            int pri;
            ss >> id >> password >> pri >> username;
            if (check_rest(ss)) { continue;}
            if (!User::useradd(id, password, username, pri, c_privileges.top(), c_users.top())) {
                std::cout << "Invalid" << std::endl;
                continue;
            }
        }
        else if (op == "register") {
            std::string id, password, username;
            ss >> id >> password >> username;
            if (check_rest(ss)) { continue;}
            if (!User::Register(id, password, username)) {
                std::cout << "Invalid" << std::endl;
                continue;
            }
        }
        else if (op == "delete") {
            std::string id;
            ss >> id;
            if (check_rest(ss)) { continue;}
            if (!User::Delete(id, c_users.top(), c_privileges.top())) {
                std::cout << "Invalid" << std::endl;
                continue;
            }
        }
        else if (op == "show") {
            std::string type;
            ss >> type;
            if (type == "") {
                auto all = Book::db.range(0, Book::db.size());
                std::sort(all.begin(), all.end(), [](const Book::BookInfo &a, const Book::BookInfo &b) {
                    return std::strcmp(a.ISBN.c_str(), b.ISBN.c_str()) < 0;
                });
                for (auto &book : all) {
                    std::cout << book.ISBN << "\t" << book.Title << "\t" << book.Author << "\t" << book.Keyword << "\t";
                    std::cout << std::fixed << std::setprecision(2) << double(book.price) / 100 << "\t" << book.Stock << std::endl;
                }
                if (all.empty()) {
                    std::cout << std::endl;
                }
                continue;
            }
            else if (type == "finance") {
                std::string count;
                ss >> count;
                if (check_rest(ss)) { continue;}
                int c;
                if (count == "") {
                    c = -1;
                }
                else {
                    if (count.find_first_not_of("0123456789") != std::string::npos) {
                        std::cout << "Invalid" << std::endl;
                        continue;
                    }
                    c = std::stoi(count);
                }
                if (c == 0) {
                    std::cout << std::endl; // 空行
                    continue;
                }
                auto res = Deal::show_finance(c, c_users.top(), c_privileges.top());
                if (res.first == -1) {
                    std::cout << "Invalid" << std::endl;
                    continue;
                }
                std::cout << "+ " << std::fixed << std::setprecision(2) << double(res.first) / 100 << " - " << double(res.second) / 100 << std::endl;
                continue;
            }
            if (type[0] != '-') {
                std::cout << "Invalid" << std::endl;
                continue;
            }
            int eq = type.find('=');
            if (eq == std::string::npos) {
                std::cout << "Invalid" << std::endl;
                continue;
            }
            if (type.substr(0, eq) == "-ISBN") {
                std::string isbn = type.substr(eq + 1);
                if (check_rest(ss)) { continue;}
                auto res = Book::show_isbn(isbn, c_privileges.top(), c_users.top());
                if (!res.first) {
                    std::cout << "Invalid" << std::endl;
                    continue;
                }
                for (auto &book : res.second) {
                    std::cout << book.ISBN << "\t" << book.Title << "\t" << book.Author << "\t" << book.Keyword << "\t";
                    std::cout << std::fixed << std::setprecision(2) << double(book.price) / 100 << "\t" << book.Stock << std::endl;
                }
                if (res.second.empty()) {
                    std::cout << std::endl;
                }
            }
            else if (type.substr(0, eq) == "-name") {
                std::string title;
                if (type[eq + 1] != '\"') {
                    std::cout << "Invalid" << std::endl;
                    continue;
                }
                if (type.back() != '\"') {
                    title = type.substr(eq + 2);
                    while (ss >> type) {
                        if (type == "") {
                            goto invalid2;
                        }
                        if (type.back() == '\"') {
                            title += " " + type.substr(0, type.size() - 1);
                            break;
                        }
                        title += " " + type;
                    }
                }
                else {
                    title = type.substr(eq + 2, type.size() - eq - 3);
                }
                if (check_rest(ss)) { continue;}
                auto res = Book::show_title(title, c_privileges.top(), c_users.top());
                if (!res.first) {
                    std::cout << "Invalid" << std::endl;
                    continue;
                }
                std::sort(res.second.begin(), res.second.end(), [](const Book::BookInfo &a, const Book::BookInfo &b) {
                    return std::strcmp(a.ISBN.c_str(), b.ISBN.c_str()) < 0;
                });
                for (auto &book : res.second) {
                    std::cout << book.ISBN << "\t" << book.Title << "\t" << book.Author << "\t" << book.Keyword << "\t";
                    std::cout << std::fixed << std::setprecision(2) << double(book.price) / 100 << "\t" << book.Stock << std::endl;
                }
                if (res.second.empty()) {
                    std::cout << std::endl;
                }
            }
            else if (type.substr(0, eq) == "-author") {
                if (type[eq + 1] != '\"') {
                    std::cout << "Invalid" << std::endl;
                    continue;
                }
                std::string author;
                if (type.back() != '\"') {
                    author = type.substr(eq + 2);
                    while (ss >> type) {
                        if (type == "") {
                            goto invalid2;
                        }
                        if (type.back() == '\"') {
                            author += " " + type.substr(0, type.size() - 1);
                            break;
                        }
                        author += " " + type;
                    }
                }
                else {
                    author = type.substr(eq + 2, type.size() - eq - 3);
                }
                if (check_rest(ss)) { continue;}
                auto res = Book::show_author(author, c_privileges.top(), c_users.top());
                if (!res.first) {
                    std::cout << "Invalid" << std::endl;
                    continue;
                }
                std::sort(res.second.begin(), res.second.end(), [](const Book::BookInfo &a, const Book::BookInfo &b) {
                    return std::strcmp(a.ISBN.c_str(), b.ISBN.c_str()) < 0;
                });
                for (auto &book : res.second) {
                    std::cout << book.ISBN << "\t" << book.Title << "\t" << book.Author << "\t" << book.Keyword << "\t";
                    std::cout << std::fixed << std::setprecision(2) << double(book.price) / 100 << "\t" << book.Stock << std::endl;
                }
                if (res.second.empty()) {
                    std::cout << std::endl;
                }
            }
            else if (type.substr(0, eq) == "-keyword") {
                if (check_rest(ss)) { continue;}
                if (type[eq + 1] != '\"' || type.back() != '\"') {
                    std::cout << "Invalid" << std::endl;
                    continue;
                }
                std::string keyword = type.substr(eq + 2, type.size() - eq - 3);
                auto res = Book::show_keyword(keyword, c_privileges.top(), c_users.top());
                if (!res.first) {
                    std::cout << "Invalid" << std::endl;
                    continue;
                }
                std::sort(res.second.begin(), res.second.end(), [](const Book::BookInfo &a, const Book::BookInfo &b) {
                    return std::strcmp(a.ISBN.c_str(), b.ISBN.c_str()) < 0;
                });
                for (auto &book : res.second) {
                    std::cout << book.ISBN << "\t" << book.Title << "\t" << book.Author << "\t" << book.Keyword << "\t";
                    std::cout << std::fixed << std::setprecision(2) << double(book.price) / 100 << "\t" << book.Stock << std::endl;
                }
                if (res.second.empty()) {
                    std::cout << std::endl;
                }
            }
            else {
                invalid2 :;
                std::cout << "Invalid" << std::endl;
                continue;
            }
        }
        else if (op == "select") {
            std::string isbn;
            ss >> isbn;
            if (check_rest(ss)) { continue;}
            int res = Book::select(isbn, c_privileges.top(), c_users.top());
            if (res != -1) {
                c_selected_books.top() = res;
            }
            else {
                std::cout << "Invalid" << std::endl;
                continue;
            }
        }
        else if (op == "modify") {
            std::string arg;
            std::string isbn, title, author, keyword;
            Book::Price_T price = -1;
            if (c_selected_books.top() == -1) {
                std::cout << "Invalid" << std::endl;
                continue;
            }
            while (ss >> arg) {
                if (arg[0] != '-') {
                    goto invalid;
                }
                int eq = arg.find('=');
                if (eq == std::string::npos) {
                    goto invalid;
                }
                if (arg.substr(0, eq) == "-ISBN") {
                    if (isbn != "") {
                        goto invalid;
                    }
                    isbn = arg.substr(eq + 1);
                }
                else if (arg.substr(0, eq) == "-name") {
                    if (title != "") {
                        goto invalid;
                    }
                    if (arg[eq + 1] != '\"') {
                        goto invalid;
                    }
                    if (arg.back() != '\"') {
                        title = arg.substr(eq + 2);
                        while (ss >> arg) {
                            if (arg == "") {
                                goto invalid;
                            }
                            if (arg.back() == '\"') {
                                title += " " + arg.substr(0, arg.size() - 1);
                                break;
                            }
                            title += " " + arg;
                        }
                    }
                    else {
                        title = arg.substr(eq + 2, arg.size() - eq - 3);
                    }
                }
                else if (arg.substr(0, eq) == "-author") {
                    if (author != "") {
                        goto invalid;
                    }
                    if (arg[eq + 1] != '\"') {
                        goto invalid;
                    }
                    if (arg.back() != '\"') {
                        author = arg.substr(eq + 2);
                        while (ss >> arg) {
                            if (arg == "") {
                                goto invalid;
                            }
                            if (arg.back() == '\"') {
                                author += " " + arg.substr(0, arg.size() - 1);
                                break;
                            }
                            author += " " + arg;
                        }
                    }
                    else {
                        author = arg.substr(eq + 2, arg.size() - eq - 3);
                    }
                }
                else if (arg.substr(0, eq) == "-keyword") {
                    if (keyword != "") {
                        goto invalid;
                    }
                    if (arg[eq + 1] != '\"' || arg.back() != '\"') {
                        goto invalid;
                    }
                    else {
                        keyword = arg.substr(eq + 2, arg.size() - eq - 3);
                    }
                }
                else if (arg.substr(0, eq) == "-price") {
                    if (price != -1) {
                        goto invalid;
                    }
                    std::string p = arg.substr(eq + 1);
                    int dot = check_price(p);
                    if (dot == -1) {
                        goto invalid;
                    }
                    std::string inte;
                    for (char c : p) {
                        if (c == '.') {
                            continue;
                        }
                        inte += c;
                    }
                    price = std::stoll(inte);
                    if (dot == 1) {
                        price *= 10;
                    }
                    else if (dot == 0) {
                        price *= 100;
                    }
                }
                else {
                    goto invalid;
                }
            }
            if (isbn == "" && title == "" && author == "" && keyword == "" && price == -1) {
                invalid :;
                std::cout << "Invalid" << std::endl;
                continue;
            }
            if (!Book::modify(c_users.top(), c_selected_books.top(), c_privileges.top(), isbn, title, author, keyword, price)) {
                std::cout << "Invalid" << std::endl;
                continue;
            }
        }
        else if (op == "buy") {
            std::string isbn;
            std::string quantity;
            ss >> isbn >> quantity;
            if (check_rest(ss)) { continue;}
            if (quantity.find_first_not_of("0123456789") != std::string::npos) {
                std::cout << "Invalid" << std::endl;
                continue;
            }
            int q = std::stoi(quantity);
            if (q <= 0) {
                std::cout << "Invalid" << std::endl;
                continue;
            }
            Book::Price_T res = Deal::buy(c_users.top(), isbn, q, c_privileges.top());
            if (res == -1) {
                std::cout << "Invalid" << std::endl;
                continue;
            }
            else {
                std::cout << std::fixed << std::setprecision(2) << double(res) / 100 << std::endl;
            }
        }
        else if (op == "import") {
            std::string quantity, total_cost;
            ss >> quantity >> total_cost;
            if (check_rest(ss)) { continue;}
            if (quantity.find_first_not_of("0123456789") != std::string::npos) {
                std::cout << "Invalid" << std::endl;
                continue;
            }
            int dot = check_price(total_cost);
            if (dot == -1) {
                std::cout << "Invalid" << std::endl;
                continue;
            }
            if (std::stoi(quantity) <= 0) {
                std::cout << "Invalid" << std::endl;
                continue;
            }
            std::string inte;
            for (char c : total_cost) {
                if (c == '.') {
                    continue;
                }
                inte += c;
            }
            Book::Price_T price = std::stoll(inte);
            if (dot == 1) {
                price *= 10;
            }
            else if (dot == 0) {
                price *= 100;
            }
            Book::Price_T res = Deal::import(c_users.top(), c_selected_books.top(), std::stoi(quantity), price, c_privileges.top());
            if (res == -1) {
                std::cout << "Invalid" << std::endl;
                continue;
            }
        }
        else if (op == "report") {

        }
        else if (op == "log") {

        }
        else if (op == "") {}
        else {
            std::cout << "Invalid" << std::endl;
            continue;
        }
    }
}