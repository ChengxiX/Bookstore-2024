#include "randomdb.cpp"
#include "seqdb.cpp"
#include "kvdb.cpp"
#include "seqdb.hpp"
#include <string>
#include <vector>

constexpr static const int INSTANCE_ID = 1;

struct textcmp {
    bool operator()(const char* a, const char* b) const {
        return strcmp(a, b) < 0;
    }
};

namespace User {
    constexpr static const int max_str_len = 31;
    using UserId = char[max_str_len];
    struct UserInfo {
        UserId id;
        char Username[max_str_len];
        char Password[max_str_len];
        int Privilege;
    };
    RandomDB<UserId, textcmp, UserInfo> db("user", INSTANCE_ID, "data/", false);
    bool su(const std::string &id, const std::string &password = "", int privilege = 0);
    bool passwd(const std::string &id, const std::string &new_password, const std::string &old_password = "", const std::string & staff = "guest");
    bool useradd(const std::string &id, const std::string &password, const std::string &username, int privilege, int current_pri, const std::string &staff);
    bool Register(const std::string &id, const std::string &password, const std::string &username) {return useradd(id, password, username, 1, 2, "guest");}
    bool Delete(const std::string &id);
    bool check_id(const std::string &id);
    bool check_str(const std::string &id);
    bool check_name(const std::string &id);
    bool check_pri(int privilege);
}

namespace Book {
    constexpr static const int max_isbn_len = 21;
    using ISBN_T = char[max_isbn_len];
    constexpr static const int max_str_len = 61;
    using Price_T = long long; // 定点两位，整数是10位
    struct BookInfo {
        int id;
        char ISBN[max_isbn_len];
        char Title[max_str_len];
        char Author[max_str_len];
        char Keyword[max_str_len]; // keywords
        int Stock;
        Price_T price; 
    };
    SeqDB<class BookInfo> db("book", INSTANCE_ID, "data/");
    RandomDB<ISBN_T, textcmp, int> isbn2id("book_isbn_index", INSTANCE_ID, "data/", false);
    KVDB<int, std::less<int>, max_str_len> title2id("book_title_index", INSTANCE_ID, "data/", false);
    KVDB<int, std::less<int>, max_str_len> author2id("book_author_index", INSTANCE_ID, "data/", false);
    KVDB<int, std::less<int>, max_str_len> keyword2id("book_keyword_index", INSTANCE_ID, "data/", false);
    // 以 [ISBN] 字典升序依次输出
    std::vector<BookInfo> show_isbn(const std::string &isbn);
    std::vector<BookInfo> show_title(const std::string &title);
    std::vector<BookInfo> show_author(const std::string &author);
    std::vector<BookInfo> show_keyword(const std::string &keyword);
    int select(const std::string &isbn); // 返回书的id
    bool modify(int book_id, const std::string &isbn = "", const std::string &title = "", const std::string &author = "", const std::string &keyword = "", Price_T price = -1);
}

namespace Deal {
    enum DealType {
        Import,
        Sale
    };
    struct DealInfo {
        int id;
        DealType type;
        Book::Price_T price; // 注意，如果是Import是总价，如果是Sale是单价
        int quantity;
    };
    SeqDB<DealInfo> db("deal", INSTANCE_ID, "data/");
    Book::Price_T buy(const std::string &isbn, int quantity);
    Book::Price_T import(int book_id, int quantity, Book::Price_T total_cost);
    std::string show_finance(int count = -1);
    void report_finance();  // 不一定是void，待定
    
}

namespace Log {
    constexpr static const int max_info_len = 64;
    enum OpType {
        UserLogin,
        UserAdd,
        UserDelete,
        UserModify,
        BookAdd,
        BookModify,
        DealImport,
        DealSale
    };
    struct LogInfo {
        int id;
        OpType type;
        char user[User::max_str_len];
        int record_id;
        char info[max_info_len];
    };
    SeqDB<LogInfo> db("log", INSTANCE_ID, "data/");
    void report_employee(); // 不一定是void，待定
    void log(); // 不一定是void，待定
}