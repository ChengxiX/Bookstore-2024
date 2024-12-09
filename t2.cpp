#include <exception>
#include <cstring>
#include <vector>
#include <fstream>
#include <string>

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
    int sizeofT = sizeof(T);
public:
    MemoryRiver() = default;

    MemoryRiver(const string& file_name) : file_name(file_name) {}

    void initialise(string FN = "") {
        if (FN != "") file_name = FN;
        file.open(file_name, std::ios::out | std::ios::binary);
        int tmp = 0;
        for (int i = 0; i < info_len; ++i)
            file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
        file.close();
    }

    //读出第n个int的值赋给tmp，1_base
    void get_info(int &tmp, int n) {
        if (n > info_len) return;
        /* your code here */
        file.open(file_name, std::ios::in | std::ios::binary);
        file.seekg((n - 1) * sizeof(int));
        file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
        file.close();
    }

    //将tmp写入第n个int的位置，1_base
    void write_info(int tmp, int n) {
        if (n > info_len) return;
        /* your code here */
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp((n - 1) * sizeof(int));
        file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
        file.close();
    }

    //在文件合适位置写入类对象t，并返回写入的位置索引index
    //位置索引意味着当输入正确的位置索引index，在以下三个函数中都能顺利的找到目标对象进行操作
    //位置索引index可以取为对象写入的起始位置
    int write(T &t) {
        /* your code here */
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(0,std::ios::end);
        int index = file.tellp();
        file.write(reinterpret_cast<char *>(&t), sizeofT);
        file.close();
        return index;
    }

    //用t的值更新位置索引index对应的对象，保证调用的index都是由write函数产生
    void update(T &t, const int index) {
        /* your code here */
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(index);
        file.write(reinterpret_cast<char *>(&t), sizeofT);
        file.close();
    }

    //读出位置索引index对应的T对象的值并赋值给t，保证调用的index都是由write函数产生
    void read(T &t, const int index) {
        /* your code here */
        file.open(file_name, std::ios::in | std::ios::binary);
        file.seekg(index);
        file.read(reinterpret_cast<char *>(&t), sizeofT);
        file.close();
    }

    //删除位置索引index对应的对象(不涉及空间回收时，可忽略此函数)，保证调用的index都是由write函数产生
    void Delete(int index) {
        /* your code here */
    }
};

template<class VT, class Comp, int key_name_len = 64>
class KVDB {
    using index = int;
    struct node {
        index down = -1;
        index right = -1;
        char key[key_name_len];
        bool invalid = false;
        VT value;
    };
    index locate(const std::string & key);
    index addKey(const std::string & key);
    node get(index idx);
    MemoryRiver<node, 2> db_file;
    int db_id;
    index begin_key = -1;
    public:
        bool _reuse_ = false;
        KVDB(const std::string & db_file_name, int db_id = -1);
        ~KVDB();
        index Insert(const std::string & key, const VT & value, index _index = -1);
        index Del(const std::string & key, const VT & value, index _index = -1);
        std::vector<VT> Find(const std::string & key, index _index = -1);
        void EnableReuse();
        void DisableReuse();
        class KeyNameException;
        class KVDuplicateException;
        class DBFileNotMatchException;
};

template<class VT, class Comp, int key_name_len>
class KVDB<VT, Comp, key_name_len>::DBFileNotMatchException : std::exception {};

template<class VT, class Comp, int key_name_len>
class KVDB<VT, Comp, key_name_len>::KeyNameException : std::exception {};

template<class VT, class Comp, int key_name_len>
class KVDB<VT, Comp, key_name_len>::KVDuplicateException : std::exception {};

template<class VT, class Comp, int key_name_len>
KVDB<VT, Comp, key_name_len>::KVDB(const std::string & db_file_name, int db_id){
    std::ifstream file(db_file_name);
    if (!file) {
        db_file.initialise(db_file_name);
        db_file.write_info(begin_key, 2);
        db_file.write_info(db_id, 1);
        this->db_id = db_id;
    }
    else {
        file.close();
        db_file = MemoryRiver<node, 2>(db_file_name);
        int id;
        db_file.get_info(id, 2);
        if (db_id != -1 && db_id != id) {
            throw DBFileNotMatchException();
        }
        this->db_id = id;
    }
    db_file.get_info(begin_key, 2);
}

template<class VT, class Comp, int key_name_len>
KVDB<VT, Comp, key_name_len>::~KVDB() {
    db_file.write_info(begin_key, 2);
}

template<class VT, class Comp, int key_name_len>
typename KVDB<VT, Comp, key_name_len>::node KVDB<VT, Comp, key_name_len>::get(index idx) {
    node tmp;
    db_file.read(tmp, idx);
    return tmp;
}

template<class VT, class Comp, int key_name_len>
typename KVDB<VT, Comp, key_name_len>::index KVDB<VT, Comp, key_name_len>::locate(const std::string & key) {
    index idx = begin_key;
    while (idx != -1) {
        node tmp = get(idx);
        if (!(std::strcmp(tmp.key, key.c_str()) < 0)) {
            if (!(std::strcmp(key.c_str(), tmp.key) < 0)) {
                return idx;
            }
            return -1;
        }
        idx = tmp.right;
    }
    return -1;
}

