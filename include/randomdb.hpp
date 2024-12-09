#ifndef _random_db
#define _random_db

#include "MemoryRiver.hpp"
#include "BlockRiver.hpp"
#include <vector>
#include <string>

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
            arr_index content;
        };
        struct array {
            T data[array_size];
            int size;
            bool deplicated = false;
        };
        RandomDB(std::string dbname, int db_id = 0, std::string path = "");
        ~RandomDB();
        void insert(const T& t);
        void erase(const T& t);
        std::vector<T> range(const T& l, const T& r);
        bool exist(const T& t);

        class ValueDuplicateException;
        class DBFileNotMatchException;
        class MissingFileException;
    private:
        MemoryRiver<head, 16> head_river;
        BlockRiver<array> content_river;
        head_index head_begin = -1;
        head_index head_end = -1;
        head_index db_id;
    protected:
        arr_index locate(const T& t);
        head get(head_index idx);
        head_index add_head(const T& t, head_index prev = -1);
};

#endif