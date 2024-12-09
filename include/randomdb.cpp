#include "randomdb.hpp"
#include <filesystem>
#include <algorithm>

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
    arr_index c = content_river.write(array{{t}, 1});
    auto p = get(prev);
    p.next = head_river.write(head{t, t, p.next, c});
    head_river.update(p, prev);
    return p.next;
}

template<class T, class Comp>
void RandomDB<T, Comp>::insert(const T& t) {
    head_index idx = locate(t);
    if (idx == -1) {
        auto h = get(head_begin);
        if (h.size < array_size) {
            array content = content_river.read(h.content);
            std::copy_backward(content.data, content.data + content.size, content.data + content.size + 1);
            content.data[0] = t;
            content.size ++;
            h.begin = t;
            content_river.update(content, h.content);
            head_river.update(h, head_begin);
            return;
        }
        else {
            arr_index c = content_river.write(array{{t}, 1});
            head_index hi = head_river.write(head{t, t, head_begin, c});
            head_begin = hi;
            return;
        }
    }
    head h = get(idx);
    if (h.size < array_size) {
        array content = content_river.read(h.content);
        auto back = std::upper_bound(content, content.data + content.size, t, Comp());
        std::copy_backward(back, content.data + content.size, content.data + content.size + 1);
        *back = t;
        content.size++;
        if (Comp()(h.end, t)) {
            h.end = t;
            head_river.update(h, head_end);
        }
        content_river.update(content, h.content);
        return;
    }
    else {
        array cont = content_river.read(h.content);
        array new_arr = array{};
        new_arr.size = array_size/2;
        for (int i = 0; i < array_size / 2; i++) {
            new_arr.data[i] = cont.data[i + array_size - array_size / 2];
        }
        cont.size = array_size / 2;
        h.size = array_size - array_size / 2;
        h.end = cont.data[array_size - array_size / 2 - 1];
        head new_head = head{new_arr.data[0], new_arr.data[array_size / 2 - 1], h.next};
        if (!Comp()(t, new_head.begin)) {
            auto back = std::upper_bound(new_arr, new_arr.data + new_arr.size, t, Comp());
            std::copy_backward(back, new_arr.data + new_arr.size, new_arr.data + new_arr.size + 1);
            new_arr.size ++;
            *back = t;
            if (Comp()(new_head.end, t)) {
                new_head.end = t;
            }
        }
        else {
            auto back = std::upper_bound(cont, cont.data + cont.size, t, Comp());
            std::copy_backward(back, cont.data + cont.size, cont.data + cont.size + 1);
            cont.size ++;
            *back = t;
            if (Comp()(h.end, t)) {
                h.end = t;
            }
        }
        arr_index a = content_river.write(new_arr);
        new_head.content = a;
        head_index hi = head_river.write(new_head);
        h.next = hi;
        head_river.update(h, idx);
    }
}

template<class T, class Comp>
void RandomDB<T, Comp>::erase(const T& t) {
    head_index idx = locate(t);
    if (idx == -1) {
        return;
    }
    head h = get(idx);
    array content = content_river.read(h.content);
    
}
