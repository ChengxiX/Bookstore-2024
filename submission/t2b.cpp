#ifndef binable_hpp
#define binable_hpp

#include <concepts>

template<class T>
concept to_bin = requires(T t) {
    { t.to_bin() } -> std::same_as<char*>;
};
template<class T>
concept from_bin = requires(T t, char* s) {
    { t.from_bin(s) } -> std::same_as<void>;
};
template<class T>
concept bin_size = requires(T t) {
    { t.bin_size() } -> std::same_as<int>;
};
template<class T>
concept binable = to_bin<T> && from_bin<T> && bin_size<T>;

#endif

#ifndef BPT_MEMORYRIVER_HPP
#define BPT_MEMORYRIVER_HPP

#include <fstream>

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

template<class T, int info_len = 2>
class MemoryRiver {
private:
    /* your code here */
    fstream file;
    string file_name;
    static const int sizeofT = binable<T> ? T::bin_size() : sizeof(T);
public:
    MemoryRiver() = default;
    
    MemoryRiver(const string& file_name) {
        this->bind(file_name);
    }

    void bind(const string& file_name) {
        if (file.is_open()) file.close();
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
        this->file_name = file_name;
    }

    ~MemoryRiver() {
        close();
    }

    void close() {
        file.close();
    }

    void initialise(string FN = "") {
        if (FN != "") file_name = FN;
        if (file.is_open()) file.close();
        file.open(file_name, std::ios::out | std::ios::binary);
        int tmp = 0;
        file.seekp(std::ios::beg);
        for (int i = 0; i < info_len; ++i)
            file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
        file.close();
        this->bind(file_name);
    }

    //读出第n个int的值赋给tmp，1_base
    void get_info(int &tmp, int n) {
        if (n > info_len) return;
        /* your code here */
        file.seekg((n - 1) * sizeof(int));
        file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
    }

    //将tmp写入第n个int的位置，1_base
    void write_info(int tmp, int n) {
        if (n > info_len) return;
        /* your code here */
        file.seekp((n - 1) * sizeof(int));
        file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
    }

    //在文件合适位置写入类对象t，并返回写入的位置索引index
    //位置索引意味着当输入正确的位置索引index，在以下三个函数中都能顺利的找到目标对象进行操作
    //位置索引index可以取为对象写入的起始位置
    int write(const T &t) {
        /* your code here */
        file.seekp(0,std::ios::end);
        int index = file.tellp();
        if constexpr (binable<T>) {
            char* data = t.to_bin();
            file.write(data, sizeofT);
        } else {
            file.write(reinterpret_cast<const char *>(&t), sizeofT);
        }
        return index;
    }

    //用t的值更新位置索引index对应的对象，保证调用的index都是由write函数产生
    void update(const T &t, const int index) {
        /* your code here */
        file.seekp(index);
        if constexpr (binable<T>) {
            char* data = t.to_bin();
            file.write(data, sizeofT);
        } else {
            file.write(reinterpret_cast<const char *>(&t), sizeofT);
        }
    }

    //读出位置索引index对应的T对象的值并赋值给t，保证调用的index都是由write函数产生
    void read(T &t, const int index) {
        /* your code here */
        file.seekg(index);
        if constexpr (binable<T>) {
            char *data = new char[sizeofT];
            file.read(data, sizeofT);
            t.from_bin(data);
            delete []data;
        } else {
            file.read(reinterpret_cast<char *>(&t), sizeofT);
        }
    }

    //删除位置索引index对应的对象(不涉及空间回收时，可忽略此函数)，保证调用的index都是由write函数产生
    void Delete(int index) {
        /* your code here */
    }
};


#endif //BPT_MEMORYRIVER_HPP

#ifndef _block_river_hpp_
#define _block_river_hpp_

#include <exception>
#include <fstream>

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

template<class T, int block_size = 4096>
class BlockRiver {
private:
    /* your code here */
    fstream file;
    string file_name;
    static const int sizeofT = binable<T> ? T::bin_size() : sizeof(T);;
public:
    struct BiggerThanABlock : std::exception {};
    BlockRiver() = default;
    
    BlockRiver(const string& file_name) {
        this->bind(file_name);
    }

    void bind(const string& file_name) {
        if (file.is_open()) file.close();
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
        this->file_name = file_name;
    }

    ~BlockRiver() {
        close();
    }

    void close() {
        file.close();
    }

    void initialise(string FN = "") {
        if (FN != "") file_name = FN;
        if (file.is_open()) file.close();
        file.open(file_name, std::ios::out | std::ios::binary);
        file.close();
        this->bind(file_name);
    }

