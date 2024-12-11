#include "randomdb.hpp"
#include "binable.hpp"
#include <filesystem>
#include <algorithm>
#include <exception>
#include <type_traits>

template<class T, class Comp, class Attachment>
class RandomDB<T, Comp, Attachment>::MissingFileException : std::exception {};

template<class T, class Comp, class Attachment>
class RandomDB<T, Comp, Attachment>::DBFileNotMatchException : std::exception {};

template<class T, class Comp, class Attachment>
class RandomDB<T, Comp, Attachment>::DuplicateException : std::exception {};

template<class T, class Comp, class Attachment>
RandomDB<T, Comp, Attachment>::RandomDB(std::string dbname, int db_id, std::string path, bool duplicate_allowed) : db_id(db_id), duplicate_allowed(duplicate_allowed) {
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

template<class T, class Comp, class Attachment>
RandomDB<T, Comp, Attachment>::~RandomDB() {
    head_river.write_info(head_begin, 3);
    head_river.write_info(head_end, 4);
    head_river.close();
    body_river.close();
}

template<class T, class Comp, class Attachment>
typename RandomDB<T, Comp, Attachment>::head RandomDB<T, Comp, Attachment>::get_head(head_index idx) {
    head tmp;
    head_river.read(tmp, idx);
    return tmp;
}

template<class T, class Comp, class Attachment>
typename RandomDB<T, Comp, Attachment>::array RandomDB<T, Comp, Attachment>::get_body(arr_index idx) {
    array tmp;
    body_river.read(tmp, idx);
    return tmp;
}

template<class T, class Comp, class Attachment>
typename RandomDB<T, Comp, Attachment>::arr_index RandomDB<T, Comp, Attachment>::locate(const T& t) {
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

template<class T, class Comp, class Attachment>
typename RandomDB<T, Comp, Attachment>::head_index 
RandomDB<T, Comp, Attachment>::add_head(const T_A_pair& t, head_index prev) {
    if (prev == -1) {
        prev = head_end;
    }
    array tmp_body = array{{t}, 1};
    arr_index c = body_river.write(tmp_body);
    if (prev != -1) {
        auto p = get_head(prev);
        head tmp_next = head{t.first, t.first, p.next, c};
        p.next = head_river.write(tmp_next);
        head_river.update(p, prev);
        head_end = p.next;
        head_river.write_info(head_end, 4);
        return p.next;
    }
    else {
        head tmp = head{t.first, t.first, -1, c};
        head_begin = head_river.write(tmp);
        head_end = head_begin;
        head_river.write_info(head_begin, 3);
        head_river.write_info(head_end, 4);
        return head_begin;
    }
}

template<class T, class Comp, class Attachment>
void RandomDB<T, Comp, Attachment>::insert(const std::pair<T, Attachment>& t) {
    insert(T_A_pair{t.first, t.second});
}

template<class T, class Comp, class Attachment>
void RandomDB<T, Comp, Attachment>::insert(const T& t) {
    insert(T_A_pair{t, Attachment{}});
}

template<class T, class Comp, class Attachment>
void RandomDB<T, Comp, Attachment>::insert(const T_A_pair& t_A) {
    head_index idx = locate(t_A.first);
    if (idx == -1) {
        if (head_begin == -1) {
            add_head(t_A);
            return;
        }
        idx = head_begin;
    }
    head h = get_head(idx);
    array content = get_body(h.body);
    if (content.size < array_size) {
        auto back = std::upper_bound(content.data, content.data + content.size, t_A, Comp_A());
        if (!duplicate_allowed && back != content.data) {
            auto p = back;
            p --;
            if (!(Comp()(t_A.first, (*p).first) || Comp()((*p).first, t_A.first))) {
                throw DuplicateException();
            }
        }
        std::copy_backward(back, content.data + content.size, 
        content.data + content.size + 1);
        *back = t_A;
        content.size++;
        if (Comp()(h.end, t_A.first)) {
            h.end = t_A.first;
            head_river.update(h, idx);
        }
        if (Comp()(t_A.first, h.begin)) {
            h.begin = t_A.first;
            head_river.update(h, idx);
        }
        body_river.update(content, h.body);
        return;
    }
    else {
        if (Comp()(h.end, t_A.first)) {
            add_head(t_A, idx);
            return;
        }
        array new_arr = array{};
        new_arr.size = array_size/2;
        content.size = array_size - array_size / 2;
        for (int i = 0; i < new_arr.size; i++) {
            new_arr.data[i] = content.data[i + content.size];
        }
        h.end = content.data[content.size - 1].first;
        head new_head = head{new_arr.data[0].first, new_arr.data[new_arr.size - 1].first, h.next};
        if (!Comp()(t_A.first, new_head.begin)) {
            auto back = std::upper_bound(new_arr.data, new_arr.data + new_arr.size, t_A, Comp_A());
            if (!duplicate_allowed && back != content.data) {
                auto p = back;
                p --;
                if (!(Comp()(t_A.first, (*p).first) || Comp()((*p).first, t_A.first))) {
                    throw DuplicateException();
                }
            }
            std::copy_backward(back, new_arr.data + new_arr.size,
             new_arr.data + new_arr.size + 1);
            new_arr.size ++;
            *back = t_A;
            if (Comp()(new_head.end, t_A.first)) {
                new_head.end = t_A.first;
            }
        }
        else {
            auto back = std::upper_bound(content.data, content.data + content.size, t_A, Comp_A());
            if (!duplicate_allowed && back != content.data) {
                auto p = back;
                p --;
                if (!(Comp()(t_A.first, (*p).first) || Comp()((*p).first, t_A.first))) {
                    throw DuplicateException();
                }
            }
            std::copy_backward(back, content.data + content.size,
             content.data + content.size + 1);
            content.size ++;
            *back = t_A;
            if (Comp()(h.end, t_A.first)) {
                h.end = t_A.first;
            }
            if (Comp()(t_A.first, h.begin)) {
                h.begin = t_A.first;
            }
        }
        new_head.body = body_river.write(new_arr);
        h.next = head_river.write(new_head);
        head_river.update(h, idx);
        if (idx == head_end) {
            head_end = h.next;
            head_river.write_info(head_end, 4);
        }
        body_river.update(content, h.body);
    }
}

template<class T, class Comp, class Attachment>
bool RandomDB<T, Comp, Attachment>::erase(const T& t) {
    head_index idx = locate(t);
    if (idx == -1) {
        return false;
    }
    head h = get_head(idx);
    array content = get_body(h.body);
    auto bigger = 
    std::upper_bound(content.data, content.data + content.size, t, Comp_A());
    if (bigger==content.data) {
        return false;
    }
    auto p = bigger;
    p--;
    if (Comp_A()(t, *p) || Comp_A()(*p, t)) {
        return false;
    }
    bool update_head = false;
    if (bigger == content.data + content.size + 1) {
        h.end = content.data[content.size - 1].first;
        update_head = true;
    }
    if (p == content.data) {
        h.begin = content.data[0].first;
        update_head = true;
    }
    std::copy(bigger, content.data + content.size, bigger - 1);
    content.size --;
    if (content.size < array_size / 2) { // 合并空间，只能向前
        head_index next = h.next;
        if (next != -1) {
            head next_head = get_head(next);
            array next_content = get_body(next_head.body);
            if (next_content.size < array_size / 2) {
                h.next = next_head.next;
                next_head.deprecated = true;
                next_content.deprecated = true;
                std::copy(next_content.data, next_content.data + next_content.size, content.data + content.size);
                content.size += next_content.size;
                update_head = true;
                head_river.update(next_head, next);
                body_river.update(next_content, next_head.body);
                h.end = next_head.end;
            }
        }
    }
    if (update_head) {head_river.update(h, idx);}
    body_river.update(content, h.body);
    return true;
}

template<class T, class Comp, class Attachment>
std::pair<typename RandomDB<T, Comp, Attachment>::head_index, int>
RandomDB<T, Comp, Attachment>::upper_bound(const T& t) {
    head_index idx = locate(t);
    if (idx == -1) {
        return std::make_pair(head_begin, 0);
    }
    head h = get_head(idx);
    array content = get_body(h.body);
    auto bigger = 
    std::upper_bound(content.data, content.data + content.size, t, Comp_A());
    return std::make_pair(idx, bigger - content.data);
}

template<class T, class Comp, class Attachment>
std::pair<typename RandomDB<T, Comp, Attachment>::head_index, int> RandomDB<T, Comp, Attachment>::lower_bound(const T& t) {
    head_index idx = locate(t);
    if (idx == -1) {
        return std::make_pair(head_begin, 0);
    }
    head h = get_head(idx);
    array content = get_body(h.body);
    auto bigger = 
    std::lower_bound(content.data, content.data + content.size, t, Comp_A());
    return std::make_pair(idx, bigger - content.data);
}

template<class T, class Comp, class Attachment>
bool RandomDB<T, Comp, Attachment>::exist(const T& t) {
    head_index idx = locate(t);
    if (idx == -1) {
        return false;
    }
    head h = get_head(idx);
    array content = get_body(h.body);
    return std::binary_search(content.data, content.data + content.size, t, Comp_A());
}

template<class T, class Comp, class Attachment>
void RandomDB<T, Comp, Attachment>::enable_duplicate() {
    duplicate_allowed = true;
    head_river.write_info(1, 5);
}

template<class T, class Comp, class Attachment>
constexpr const int RandomDB<T, Comp, Attachment>::head::bin_size() {
    if constexpr (binable<T>) {
        return T::bin_size() * 2 + sizeof(head_index) + sizeof(arr_index) + sizeof(bool);
    }
    else {
        return sizeof(T) * 2 + sizeof(head_index) + sizeof(arr_index) + sizeof(int) + sizeof(bool);
    }
}

template<class T, class Comp, class Attachment>
char* RandomDB<T, Comp, Attachment>::head::to_bin() {
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
        ptr += sizeof(arr_index);
        std::copy(reinterpret_cast<char*>(&deprecated), reinterpret_cast<char*>(&deprecated) + sizeof(bool), ptr);
        return bin;
    }
    else {
        char* bin = new char[bin_size()];
        std::copy(reinterpret_cast<char*>(this), reinterpret_cast<char*>(this) + to_bin(), bin);
        return bin;
    }
}

template<class T, class Comp, class Attachment>
void RandomDB<T, Comp, Attachment>::head::from_bin(char* bin) {
    if constexpr (binable<T>) {
        char* ptr = bin;
        this->begin.from_bin(ptr);
        ptr += sizeofT;
        this->end.from_bin(ptr);
        ptr += sizeofT;
        std::copy(ptr, ptr + sizeof(head_index), reinterpret_cast<char*>(&next));
        ptr += sizeof(head_index);
        std::copy(ptr, ptr + sizeof(arr_index), reinterpret_cast<char*>(&body));
        ptr += sizeof(arr_index);
        std::copy(ptr, ptr + sizeof(bool), reinterpret_cast<char*>(&deprecated));
    }
    else {
        std::copy(bin, bin + sizeof(head), reinterpret_cast<char*>(this));
    }
}

template<class T, class Comp, class Attachment>
constexpr const int RandomDB<T, Comp, Attachment>::array::bin_size() {
    return sizeof(int) + sizeof(bool) + T_A_pair::bin_size() * array_size;
}

template<class T, class Comp, class Attachment>
char* RandomDB<T, Comp, Attachment>::array::to_bin() {
    char* bin = new char[bin_size()];
    char* ptr = bin;
    std::copy(reinterpret_cast<char*>(&size), reinterpret_cast<char*>(&size) + sizeof(int), ptr);
    ptr += sizeof(int);
    std::copy(reinterpret_cast<char*>(&deprecated), reinterpret_cast<char*>(&deprecated) + sizeof(bool), ptr);
    ptr += sizeof(bool);
    for (int i = 0; i < size; i++) {
        char* str_data = data[i].to_bin();
        std::copy(str_data, str_data + T_A_pair::bin_size(), ptr);
        ptr += T_A_pair::bin_size();
        delete[] str_data;
    }
    return bin;
}

template<class T, class Comp, class Attachment>
void RandomDB<T, Comp, Attachment>::array::from_bin(char* bin) {
    char* ptr = bin;
    std::copy(ptr, ptr + sizeof(int), reinterpret_cast<char*>(&size));
    ptr += sizeof(int);
    std::copy(ptr, ptr + sizeof(bool), reinterpret_cast<char*>(&deprecated));
    ptr += sizeof(bool);
    for (int i = 0; i < size; i++) {
        char* str_data = new char[T_A_pair::bin_size()];
        std::copy(ptr, ptr + T_A_pair::bin_size(), str_data);
        data[i].from_bin(str_data);
        delete[] str_data;
        ptr += T_A_pair::bin_size();
    }
}

template<class T, class Comp, class Attachment>
constexpr const int RandomDB<T, Comp, Attachment>::T_A_pair::bin_size() {
    if constexpr (std::is_same<Attachment, Empty>::value) {
        if constexpr (binable<T>) {
            return T::bin_size();
        }
        else {
            return sizeof(T);
        }
    }
    else {
        if constexpr (binable<T> && binable<Attachment>) {
            return T::bin_size() + Attachment::bin_size();
        }
        else if constexpr (binable<T> && (!binable<Attachment>)) {
            return T::bin_size() + sizeof(Attachment);
        }
        else if constexpr ((!binable<T>) && binable<Attachment>) {
            return sizeof(T) + Attachment::bin_size();
        }
        else {
            return sizeof(T) + sizeof(Attachment);
        }
    }
}

template<class T, class Comp, class Attachment>
void RandomDB<T, Comp, Attachment>::T_A_pair::from_bin(char* bin) {
    if constexpr (std::is_same<Attachment, Empty>::value) {
        if constexpr (binable<T>) {
            first.from_bin(bin);
        }
        else {
            std::copy(bin, bin + sizeofT, reinterpret_cast<char*>(&(this->first)));
        }
    }
    else {
        char* ptr = bin;
        if constexpr (binable<T>) {
            first.from_bin(ptr);
        }
        else {
            std::copy(bin, bin + sizeofT, reinterpret_cast<char*>(&(this->first)));
        }
        ptr += sizeofT;
        if constexpr (binable<Attachment>) {
            second.from_bin(ptr);
        }
        else {
            std::copy(ptr, ptr +  sizeofA, reinterpret_cast<char*>(&(this->second)));
        }
    }
}

template<class T, class Comp, class Attachment>
char* RandomDB<T, Comp, Attachment>::T_A_pair::to_bin() {
    if constexpr (std::is_same<Attachment, Empty>::value) {
        if constexpr (binable<T>) {
            return first.to_bin();
        }
        else {
            char* bin = new char[bin_size()];
            std::copy(reinterpret_cast<char*>(&first), reinterpret_cast<char*>(&first) + sizeofT, bin);
            return bin;
        }
    }
    else {
        char* bin = new char[bin_size()];
        char* ptr = bin;
        if constexpr (binable<T>) {
            char* data = first.to_bin();
            std::copy(data, data + sizeofT, ptr);
            delete[] data;
        }
        else {
            std::copy(reinterpret_cast<char*>(&first), reinterpret_cast<char*>(&first) + sizeofT, ptr); 
        }
        ptr += sizeofT;
        if constexpr (binable<Attachment>) {
            char* data2 = second.to_bin();
            std::copy(data2, data2 + sizeofA, ptr);
            delete[] data2;
        }
        else {
            std::copy(reinterpret_cast<char*>(&second), reinterpret_cast<char*>(&second) + sizeofA, ptr);
        }
        return bin;
    }
}

template<class T, class Comp, class Attachment>
std::vector<typename RandomDB<T, Comp, Attachment>::T_A_pair> RandomDB<T, Comp, Attachment>::find(const T& t) {
    auto [idx_l, pos_l] = lower_bound(t);
    auto [idx_r, pos_r] = upper_bound(t);
}

template<class T, class Comp, class Attachment>
std::vector<typename RandomDB<T, Comp, Attachment>::T_A_pair> RandomDB<T, Comp, Attachment>::range(head_index lh, int lp, head_index rh, int rp) {
    /**
        * @brief [ [lh, lp], [rh, rp] )
        * @note 如果rh == -1, rp == 0，则表示到最后一个元素，如果lh == -1, lp == 0，则为空
     */
    auto idx = lh;
    auto pos = lp;
    std::vector<T_A_pair> res;
    if (idx == -1) {
        return res;
    }
    head h = get_head(idx);
    array content = get_body(h.body);
    while (idx != rh || pos != rp) {
        if (pos == content.size) {
            idx = h.next;
            if (idx == -1) {
                break;
            }
            h = get_head(idx);
            content = get_body(h.body);
            pos = 0;
            continue;
        }
        res.push_back(content.data[pos]);
        pos ++;
    }
    return res;
}
