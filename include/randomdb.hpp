#ifndef _random_db
#define _random_db

#include "MemoryRiver.hpp"
#include "BlockRiver.hpp"
#include <functional>
#include <string>
#include <vector>
#include "binable.hpp"

struct Empty {};

template<class T, class Comp=std::less<T>, class Attachment = Empty>
class RandomDB {
    using head_index = int;
    using arr_index = int;
    constexpr static const int sizeofT = (binable<T>) ? T::bin_size() : sizeof(T);
    constexpr static const int sizeofA = (binable<Attachment>) ? Attachment::bin_size() : sizeof(Attachment);
public:
    struct T_A_pair {
        T first;
        Attachment second;
        const char* to_bin();
        const void from_bin(char* bin);
        constexpr static const int bin_size();
    };
    constexpr static const int array_size = (4096 - sizeof(bool) - sizeof(int)) / T_A_pair::bin_size();
    struct Comp_A {
        bool operator()(const T_A_pair& a, const T_A_pair& b) const {
            return Comp()(a.first, b.first);
        }
        bool operator()(const T&a, const T& b) const {
            return Comp()(a, b);
        }
        bool operator()(const T& a, const T_A_pair& b) const {
            return Comp()(a, b.first);
        }
        bool operator()(const T_A_pair& a, const T& b) const {
            return Comp()(a.first, b);
        }
    };
public:
    struct head {
        T begin;
        T end;
        head_index next;
        arr_index body;
        const char* to_bin();
        const void from_bin(char* bin); // 虚假const
        constexpr static const int bin_size();
    };
    struct array {
        T_A_pair data[array_size];
        int size;
        bool deprecated = false;
        const char* to_bin();
        const void from_bin(char* bin);
        constexpr static const int bin_size();
    };
    RandomDB(std::string dbname, int db_id = 0, std::string path = "", bool duplicate_allowed = true);
    ~RandomDB();
    void insert(const T& t);
    void insert(const T_A_pair& t);
    void insert(const std::pair<T, Attachment>& t);
    bool erase(const T& t);
    std::pair<head_index, int> upper_bound(const T& t);
    std::pair<head_index, int> lower_bound(const T& t);
    bool exist(const T& t);
    std::vector<T_A_pair> find(const T& t);
    void enable_duplicate();
    class DBFileNotMatchException;
    class MissingFileException;
    class DuplicateException;
    std::vector<T_A_pair> range(head_index lh, int lp, head_index rh, int rp);
private:
    MemoryRiver<head, 16> head_river;
    BlockRiver<array> body_river;
    head_index head_begin = -1;
    head_index head_end = -1;
    head_index db_id;
    bool duplicate_allowed;
protected:
    arr_index locate(const T& t);
    // arr_index locate(const T_A_pair& t);
    head get_head(head_index idx);
    array get_body(arr_index idx);
    head_index add_head(const T_A_pair& t, head_index prev = -1);
};

#endif