    //在文件合适位置写入类对象t，并返回写入的位置索引index
    //位置索引意味着当输入正确的位置索引index，在以下三个函数中都能顺利的找到目标对象进行操作
    //位置索引index可以取为对象写入的起始位置
    int write(const T &t) {
        /* your code here */
        file.seekp(0,std::ios::end);
        int index = file.tellp();
        if constexpr (binable<T>) {
            char* data = t.to_bin();
            file.write(data, sizeofT);
            for (int i = 0; i < 4096-sizeofT; i++) {
                file.put('\0');
            }
            delete []data;
        }
        else {
            file.write(reinterpret_cast<const char *>(&t), sizeofT);
            for (int i = 0; i < 4096-sizeofT; i++) {
                file.put('\0');
            }
        }
        return index;
    }

    //用t的值更新位置索引index对应的对象，保证调用的index都是由write函数产生
    void update(const T &t, const int index) {
        /* your code here */
        file.seekp(index);
        if constexpr (binable<T>) {
            char* data = t.to_bin();
            file.write(data, T::bin_size());
            delete []data;
        }
        else {
            file.write(reinterpret_cast<const char *>(&t), sizeofT);
        }
    }

    //读出位置索引index对应的T对象的值并赋值给t，保证调用的index都是由write函数产生
    void read(T &t, const int index) {
        /* your code here */
        file.seekg(index);
        if constexpr (binable<T>) {
            char *data = new char[sizeofT];
            file.read(data, sizeofT);
            t.from_bin(data);
            delete []data;
        }
        else {
            file.read(reinterpret_cast<char *>(&t), sizeofT);
        }
    }

    //删除位置索引index对应的对象(不涉及空间回收时，可忽略此函数)，保证调用的index都是由write函数产生
    void Delete(int index) {
        /* your code here */
    }
};


#endif

#ifndef _random_db
#define _random_db

