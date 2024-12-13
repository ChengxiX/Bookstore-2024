#include "models.hpp"
#include <string>
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
    auto res = db.get(binstring<max_str_len>(id));
    if (! res.first) {return false;}
    if (privilege > res.second.second.Privilege) {
        Log::login(id);
        return true;
    }
    if (std::strcmp(res.second.second.Password.c_str(), password.c_str()) == 0) {
        Log::login(id);
        return true;
    }
    return false;
}

bool User::useradd(const std::string &id, const std::string &password, const std::string &username, int privilege, int current_pri, const std::string &staff) {
    if (!check_id(id)) return false;
    if (!check_str(password)) return false;
    if (!check_name(username)) return false;
    if (!check_pri(privilege)) return false;
    if (current_pri != 2) {
        if (!check_pri(current_pri)) return false;
        if (current_pri < 3) return false;
    }
    if (privilege >= current_pri) return false;
    std::pair<binstring<max_str_len>, UserInfo> p(id, UserInfo{id, binstring<max_str_len>(username),
    binstring<max_str_len>(password), privilege});
    try {
        db.insert(p);
        Log::useradd(staff, "add user " + id);
        return true;
    }
    catch (RandomDB<User::UserId, textcmp, UserInfo>::DuplicateException &e) {
        return false;
    }
}

bool User::passwd(const std::string &id, const std::string &new_password, const std::string & staff, int privilege, const std::string &old_password) {
    if (!check_id(id)) return false;
    if (!check_str(new_password)) return false;
    if (!check_pri(privilege)) return false;
    if (privilege < 1) return false;
    if (privilege == 7) {
        auto res = db.get(binstring<max_str_len>(id));
        if (! res.first) {return false;}
        res.second.second.Password = binstring<max_str_len>(new_password);
        db.erase(res.second.first);
        db.insert(res.second);
        Log::userpw(staff, "change password of user " + id);
        return true;
    }
    else {
        if (!check_str(old_password)) return false;
        auto res = db.get(binstring<max_str_len>(id));
        if (! res.first) {return false;}
        if (std::strcmp(res.second.second.Password.c_str(), old_password.c_str()) != 0) return false;
        res.second.second.Password = binstring<max_str_len>(new_password);
        db.erase(res.second.first);
        db.insert(res.second);
        Log::userpw(staff, "change password of user " + id);
        return true;
    }
}

bool User::Delete(const std::string &id, const std::string &staff, int privilege) {
    if (!check_id(id)) return false;
    if (privilege != 7) return false;
    auto res = db.get(binstring<max_str_len>(id));
    if (! res.first) {return false;}
    db.erase(res.second.first);
    Log::userdelete(staff, "delete user " + id);
    return true;
}

bool Book::check_isbn(const std::string &isbn) {
    if (isbn.size() != 13) return false;
    for (char c : isbn) {
        if (c < 32 || c >= 127) return false;
    }
    return true;
}

bool Book::check_str(const std::string &str) {
    if (str.size() > max_str_len - 1) return false;
    for (char c : str) {
        if (c < 32 || c >= 127 || c == '\"') return false;
    }
    return true;
}

bool Book::Keywords::check_single_kw(const std::string &str) {
    if (str.size() > max_str_len - 1) return false;
    if (str.size() == 0) return false;
    for (char c : str) {
        if (c < 32 || c >= 127 || c == '\"' || c == '|' || c == ' ') return false;
    }
    return true;
}

std::vector<Book::BookInfo> Book::show_isbn(const std::string &isbn, int privilege, const std::string &staff) {
    if (!User::check_pri(privilege)) return {};
    if (check_isbn(isbn)) return {};
    if (privilege < 1) return {};
    auto res = isbn2id.get(binstring<max_isbn_len>(isbn));
    std::vector<BookInfo> ret;
    if (res.first) {
        ret.push_back(db[res.second.second]);
    }
    Log::showbook(staff, "show book with ISBN " + isbn, res.second.second);
    return ret;
}

