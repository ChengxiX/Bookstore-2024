#include "../include/kvdb.hpp"
#include <exception>
#include <cstring>

class DBFileNotMatchException : std::exception {};

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
        if (db_id!=-1 && db_id != id) {
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
        if (!Comp()(tmp.key, key)) {
            if (!Comp()(key, tmp.key)) {
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
        node new_node = node{-1, -1, key, true};
        begin_key = db_file.write(new_node);
        return begin_key;
    }
    index idx = begin_key;
    index pre = -1;
    while (idx != -1) {
        node tmp = get(idx);
        if (strcmp(tmp.key, key.c_str()) < 0) {
            pre = idx;
            idx = tmp.right;
        }
        else {
            if (!strcmp(key, tmp.key.c_str) < 0) {
                return idx;
            }
            break;
        }
    }
    if (pre == -1) {
        node new_node = node{-1, begin_key, key, true};
        begin_key = db_file.write(new_node);
        return begin_key;
    }
    node new_node = node{-1, idx, key, true};
    node pre_node = get(pre);
    pre_node.right = db_file.write(new_node);
    db_file.update(pre_node, pre);
    return pre_node.right;
}

template<class VT, class Comp, int key_name_len>
typename KVDB<VT, Comp, key_name_len>::index KVDB<VT, Comp, key_name_len>::Insert(const std::string & key, const VT & value, const index _index) {
    if (key.size() > key_name_len) {
        throw KeyNameException();
    }
    if (_index != -1) {
        index pre = -1;
        
                
    }