template<class VT, class Comp, int key_name_len>
typename KVDB<VT, Comp, key_name_len>::index KVDB<VT, Comp, key_name_len>::addKey(const std::string & key) {
    if (begin_key == -1) {
        node new_node = node{-1, -1};
        strcpy(new_node.key, key.c_str());
        new_node.invalid = true;
        begin_key = db_file.write(new_node);
        return begin_key;
    }
    index idx = begin_key;
    index pre = -1;
    while (idx != -1) {
        node tmp = get(idx);
        if (std::strcmp(tmp.key, key.c_str()) < 0) {
            pre = idx;
            idx = tmp.right;
        }
        else {
            if (!(std::strcmp(key.c_str(), tmp.key) < 0)) {
                return idx;
            }
            break;
        }
    }
    if (pre == -1) {
        node new_node = node{-1, begin_key};
        strcpy(new_node.key, key.c_str());
        new_node.invalid = true;
        begin_key = db_file.write(new_node);
        return begin_key;
    }
    node new_node = node{-1, idx};
    strcpy(new_node.key, key.c_str());
    new_node.invalid = true;
    node pre_node = get(pre);
    pre_node.right = db_file.write(new_node);
    db_file.update(pre_node, pre);
    return pre_node.right;
}

template<class VT, class Comp, int key_name_len>
typename KVDB<VT, Comp, key_name_len>::index KVDB<VT, Comp, key_name_len>::Insert(const std::string & key, const VT & value, index _index) {
    if (key.size() > key_name_len) {
        throw KeyNameException();
    }
    if (_index == -1) {
        _index = locate(key);
    }
    if (_index == -1) {
        _index = addKey(key);
    }
    index pre = _index;
    index idx = get(pre).down;
    while (idx != -1) {
        node tmp = get(idx);
        if (Comp()(tmp.value, value) || tmp.invalid) {
            pre = idx;
            idx = tmp.down;
        }
        else {
            if (!Comp()(value, tmp.value)) {
                throw KVDuplicateException();
            }
            break;
        }
    }
    node new_node = node{idx, -1};
    strcpy(new_node.key, key.c_str());
    new_node.value = value;
    new_node.invalid = false;
    index np = db_file.write(new_node);
    node pre_node = get(pre);
    pre_node.down = np;
    db_file.update(pre_node, pre);
    return np;
}

template<class VT, class Comp, int key_name_len>
typename KVDB<VT, Comp, key_name_len>::index KVDB<VT, Comp, key_name_len>::Del(const std::string & key, const VT & value, index _index) {
    if (key.size() > key_name_len) {
        throw KeyNameException();
    }
    if (_index == -1) {
        _index = locate(key);
    }
    if (_index == -1) {
        return -1;
    }
    index pre = _index;
    index idx = get(pre).down;
    while (idx != -1) {
        node tmp = get(idx);
        if (Comp()(tmp.value, value) || tmp.invalid) {
            pre = idx;
            idx = tmp.down;
        }
        else {
            if (!Comp()(value, tmp.value)) {
                tmp.invalid = true;
                db_file.update(tmp, idx);
                return idx;
            }
            break;
        }
    }
    return -1;
}

template <class VT, class Comp, int key_name_len>
std::vector<VT> KVDB<VT, Comp, key_name_len>::Find(const std::string & key, index _index) {
    if (key.size() > key_name_len) {
        throw KeyNameException();
    }
    std::vector<VT> res;
    if (_index == -1) {
        _index = locate(key);
    }
    if (_index == -1) {
        return res;
    }
    index idx = get(_index).down;
    while (idx != -1) {
        node tmp = get(idx);
        if (!tmp.invalid) {
            res.push_back(tmp.value);
        }
        idx = tmp.down;
    }
    return res;
}

template <class VT, class Comp, int key_name_len>
void KVDB<VT, Comp, key_name_len>::EnableReuse() {
    _reuse_ = true;
}

template <class VT, class Comp, int key_name_len>
void KVDB<VT, Comp, key_name_len>::DisableReuse() {
    _reuse_ = false;
}

#include <iostream>

int main() {
    std::string op;
    int n;
    std::cin >> n;
    KVDB<int, std::less<int>> db("db");
    for (int i = 0; i < n; i++) {
        std::cin >> op;
        if (op == "insert") {
            std::string key;
            int value;
            std::cin >> key >> value;
            db.Insert(key, value);
        }
        else if (op == "delete") {
            std::string key;
            int value;
            std::cin >> key >> value;
            db.Del(key, value);
        }
        else if (op == "find") {
            std::string key;
            std::cin >> key;
            std::vector<int> res = db.Find(key);
            for (int i : res) {
                std::cout << i << ' ';
            }
            if (res.empty()) {
                std::cout << "null";
            }
            std::cout << '\n';
        }
    }
}