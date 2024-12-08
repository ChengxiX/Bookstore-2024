#include <fstream>
#include <string>
#include <vector>
#include <exception>
#include "MemoryRiver.hpp"

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
        index Insert(const std::string & key, const VT & value, const index _index = -1);
        index Del(const std::string & key, const VT & value, const index _index = -1);
        std::pair<index, std::vector<VT>> Find(const std::string & key, const index _index = -1);
        void EnableReuse();
        void DisableReuse();
        class KeyNameException;
        class KVDuplicateException;
        class DBFileNotMatchException;
};