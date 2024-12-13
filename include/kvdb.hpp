#ifndef _kvdb_hpp_
#define _kvdb_hpp_

#include <cstring>
#include <string>
#include <vector>
#include <exception>
#include "randomdb.hpp"

template<class VT, class Comp = std::less<VT>, int key_name_len = 64>
class KVDB {
    struct KeyTooLongException : std::exception {};
    using key_type = char[key_name_len];
    struct kv_pair {
        key_type key;
        VT value;
        constexpr static const int bin_size();
        char* to_bin();
        void from_bin(char* bin);
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