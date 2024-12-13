#ifndef MODELS_HPP
#define MODELS_HPP

#include "kvdb.hpp"
#include "randomdb.cpp"
#include "seqdb.cpp"
#include "kvdb.cpp"
#include "seqdb.hpp"
#include <array>
#include <string>
#include <vector>
#include <set>

constexpr static const int INSTANCE_ID = 1;

template<int max>
struct binstring {
    std::array<char, max> str;
    constexpr const static int max_size = max;
    char* to_bin();
    void from_bin(char* bin);
    constexpr static const int bin_size();
    binstring(const std::string& s) : str(s) {}
    binstring() {str.resize(max_size);};
    binstring(const char* s) : str(s) {}
    const char* c_str() const {return str.c_str();}
    operator std::string() const {
        return str;
    }
    friend std::ostream& operator<<(std::ostream& os, const binstring& bs) {
        os << bs.str;
        return os;
    }
    binstring<max_size> & operator=(const binstring<max_size> &a) {
        str.resize(max_size);
        str.assign(a.str, 0, max_size - 1);
        return *this;
    }
};

template<int max_size>
void binstring<max_size>::from_bin(char* bin) {
    str.resize(max_size);
    str.assign(bin, 0, max_size - 1);
}

template<int max_size>
char* binstring<max_size>::to_bin() {
    char* bin = new char[max_size];
    std::copy(str.c_str(), str.c_str() + max_size, bin);
    return bin;
}

template<int max_size>
constexpr const int binstring<max_size>::bin_size() {
    return max_size;
}

template<int max_len>
struct textcmp {
    bool operator()(const binstring<max_len> & a, const binstring<max_len> & b) const {
        return strcmp(a.c_str(), b.c_str()) < 0;
    }
};

namespace User {
    constexpr static const int max_str_len = 31;
    using UserId = binstring<max_str_len>;
    struct UserInfo {
        UserId id;
        binstring<max_str_len> Username;
        binstring<max_str_len> Password;
        int Privilege;
        char* to_bin() {
            char* bin = new char[bin_size()];
            char* ptr = bin;
            std::copy(id.c_str(), id.c_str() + binstring<max_str_len>::bin_size(), ptr);
            ptr += binstring<max_str_len>::bin_size();
            std::copy(Username.c_str(), Username.c_str() + binstring<max_str_len>::bin_size(), ptr);
            ptr += binstring<max_str_len>::bin_size();
            std::copy(Password.c_str(), Password.c_str() + binstring<max_str_len>::bin_size(), ptr);
            ptr += binstring<max_str_len>::bin_size();
            std::copy(reinterpret_cast<char*>(&Privilege), reinterpret_cast<char*>(&Privilege) + sizeof(int), ptr);
            return bin;
        }
        void from_bin(char* bin) {
            char* ptr = bin;
            id.from_bin(ptr);
            ptr += binstring<max_str_len>::bin_size();
            Username.from_bin(ptr);
            ptr += binstring<max_str_len>::bin_size();
            Password.from_bin(ptr);
            ptr += binstring<max_str_len>::bin_size();
            std::copy(ptr, ptr + sizeof(int), reinterpret_cast<char*>(&Privilege));
        }
        constexpr static const int bin_size() {
            return binstring<max_str_len>::bin_size() * 3 + sizeof(int);
        }
    };
    RandomDB<UserId, textcmp<max_str_len>, UserInfo> db;
    int su(const std::string &id, const std::string &password = "", int privilege = 0);
    bool passwd(const std::string &id, const std::string &new_password, const std::string & staff, int privilege, const std::string &old_password = "");
    bool useradd(const std::string &id, const std::string &password, const std::string &username, int privilege, int current_pri, const std::string &staff);
    bool Register(const std::string &id, const std::string &password, const std::string &username) {return useradd(id, password, username, 1, 2, "guest");}
    bool Delete(const std::string &id, const std::string &staff, int privilege);
    bool check_id(const std::string &id);
    bool check_str(const std::string &id);
    bool check_name(const std::string &id);
    bool check_pri(int privilege);
}

