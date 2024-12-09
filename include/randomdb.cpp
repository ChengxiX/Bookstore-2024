#include "randomdb.hpp"
#include <filesystem>

template<class T, class Comp>
RandomDB<T, Comp>::RandomDB(std::string dbname, int db_id, std::string path) : db_id(db_id) {
    if (!std::filesystem::exists(path + dbname + ".head")) {
        head_river.initialise(path + dbname + ".head");
        head_river.write_info(db_id, 1);
        head_river.write_info(sizeof(T) , 2);
        head_river.write_info(head_begin, 3);
        head_river.write_info(head_end, 4);
        content_river.initialise(path + dbname + ".arr");
    }
    else {
        if (!std::filesystem::exists(path + dbname + ".arr")) {
            throw MissingFileException();
        }
        head_river.bind(path + dbname + ".head");
        content_river.bind(path + dbname + ".arr");
        int id;
        head_river.get_info(id, 1);
        if (db_id != 0 && id != 0 && db_id != id) {
            throw DBFileNotMatchException();
        }
        int size;
        head_river.get_info(size, 2);
        if (size != sizeof(T)) {
            throw DBFileNotMatchException();
        }
        head_river.get_info(head_begin, 3);
        head_river.get_info(head_end, 4);
    }
}

template<class T, class Comp>
RandomDB<T, Comp>::~RandomDB() {
    head_river.write_info(head_begin, 3);
    head_river.write_info(head_end, 4);
    head_river.close();
    content_river.close();
}

template<class T, class Comp>
typename RandomDB<T, Comp>::head RandomDB<T, Comp>::get(head_index idx) {
    head tmp;
    head_river.read(tmp, idx);
    return tmp;
}

template<class T, class Comp>
typename RandomDB<T, Comp>::arr_index RandomDB<T, Comp>::locate(const T& t) {
    head_index idx = head_begin;
    head_index prv = -1;
    while (idx != -1) {
        head tmp = get(idx);
        if (Comp()(t, tmp.begin)) {
            if (prv == -1) {
                return -1;
            }
        }
        prv = idx;
        idx = tmp.next;
    }
    return prv;
}

template<class T, class Comp>
typename RandomDB<T, Comp>::head_index RandomDB<T, Comp>::add_head(const T& t, head_index prev) {
    if (prev == -1) {
        prev = head_end;
    }
    arr_index c = content_river.write(array{{t}});
    auto p = get(prev);
    p.next = head_river.write(head{t, t, p.next, c}, prev);
    head_river.update(p, prev);
    return p.next;
}

template<class T, class Comp>
typename RandomDB<T, Comp>::