#include <functional>
#include <string>
#include <vector>

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
    arr_index c = body_river.write(array{{t}, 1});
    if (prev != -1) {
        auto p = get_head(prev);
        p.next = head_river.write(head{t.first, t.first, p.next, c});
        head_river.update(p, prev);
        head_end = p.next;
        head_river.write_info(head_end, 4);
        return p.next;
    }
    else {
        head_begin = head_river.write(head{t.first, t.first, -1, c});
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
        if (!duplicate_allowed) {
            auto p = back;
            p --;
            if (!(Comp()(t_A.first, (*p).first) || Comp()((*p).first, t_A.first))) { // == !(Comp()(t, *p) || Comp()(*p, t))
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
            std::copy_backward(back, content.data + content.size,
             content.data + content.size + 1);
            content.size ++;
            *back = t_A;
            if (Comp()(h.end, t_A.first)) {
                h.end = t_A.first;
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
    std::copy(bigger, content.data + content.size, bigger - 1);
    content.size --;
    if (bigger == content.data + content.size + 1) {
        h.end = content.data[content.size - 1].first;
        head_river.update(h, idx);
    }
    if (p == content.data) {
        h.begin = content.data[0].first;
        head_river.update(h, idx);
    }
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
        return T::bin_size() * 2 + sizeof(head_index) + sizeof(arr_index);
    }
    else {
        return sizeof(T) * 2 + sizeof(head_index) + sizeof(arr_index) + sizeof(int);
    }
}

template<class T, class Comp, class Attachment>
const char* RandomDB<T, Comp, Attachment>::head::to_bin() {
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
        char* bin = new char[bin_size()];
        std::copy(reinterpret_cast<char*>(this), reinterpret_cast<char*>(this) + to_bin(), bin);
        return bin;
    }
}

template<class T, class Comp, class Attachment>
const void RandomDB<T, Comp, Attachment>::head::from_bin(char* bin) {
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

template<class T, class Comp, class Attachment>
constexpr const int RandomDB<T, Comp, Attachment>::array::bin_size() {
    return sizeof(int) + sizeof(bool) + T_A_pair::bin_size() * array_size;
}

template<class T, class Comp, class Attachment>
const char* RandomDB<T, Comp, Attachment>::array::to_bin() {
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
const void RandomDB<T, Comp, Attachment>::array::from_bin(char* bin) {
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
const void RandomDB<T, Comp, Attachment>::T_A_pair::from_bin(char* bin) {
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
const char* RandomDB<T, Comp, Attachment>::T_A_pair::to_bin() {
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

#ifndef _kvdb_hpp_
#define _kvdb_hpp_

#include <cstring>
#include <string>
#include <vector>
#include <exception>

template<class VT, class Comp = std::less<VT>, int key_name_len = 64>
class KVDB {
    struct KeyTooLongException : std::exception {};
    using key_type = char[key_name_len];
    struct kv_pair {
        key_type key;
        VT value;
        static const int bin_size();
        const char* to_bin();
        const void from_bin(char* bin);
    };
    struct kv_pair_comp {
        bool operator()(const kv_pair& a, const kv_pair& b) const {
            int res = std::strcmp(a.key, b.key);
            if (res == 0) {
                return Comp()(a.value, b.value);
            }
            return res < 0;
        }
    };
    RandomDB<kv_pair, kv_pair_comp> db;
public:
    KVDB(std::string dbname, int db_id = 0, std::string path = "", bool duplicate_allowe = false);
    // ~KVDB() {if (db) delete db;}
    void insert(std::string k, int v);
    void erase(std::string k, int v);
    std::vector<std::pair<typename KVDB<VT, Comp, key_name_len>::key_type, VT>> find(std::string k);
};

#endif

#include <cstring>
#include <utility>


template<class VT, class Comp, int key_name_len>
const int KVDB<VT, Comp, key_name_len>::kv_pair::bin_size() {
    if constexpr (binable<VT>) {
        return key_name_len + VT::bin_size();
    } else {
        return key_name_len + sizeof(VT);
    }
}

template<class VT, class Comp, int key_name_len>
const char* KVDB<VT, Comp, key_name_len>::kv_pair::to_bin() {
    char* bin = new char[bin_size()];
    memcpy(bin, key.c_str, key_name_len);
    if constexpr (binable<VT>) {
        value.to_bin(bin + key_name_len);
    } else {
        memcpy(bin + key_name_len, &value, sizeof(VT));
    }
    return bin;
}

template<class VT, class Comp, int key_name_len>
const void KVDB<VT, Comp, key_name_len>::kv_pair::from_bin(char* bin) {
    memcpy(key, bin, key_name_len);
    if constexpr (binable<VT>) {
        value.from_bin(bin + key_name_len);
    } else {
        memcpy(&value, bin + key_name_len, sizeof(VT));
    }
}

template<class VT, class Comp, int key_name_len>
KVDB<VT, Comp, key_name_len>::KVDB(std::string dbname, int db_id, std::string path, bool duplicate_allowed) : db(dbname, db_id, path, duplicate_allowed) {
}

template<class VT, class Comp, int key_name_len>
void KVDB<VT, Comp, key_name_len>::insert(std::string k, int v) {
    if (k.length() >= key_name_len) {
        throw KeyTooLongException();
    }
    kv_pair p;
    strcpy(p.key, k.c_str());
    p.value = v;
    db.insert(p);
}

template<class VT, class Comp, int key_name_len>
void KVDB<VT, Comp, key_name_len>::erase(std::string k, int v) {
    if (k.length() >= key_name_len) {
        throw KeyTooLongException();
    }
    kv_pair p;
    strcpy(p.key, k.c_str());
    p.value = v;
    db.erase(p);
}

template<class VT, class Comp, int key_name_len>
std::vector<std::pair<typename KVDB<VT, Comp, key_name_len>::key_type, VT>> KVDB<VT, Comp, key_name_len>::find(std::string k) {
    if (k.length() >= key_name_len) {
        throw KeyTooLongException();
    }
    kv_pair p;
    strcpy(p.key, k.c_str());
    std::vector<std::pair<key_type, VT>> res;
    p.value = 0;
    auto l = db.lower_bound(p);
    p.value = 2147483647;
    auto r = db.upper_bound(p);
    auto call = db.range(l.first, l.second, r.first, r.second);
    res.resize(call.size());
    for (int i = 0; i < call.size(); i++) {
        res[i].second = call[i].first.value;
        strcpy(res[i].first, call[i].first.key);
    }
    return res;
}

#include <vector>
#include <iostream>

int main() {
    std::string op;
    int n;
    std::cin >> n;
    KVDB<int, std::less<int>> db("db2");
    for (int i = 0; i < n; i++) {
        std::cin >> op;
        if (op == "insert") {
            std::string key;
            int value;
            std::cin >> key >> value;
            db.insert(key, value);
        }
        else if (op == "delete") {
            std::string key;
            int value;
            std::cin >> key >> value;
            db.erase(key, value);
        }
        else if (op == "find") {
            std::string key;
            std::cin >> key;
            auto res = db.find(key);
            for (auto i : res) {
                std::cout << i.second << ' ';
            }
            if (res.empty()) {
                std::cout << "null";
            }
            std::cout << '\n';
        }
    }
}