namespace Book {
    constexpr static const int max_isbn_len = 21;
    using ISBN_T = binstring<max_isbn_len>;
    constexpr static const int max_str_len = 61;
    using Price_T = long long; // 定点两位，整数是10位
    struct BookInfo {
        int id;
        binstring<max_isbn_len> ISBN;
        binstring<max_str_len> Title;
        binstring<max_str_len> Author;
        binstring<max_str_len> Keyword; // keywords
        int Stock = 0;
        Price_T price = 0;
        char* to_bin() {
            char* bin = new char[bin_size()];
            char* ptr = bin;
            std::copy(reinterpret_cast<char*>(&id), reinterpret_cast<char*>(&id) + sizeof(int), ptr);
            ptr += sizeof(int);
            std::copy(ISBN.c_str(), ISBN.c_str() + binstring<max_isbn_len>::bin_size(), ptr);
            ptr += binstring<max_isbn_len>::bin_size();
            std::copy(Title.c_str(), Title.c_str() + binstring<max_str_len>::bin_size(), ptr);
            ptr += binstring<max_str_len>::bin_size();
            std::copy(Author.c_str(), Author.c_str() + binstring<max_str_len>::bin_size(), ptr);
            ptr += binstring<max_str_len>::bin_size();
            std::copy(Keyword.c_str(), Keyword.c_str() + binstring<max_str_len>::bin_size(), ptr);
            ptr += binstring<max_str_len>::bin_size();
            std::copy(reinterpret_cast<char*>(&Stock), reinterpret_cast<char*>(&Stock) + sizeof(int), ptr);
            ptr += sizeof(int);
            std::copy(reinterpret_cast<char*>(&price), reinterpret_cast<char*>(&price) + sizeof(Price_T), ptr);
            return bin;
        }
        void from_bin(char* bin) {
            char* ptr = bin;
            std::copy(ptr, ptr + sizeof(int), reinterpret_cast<char*>(&id));
            ptr += sizeof(int);
            ISBN.from_bin(ptr);
            ptr += binstring<max_isbn_len>::bin_size();
            Title.from_bin(ptr);
            ptr += binstring<max_str_len>::bin_size();
            Author.from_bin(ptr);
            ptr += binstring<max_str_len>::bin_size();
            Keyword.from_bin(ptr);
            ptr += binstring<max_str_len>::bin_size();
            std::copy(ptr, ptr + sizeof(int), reinterpret_cast<char*>(&Stock));
            ptr += sizeof(int);
            std::copy(ptr, ptr + sizeof(Price_T), reinterpret_cast<char*>(&price));
        }
        constexpr static const int bin_size() {
            return sizeof(int) + binstring<max_isbn_len>::bin_size() + binstring<max_str_len>::bin_size() * 3 + sizeof(int) + sizeof(Price_T);
        }
    };
    bool check_isbn(const std::string &isbn);
    bool check_str(const std::string &str);
    SeqDB<class BookInfo> db("book", INSTANCE_ID, "data/");
    RandomDB<ISBN_T, textcmp<max_isbn_len>, int> isbn2id("book_isbn_index", INSTANCE_ID, "data/", false);
    KVDB<int, std::less<int>, max_str_len> title2id("book_title_index", INSTANCE_ID, "data/", false);
    KVDB<int, std::less<int>, max_str_len> author2id("book_author_index", INSTANCE_ID, "data/", false);
    KVDB<int, std::less<int>, max_str_len> keyword2id("book_keyword_index", INSTANCE_ID, "data/", false);
    // 以 [ISBN] 字典升序依次输出
    std::vector<BookInfo> show_isbn(const std::string &isbn, int privilege, const std::string &staff);
    std::vector<BookInfo> show_title(const std::string &title, int privilege, const std::string &staff);
    std::vector<BookInfo> show_author(const std::string &author, int privilege, const std::string &staff);
    std::vector<BookInfo> show_keyword(const std::string &keyword, int privilege, const std::string &staff);
    int select(const std::string &isbn, int privilege, const std::string & user); // 返回书的id
    bool modify(const std::string & userid, int book_id, int privilege, const std::string &isbn = "", const std::string &title = "", const std::string &author = "", const std::string &keyword = "", Price_T price = -1);

