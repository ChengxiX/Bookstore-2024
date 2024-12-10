#ifndef _random_db
#define _random_db

#include "MemoryRiver.hpp"
#include "BlockRiver.hpp"
#include <functional>
#include <string>
#include "binable.hpp"

template<class T, class Comp=std::less<T>>
class RandomDB {
    using head_index = int;
    using arr_index = int;
    constexpr static const int array_size = (4096 - 1 - 8) / sizeof(T);
    public:
        struct head {
            T begin;
            T end;
            head_index next;
            arr_index body;
            const char* to_bin();
            const void from_bin(char* bin); // 虚假const
            static const int bin_size();
        };
        struct array {
            T data[array_size];
            int size;
            bool deprecated = false;
            const char* to_bin();
            const void from_bin(char* bin);
            static const int bin_size();
        };
        RandomDB(std::string dbname, int db_id = 0, std::string path = "", bool duplicate_allowed = true);
        ~RandomDB();
        void insert(const T& t);
        bool erase(const T& t);
        std::pair<head_index, int> upper_bound(const T& t);
        std::pair<head_index, int> lower_bound(const T& t);
        bool exist(const T& t);
        void enable_duplicate();
        class DBFileNotMatchException;
        class MissingFileException;
        class DuplicateException;
    private:
        MemoryRiver<head, 16> head_river;
        BlockRiver<array> body_river;
        head_index head_begin = -1;
        head_index head_end = -1;
        head_index db_id;
        bool duplicate_allowed;
    protected:
        arr_index locate(const T& t);
        head get_head(head_index idx);
        array get_body(arr_index idx);
        head_index add_head(const T& t, head_index prev = -1);
        static const int sizeofT = binable<T> ? T::bin_size() : sizeof(T);
};

#endif