std::vector<Book::BookInfo> Book::show_title(const std::string &title, int privilege, const std::string &staff) {
    if (!User::check_pri(privilege)) return {};
    if (!check_str(title)) return {};
    if (privilege < 1) return {};
    auto res = title2id.find(binstring<max_str_len>(title));
    std::vector<BookInfo> ret;
    for (auto &p : res) {
        ret.push_back(db[p.second]);
    }
    Log::showbook(staff, "show book with title " + title, -1);
    return ret;
}

std::vector<Book::BookInfo> Book::show_author(const std::string &author, int privilege, const std::string &staff) {
    if (!User::check_pri(privilege)) return {};
    if (!check_str(author)) return {};
    if (privilege < 1) return {};
    auto res = author2id.find(binstring<max_str_len>(author));
    std::vector<BookInfo> ret;
    for (auto &p : res) {
        ret.push_back(db[p.second]);
    }
    Log::showbook(staff, "show book with author " + author, -1);
    return ret;
}   

std::vector<Book::BookInfo> Book::show_keyword(const std::string &keyword, int privilege, const std::string &staff) {
    // keyword
    if (!User::check_pri(privilege)) return {};
    if (!Keywords::check_single_kw(keyword)) return {};
    if (privilege < 1) return {};
    auto res = keyword2id.find(binstring<max_str_len>(keyword));
    std::vector<BookInfo> ret;
    for (auto &p : res) {
        ret.push_back(db[p.second]);
    }
    Log::showbook(staff, "show book with keyword " + keyword, -1);
    return ret;
}

int Book::select(const std::string &isbn, int privilege) {
    if (!User::check_pri(privilege)) return -1;
    if (privilege < 3) return -1;
    if (!check_isbn(isbn)) return -1;
    auto res = isbn2id.get(binstring<max_isbn_len>(isbn));
    if (!res.first) {
        db.push_back(BookInfo{db.size(), binstring<max_isbn_len>(isbn), binstring<max_str_len>(""), binstring<max_str_len>(""), binstring<max_str_len>(""), 0, 0});
        isbn2id.insert(std::make_pair(binstring<max_isbn_len>(isbn), db.size() - 1));
        return db.size() - 1;
    }
    return res.second.second;
}

bool Book::modify(const std::string & userid, int book_id, int privilege, const std::string &isbn, const std::string &title, const std::string &author, const std::string &keyword, Price_T price) {
    if (book_id < 0 || book_id >= db.size()) return false;
    if (!User::check_pri(privilege)) return false;
    if (privilege < 3) return false;
    if (title != "") {
        if (!check_str(title)) return false;
    }
    if (author != "") {
        if (!check_str(author)) return false;
    }
    if (keyword != "") {
        if (!Keywords::check(keyword)) return false;
    }
    if (price != -1) {
        if (price < 0) return false;
    }
    auto book = db[book_id];
    if (isbn == book.ISBN) {
        return false;
    }
    if (isbn != "") {
        if (!check_isbn(isbn)) return false;
        auto res = isbn2id.get(binstring<max_isbn_len>(isbn));
        if (!res.first) {
            return false;
        }
        isbn2id.erase(binstring<max_isbn_len>(book.ISBN));
        res.second.first = binstring<max_isbn_len>(isbn);
        isbn2id.insert(res.second);
    }
    if (title != "") {
        title2id.erase(binstring<max_str_len>(book.Title), book_id);
        book.Title = binstring<max_str_len>(title);
        title2id.insert(binstring<max_str_len>(title), book_id);
    }
    if (author != "") {
        author2id.erase(binstring<max_str_len>(book.Author), book_id);
        book.Author = binstring<max_str_len>(author);
        author2id.insert(binstring<max_str_len>(author), book_id);
    }
    if (keyword != "") {
        auto kws = Keywords::get(book.Keyword);
        for (auto &kw : kws) {
            keyword2id.erase(binstring<max_str_len>(kw), book_id);
        }
        kws = Keywords::get(keyword);
        book.Keyword = binstring<max_str_len>(keyword);
        for (auto &kw : kws) {
            keyword2id.insert(binstring<max_str_len>(kw), book_id);
        }
    }
    if (price != -1) {
        book.price = price;
    }
    db.update(book, book_id);
    Log::bookmodify(userid, book_id, "modify book " + book.ISBN);
    return true;
}

