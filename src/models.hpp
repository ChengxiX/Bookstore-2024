#include "randomdb.cpp"
#include "seqdb.cpp"
#include "kvdb.cpp"
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
    struct info {
        UserId id;
        char Username[max_str_len];
        char Password[max_str_len];
        User::UserId staff;
        int Privilege;
    };
    RandomDB<UserId, textcmp, info> db("user", INSTANCE_ID, "data/", false);
    bool su(std::string id, std::string password = "", int privilege = 0);
    bool logout();
    bool passwd(std::string id, std::string new_password, std::string old_password = "");
    bool useradd(std::string id, std::string password, std::string username, int privilege);
    bool Register(std::string id, std::string password, std::string username) {return useradd(id, password, username, 1);}
    bool Delete(std::string id);
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
        User::UserId staff;
        int Stock;
        Price_T price; 
    };
    SeqDB<class BookInfo> db("book", INSTANCE_ID, "data/");
    RandomDB<ISBN_T, textcmp, int> isbn2id("book_isbn_index", INSTANCE_ID, "data/", false);
    KVDB<int, std::less<int>, max_str_len> title2id("book_title_index", INSTANCE_ID, "data/", false);
    KVDB<int, std::less<int>, max_str_len> author2id("book_author_index", INSTANCE_ID, "data/", false);
    KVDB<int, std::less<int>, max_str_len> keyword2id("book_keyword_index", INSTANCE_ID, "data/", false);
    // 以 [ISBN] 字典升序依次输出
    std::vector<BookInfo> show_isbn(std::string isbn);
    std::vector<BookInfo> show_title(std::string title);
    std::vector<BookInfo> show_author(std::string author);
    std::vector<BookInfo> show_keyword(std::string keyword);
    int select(std::string isbn); // 返回书的id
    bool modify(int book_id, std::string isbn = "", std::string title = "", std::string author = "", std::string keyword = "", Price_T price = "");
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
        User::UserId staff;
    };
    SeqDB<DealInfo> db("deal", INSTANCE_ID, "data/");
    Book::Price_T buy(std::string isbn, int quantity);
    Book::Price_T import(int book_id, int quantity, Book::Price_T total_cost);
    std::string show_finance(int count = -1);
    void report_finance();  // 不一定是void，待定
    void report_employee(); // 不一定是void，待定
    void log(); // 不一定是void，待定
}
