#include "randomdb.hpp"
#include <filesystem>
#include <algorithm>
#include <exception>

template<class T, class Comp>
class RandomDB<T, Comp>::MissingFileException : std::exception {};

template<class T, class Comp>
class RandomDB<T, Comp>::DBFileNotMatchException : std::exception {};

template<class T, class Comp>
class RandomDB<T, Comp>::DuplicateException : std::exception {};

template<class T, class Comp>
RandomDB<T, Comp>::RandomDB(std::string dbname, int db_id, std::string path, bool duplicate_allowed) : db_id(db_id), duplicate_allowed(duplicate_allowed) {
    if (!std::filesystem::exists(path + dbname + ".head")) {
        head_river.initialise(path + dbname + ".head");
        head_river.write_info(db_id, 1);
        head_river.write_info(sizeof(T) , 2);
        head_river.write_info(head_begin, 3);
        head_river.write_info(head_end, 4);
        head_river.write_info(duplicate_allowed, 5);
        body_river.initialise(path + dbname + ".data");
    }
    else {
        if (!std::filesystem::exists(path + dbname + ".data")) {
            throw MissingFileException();
        }
        head_river.bind(path + dbname + ".head");
        body_river.bind(path + dbname + ".data");
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
        int da;
        head_river.get_info(da, 5);
        if (da && (! duplicate_allowed)) {
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
    body_river.close();
}

template<class T, class Comp>
typename RandomDB<T, Comp>::head RandomDB<T, Comp>::get_head(head_index idx) {
    head tmp;
    head_river.read(tmp, idx);
    return tmp;
}

template<class T, class Comp>
typename RandomDB<T, Comp>::array RandomDB<T, Comp>::get_body(arr_index idx) {
    array tmp;
    body_river.read(tmp, idx);
    return tmp;
}

template<class T, class Comp>
typename RandomDB<T, Comp>::arr_index RandomDB<T, Comp>::locate(const T& t) {
    head_index idx = head_begin;
    head_index prv = -1;
    while (idx != -1) {
        head tmp = get_head(idx);
        if (Comp()(t, tmp.begin)) {
            if (prv == -1) {
                return -1;
            }
            break;
        }
        prv = idx;
        idx = tmp.next;
    }
    return prv;
}

template<class T, class Comp>
typename RandomDB<T, Comp>::head_index 
RandomDB<T, Comp>::add_head(const T& t, head_index prev) {
    if (prev == -1) {
        prev = head_end;
    }
    arr_index c = body_river.write(array{{t}, 1});
    if (prev != -1) {
        auto p = get_head(prev);
        p.next = head_river.write(head{t, t, p.next, c});
        head_river.update(p, prev);
        head_end = p.next;
        head_river.write_info(head_end, 4);
        return p.next;
    }
    else {
        head_begin = head_river.write(head{t, t, -1, c});
        head_end = head_begin;
        head_river.write_info(head_begin, 3);
        head_river.write_info(head_end, 4);
        return head_begin;
    }
}

template<class T, class Comp>
void RandomDB<T, Comp>::insert(const T& t) {
    head_index idx = locate(t);
    if (idx == -1) {
        if (head_begin == -1) {
            add_head(t);
            return;
        }
        idx = head_begin;
    }
    head h = get_head(idx);
    array content = get_body(h.body);
    if (content.size < array_size) {
        auto back = std::upper_bound(content.data, content.data + content.size, t, Comp());
        if (!duplicate_allowed) {
            auto p = back;
            p --;
            if (!(Comp()(t, *p) || Comp()(*p, t))) { // == !(Comp()(t, *p) || Comp()(*p, t))
                throw DuplicateException();
            }
        }
        std::copy_backward(back, content.data + content.size, 
        content.data + content.size + 1);
        *back = t;
        content.size++;
        if (Comp()(h.end, t)) {
            h.end = t;
            head_river.update(h, idx);
        }
        if (Comp()(t, h.begin)) {
            h.begin = t;
            head_river.update(h, idx);
        }
        body_river.update(content, h.body);
        return;
    }
    else {
        if (Comp()(h.end, t)) {
            add_head(t);
            return;
        }
        array cont = get_body(h.body);
        array new_arr = array{};
        new_arr.size = array_size/2;
        for (int i = 0; i < array_size / 2; i++) {
            new_arr.data[i] = cont.data[i + array_size - array_size / 2];
        }
        new_arr.size = array_size / 2;
        cont.size = array_size - array_size / 2;
        h.end = cont.data[array_size - array_size / 2 - 1];
        head new_head = head{new_arr.data[0], new_arr.data[array_size / 2 - 1], h.next};
        if (!Comp()(t, new_head.begin)) {
            auto back = std::upper_bound(new_arr.data, new_arr.data + new_arr.size, t, Comp());
            std::copy_backward(back, new_arr.data + new_arr.size,
             new_arr.data + new_arr.size + 1);
            new_arr.size ++;
            *back = t;
            if (Comp()(new_head.end, t)) {
                new_head.end = t;
            }
        }
        else {
            auto back = std::upper_bound(cont.data, cont.data + cont.size, t, Comp());
            std::copy_backward(back, cont.data + cont.size,
             cont.data + cont.size + 1);
            cont.size ++;
            *back = t;
            if (Comp()(h.end, t)) {
                h.end = t;
            }
            if (Comp()(t, h.begin)) {
                h.begin = t;
            }
        }
        arr_index a = body_river.write(new_arr);
        new_head.body = a;
        h.next = head_river.write(new_head);
        head_river.update(h, idx);
        if (idx == head_end) {
            head_end = h.next;
            head_river.write_info(head_end, 4);
        }
        body_river.update(cont, h.body);
    }
}

template<class T, class Comp>
bool RandomDB<T, Comp>::erase(const T& t) {
    head_index idx = locate(t);
    if (idx == -1) {
        return false;
    }
    head h = get_head(idx);
    array content = get_body(h.body);
    auto bigger = 
    std::upper_bound(content.data, content.data + content.size, t, Comp());
    if (bigger==content.data) {
        return false;
    }
    auto p = bigger;
    p--;
    if (Comp()(t, *p) || Comp()(*p, t)) {
        return false;
    }
    std::copy(bigger, content.data + content.size, bigger - 1);
    content.size --;
    if (bigger == content.data + content.size + 1) {
        h.end = content.data[content.size - 1];
        head_river.update(h, idx);
    }
    if (p == content.data) {
        h.begin = content.data[0];
        head_river.update(h, idx);
    }
    body_river.update(content, h.body);
    return true;
}

template<class T, class Comp>
std::pair<typename RandomDB<T, Comp>::head_index, int>
RandomDB<T, Comp>::upper_bound(const T& t) {
    head_index idx = locate(t);
    if (idx == -1) {
        return std::make_pair(-1, 0);
    }
    head h = get_head(idx);
    array content = get_body(h.content);
    auto bigger = 
    std::upper_bound(content.data, content.data + content.size, t, Comp());
    if (bigger == content.data + content.size) {
        return std::make_pair(h.next, 0);
    }
    return std::make_pair(idx, content.data + content.size - bigger);
}

template<class T, class Comp>
std::pair<typename RandomDB<T, Comp>::head_index, int> RandomDB<T, Comp>::lower_bound(const T& t) {
    head_index idx = locate(t);
    if (idx == -1) {
        return std::make_pair(-1, 0);
    }
    head h = get_head(idx);
    array content = get_body(h.content);
    auto bigger = 
    std::lower_bound(content.data, content.data + content.size, t, Comp());
    if (bigger == content.data + content.size) {
        return std::make_pair(h.next, 0);
    }
    return std::make_pair(idx, content.data + content.size - bigger);
}

template<class T, class Comp>
bool RandomDB<T, Comp>::exist(const T& t) {
    head_index idx = locate(t);
    if (idx == -1) {
        return false;
    }
    head h = get_head(idx);
    array content = get_body(h.body);
    return std::binary_search(content.data, content.data + content.size, t, Comp());
}

template<class T, class Comp>
void RandomDB<T, Comp>::enable_duplicate() {
    duplicate_allowed = true;
    head_river.write_info(1, 5);
}

template<class T, class Comp>
const int RandomDB<T, Comp>::head::bin_size() {
    if constexpr (binable<T>) {
        return T::bin_size() * 2 + sizeof(head_index) + sizeof(arr_index);
    }
    else {
        return sizeof(T) * 2 + sizeof(head_index) + sizeof(arr_index) + sizeof(int);
    }
}

template<class T, class Comp>
const char* RandomDB<T, Comp>::head::to_bin() {
    if constexpr (binable<T>) {
        char* bin = new char[bin_size()];
        char* ptr = bin;
        auto data = begin.to_bin();
        std::copy(data, data + sizeofT, ptr);
        ptr += sizeofT;
        delete[] data;
        char* data2 = end.to_bin();
        std::copy(data2, data2 + sizeofT, ptr);
        ptr += sizeofT;
        std::copy(reinterpret_cast<char*>(&next), reinterpret_cast<char*>(&next) + sizeof(head_index), ptr);
        ptr += sizeof(head_index);
        std::copy(reinterpret_cast<char*>(&body), reinterpret_cast<char*>(&body) + sizeof(arr_index), ptr);
        return bin;
    }
    else {
        return reinterpret_cast<char*>(this);
    }
}

template<class T, class Comp>
const void RandomDB<T, Comp>::head::from_bin(char* bin) {
    if constexpr (binable<T>) {
        char* ptr = bin;
        this->begin.from_bin(ptr);
        ptr += sizeofT;
        this->end.from_bin(ptr);
        ptr += sizeofT;
        std::copy(ptr, ptr + sizeof(head_index), reinterpret_cast<char*>(&next));
        ptr += sizeof(head_index);
        std::copy(ptr, ptr + sizeof(arr_index), reinterpret_cast<char*>(&body));
    }
    else {
        std::copy(bin, bin + sizeof(head), reinterpret_cast<char*>(this));
    }
}

template<class T, class Comp>
const int RandomDB<T, Comp>::array::bin_size() {
    if constexpr (binable<T>) {
        return sizeof(int) + sizeof(bool) + T::bin_size() * array_size;
    }
    else {
        return sizeof(int) + sizeof(bool) + sizeof(T) * array_size + sizeof(int);
    }
}

template<class T, class Comp>
const char* RandomDB<T, Comp>::array::to_bin() {
    if constexpr (binable<T>) {
        char* bin = new char[bin_size()];
        char* ptr = bin;
        std::copy(reinterpret_cast<char*>(&size), reinterpret_cast<char*>(&size) + sizeof(int), ptr);
        ptr += sizeof(int);
        std::copy(reinterpret_cast<char*>(&deprecated), reinterpret_cast<char*>(&deprecated) + sizeof(bool), ptr);
        ptr += sizeof(bool);
        for (int i = 0; i < size; i++) {
            char* str_data = data[i].to_bin();
            std::copy(str_data, str_data + sizeofT, ptr);
            ptr += sizeofT;
            delete[] str_data;
        }
        return bin;
    }
    else {
        return reinterpret_cast<char*>(this);
    }
}

template<class T, class Comp>
const void RandomDB<T, Comp>::array::from_bin(char* bin) {
    if constexpr (binable<T>) {
        char* ptr = bin;
        std::copy(ptr, ptr + sizeof(int), reinterpret_cast<char*>(&size));
        ptr += sizeof(int);
        std::copy(ptr, ptr + sizeof(bool), reinterpret_cast<char*>(&deprecated));
        ptr += sizeof(bool);
        for (int i = 0; i < size; i++) {
            char* str_data = new char[sizeofT];
            std::copy(ptr, ptr + sizeofT, str_data);
            data[i].from_bin(str_data);
            delete[] str_data;
            ptr += sizeofT;
        }
    }
    else {
        std::copy(bin, bin + sizeof(array), reinterpret_cast<char*>(this));
    }
}