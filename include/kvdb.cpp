#include "kvdb.hpp"
#include <exception>
#include <cstring>
#include <vector>
#include <filesystem>

template<class VT, class Comp, int key_name_len>
class KVDB<VT, Comp, key_name_len>::DBFileNotMatchException : std::exception {};

template<class VT, class Comp, int key_name_len>
class KVDB<VT, Comp, key_name_len>::KeyNameException : std::exception {};

template<class VT, class Comp, int key_name_len>
class KVDB<VT, Comp, key_name_len>::KVDuplicateException : std::exception {};

template<class VT, class Comp, int key_name_len>
KVDB<VT, Comp, key_name_len>::KVDB(const std::string & db_file_name, int db_id){
    if (!std::filesystem::exists(db_file_name)) {
        db_file.initialise(db_file_name);
        db_file.write_info(begin_key, 2);
        db_file.write_info(db_id, 1);
        this->db_id = db_id;
    }
    else {
        db_file.bind(db_file_name);
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
    db_file.close();
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
        std::strcpy(new_node.key, key.c_str());
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
        std::strcpy(new_node.key, key.c_str());
        new_node.invalid = true;
        begin_key = db_file.write(new_node);
        return begin_key;
    }
    node new_node = node{-1, idx};
    std::strcpy(new_node.key, key.c_str());
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
    std::strcpy(new_node.key, key.c_str());
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