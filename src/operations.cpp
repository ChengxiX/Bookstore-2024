#include "models.hpp"
#include <utility>

bool User::check_id(const std::string &id) {
    if (id == "guest") return false;
    return check_str(id);
}

bool User::check_str(const std::string &id) {
    if (id.size() > max_str_len - 1) return false;
    for (char c : id) {
        if ((!std::isalnum(c)) && c != '_') return false;
    }
    return true;
}

bool User::check_name(const std::string &id) {
    if (id.size() > max_str_len - 1) return false;
    for (char c : id) {
        if (c < 32 || c >= 127) return false; // 空格是可见的
    }
    return true;
}

bool User::check_pri(int privilege) {
    switch (privilege) {
        case 0: return true;
        case 1: return true;
        case 3: return true;
        case 7: return true;
        default: return false;
    }
}

bool User::su(const std::string &id , const std::string &password, int privilege) {
    if (!check_id(id)) return false;
    if (!check_name(password)) return false;
    if (!check_pri(privilege)) return false;
    char id_arr[max_str_len];
    std::strcpy(id_arr, id.c_str());
    auto res = db.get(id_arr);
    if (! res.first) {return false;}
    if (privilege > res.second.second.Privilege) {return true;}
    if (std::strcmp(res.second.second.Password, password.c_str()) == 0) {return true;}
    return false;
}

bool User::useradd(const std::string &id, const std::string &password, const std::string &username, int privilege, int current_pri, const std::string &staff) {
    if (!check_id(id)) return false;
    if (!check_str(password)) return false;
    if (!check_name(username)) return false;
    if (!check_pri(privilege)) return false;
    if (current_pri != 2) {
        if (!check_pri(current_pri)) return false;
    }
    if (privilege >= current_pri) return false;
    std::pair<char[max_str_len], UserInfo> p;
    std::strcpy(p.first, id.c_str());
    std::strcpy(p.second.id, id.c_str());
    std::strcpy(p.second.Password, password.c_str());
    std::strcpy(p.second.Username, username.c_str());
    p.second.Privilege = privilege;
    try {
        db.insert(p);
        return true;
    }
    catch (RandomDB<User::UserId, textcmp, UserInfo>::DuplicateException &e) {
        return false;
    }
}

bool User::passwd(const std::string &id, const std::string &new_password, const std::string &old_password, const std::string & staff) {

}

bool User::Delete(const std::string &id) {

}

std::vector<Book::BookInfo> Book::show_isbn(const std::string &isbn) {

}

std::vector<Book::BookInfo> Book::show_title(const std::string &title) {

}

std::vector<Book::BookInfo> Book::show_author(const std::string &author) {

}   

std::vector<Book::BookInfo> Book::show_keyword(const std::string &keyword) {

}

int Book::select(const std::string &isbn) {

}

bool Book::modify(int book_id, const std::string &isbn, const std::string &title, const std::string &author, const std::string &keyword, Price_T price) {

}

Book::Price_T Deal::buy(const std::string &isbn, int quantity) {

}

Book::Price_T Deal::import(int book_id, int quantity, Book::Price_T total_cost) {

}

std::string Deal::show_finance(int count) {
    
}
