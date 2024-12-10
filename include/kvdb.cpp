#include "kvdb.hpp"
#include "binable.hpp"


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
std::vector<VT> KVDB<VT, Comp, key_name_len>::find(std::string k) {
    if (k.length() >= key_name_len) {
        throw KeyTooLongException();
    }
    kv_pair p;
    strcpy(p.key, k.c_str());
    std::vector<VT> res;
    p.value = 0;
    auto l = db.lower_bound(p);
    p.value = 2147483647;
    auto r = db.upper_bound(p);
    auto call = db.range(l.first, l.second, r.first, r.second);
    for (int i = 0; i < call.size(); i++) {
        res.push_back(call[i].first.value);
    }
    return res;
}