bool Book::Keywords::check(const std::string &kw) {
    if (kw.size() > max_str_len - 1) return false;
    auto kws = get(kw);
    if (kws.empty()) return false;
    return true;
}

std::set<std::string> Book::Keywords::get(const std::string &kw) {
    std::set<std::string> ret;
    std::string tmp;
    std::stringstream ss(kw);
    while (std::getline(ss, tmp, '|')) {
        if (! check_single_kw(tmp)) return {};
        auto res = ret.insert(tmp);
        if (!res.second) return {};
    }
    return ret;
}

Book::Price_T Deal::buy(const std::string & user, const std::string &isbn, int quantity, int privilege) {
    if (!User::check_pri(privilege)) return -1;
    if (privilege < 1) return -1;
    if (!Book::check_isbn(isbn)) return -1;
    if (quantity <= 0) return -1;
    auto res = Book::isbn2id.get(binstring<Book::max_isbn_len>(isbn));
    if (!res.first) return -1;
    auto book = Book::db[res.second.second];
    if (book.Stock < quantity) return -1;
    book.Stock -= quantity;
    Book::db.update(book, res.second.second);
    db.push_back(DealInfo{db.size(), DealType::Sale, book.price, quantity});
    Log::dealsale(user, res.second.second, "buy book " + isbn + " " + std::to_string(quantity));
    return book.price * quantity;
}

Book::Price_T Deal::import(const std::string & user, int book_id, int quantity, Book::Price_T total_cost, int privilege) {
    if (!User::check_pri(privilege)) return -1;
    if (privilege < 3) return -1;
    if (quantity <= 0) return -1;
    if (total_cost < 0) return -1;
    if (book_id < 0 || book_id >= Book::db.size()) return -1;
    auto book = Book::db[book_id];
    book.Stock += quantity;
    Book::db.update(book, book_id);
    db.push_back(DealInfo{db.size(), DealType::Import, total_cost, quantity});
    Log::dealimport(user, book_id, "import book " + book.ISBN + " " + std::to_string(quantity));
    return total_cost;
}

std::string Deal::show_finance(int count) {
    if (count == -1) {
        count = db.size();
    }
    Book::Price_T in = 0, out = 0;
    auto list = db.range(db.size() - count, db.size());
    for (auto &deal : list) {
        if (deal.type == DealType::Sale) {
            in += deal.price;
        }
        else {
            out += deal.price;
        }
    }
    return "+ " + std::to_string(in) + " - " + std::to_string(out);
}

void Deal::report_finance() {
}

// namespace Log {
void Log::login(const std::string &id) {
    db.push_back(LogInfo{db.size(), OpType::UserLogin, id, -1, ""});
}

void Log::logout(const std::string &id) {
    db.push_back(LogInfo{db.size(), OpType::UserLogout, id, -1, ""});
}

void Log::useradd(const std::string &userId, const std::string &info) {
    db.push_back(LogInfo{db.size(), OpType::UserAdd, userId, -1, info});
}

void Log::userdelete(const std::string &userId, const std::string &info) {
    db.push_back(LogInfo{db.size(), OpType::UserDelete, userId, -1, info});
}

void Log::userpw(const std::string &userId, const std::string &info) {
    db.push_back(LogInfo{db.size(), OpType::UserPW, userId, -1, info});
}

void Log::showbook(const std::string &userId, const std::string &info, int book_id) {
    db.push_back(LogInfo{db.size(), OpType::BookAdd, userId, book_id, info});
}

void Log::bookadd(const std::string &userId, int book_id, const std::string &info) {
    db.push_back(LogInfo{db.size(), OpType::BookAdd, userId, book_id, info});
}

void Log::bookmodify(const std::string &userId, int book_id, const std::string &info) {
    db.push_back(LogInfo{db.size(), OpType::BookModify, userId, book_id, info});
}

void Log::dealimport(const std::string &userId, int deal_id, const std::string &info) {
    db.push_back(LogInfo{db.size(), OpType::DealImport, userId, deal_id, info});
}

void Log::dealsale(const std::string &userId, int deal_id, const std::string &info) {
    db.push_back(LogInfo{db.size(), OpType::DealSale, userId, deal_id, info});
}

void Log::report_employee() {
}

void Log::getlog() {
}