    namespace Keywords {
        bool check(const std::string &kw);
        std::set<std::string> get(const std::string &kw);
        bool check_single_kw(const std::string &str);
    }
}

namespace Deal {
    enum DealType {
        Import,
        Sale
    };
    struct DealInfo {
        int id;
        DealType type;
        Book::Price_T price = 0; // 注意，如果是Import是总价，如果是Sale是单价
        int quantity;
    };
    SeqDB<DealInfo> db("deal", INSTANCE_ID, "data/");
    Book::Price_T buy(const std::string & user, const std::string &isbn, int quantity, int privilege);
    Book::Price_T import(const std::string & user, int book_id, int quantity, Book::Price_T total_cost, int privilege);
    std::pair<Book::Price_T, Book::Price_T> show_finance(int count = -1);
    void report_finance();  // 不一定是void，待定
}

namespace Log {
    constexpr static const int max_info_len = 64;
    enum OpType {
        UserLogin,
        UserLogout,
        UserAdd,
        UserDelete,
        UserPW,
        BookAdd,
        BookModify,
        DealImport,
        DealSale
    };
    struct LogInfo {
        int id;
        OpType type;
        binstring<User::max_str_len> user;
        int record_id;
        binstring<max_info_len> info;
        constexpr static const int bin_size() {
            return sizeof(int) + sizeof(OpType) + binstring<User::max_str_len>::bin_size() + sizeof(int) + binstring<max_info_len>::bin_size();
        }
        char* to_bin() {
            char* bin = new char[bin_size()];
            char* ptr = bin;
            std::copy(reinterpret_cast<char*>(&id), reinterpret_cast<char*>(&id) + sizeof(int), ptr);
            ptr += sizeof(int);
            std::copy(reinterpret_cast<char*>(&type), reinterpret_cast<char*>(&type) + sizeof(OpType), ptr);
            ptr += sizeof(OpType);
            std::copy(user.c_str(), user.c_str() + binstring<User::max_str_len>::bin_size(), ptr);
            ptr += binstring<User::max_str_len>::bin_size();
            std::copy(reinterpret_cast<char*>(&record_id), reinterpret_cast<char*>(&record_id) + sizeof(int), ptr);
            ptr += sizeof(int);
            std::copy(info.c_str(), info.c_str() + binstring<max_info_len>::bin_size(), ptr);
            return bin;
        }
        void from_bin(char* bin) {
            char* ptr = bin;
            std::copy(ptr, ptr + sizeof(int), reinterpret_cast<char*>(&id));
            ptr += sizeof(int);
            std::copy(ptr, ptr + sizeof(OpType), reinterpret_cast<char*>(&type));
            ptr += sizeof(OpType);
            user.from_bin(ptr);
            ptr += binstring<User::max_str_len>::bin_size();
            std::copy(ptr, ptr + sizeof(int), reinterpret_cast<char*>(&record_id));
            ptr += sizeof(int);
            info.from_bin(ptr);
        }
    };
    SeqDB<LogInfo> db("log", INSTANCE_ID, "data/");
    KVDB<int, std::less<int>, User::max_str_len> index_db("op2log", INSTANCE_ID, "data/", false);
    void report_employee(); // 不一定是void，待定
    void getlog(); // 不一定是void，待定
    void login(const std::string & userId);
    void logout(const std::string & userId);
    void useradd(const std::string & userId, const std::string & info);
    void userdelete(const std::string & userId, const std::string & info);
    void userpw(const std::string & userId, const std::string & info);
    void showbook(const std::string & userId, const std::string & info, int book_id);
    void bookadd(const std::string & userId, int book_id, const std::string & info);
    void bookmodify(const std::string & userId, int book_id, const std::string & info);
    void dealimport(const std::string & userId, int deal_id, const std::string & info);
    void dealsale(const std::string & userId, int deal_id, const std::string & info);
}

#endif