#ifndef _kvdb_hpp_
#define _kvdb_hpp_

#include <cstring>
#include <vector>
#include "randomdb.hpp"

template<class VT, class Comp = std::less<VT>, int key_name_len = 64>
class KVDB {
    using key_type = char[key_name_len];
    struct kv_pair {
        key_type key;
        VT value;
        static const int bin_size();
        const void to_bin(char* bin);
        const void from_bin(char* bin);
    };
    struct kv_pair_comp {
        bool operator()(const kv_pair& a, const kv_pair& b) const {
            int res = strcmp(a, b);
            if (res == 0) {
                return Comp()(a.value, b.value);
            }
            return res < 0;
        }
    };
    RandomDB<kv_pair, Comp> *db = nullptr;
    ~KVDB() {if (db) delete db;}
    
    
};

#endif