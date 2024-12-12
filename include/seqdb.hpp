#ifndef SEQDB_HPP
#define SEQDB_HPP

#include "binable.hpp"
#include "BlockRiver.hpp"
#include <exception>
#include <vector>

template<class T, int block_size = 4096>
class SeqDB {
    class DBFileNotMatchException : std::exception {};
    struct FileBroken : std::exception {};
    using index = int;
    constexpr static const int sizeofT_() {
        if constexpr (binable<T>) {
            return T::bin_size();
        }
        else {
            return sizeof(T);
        }
    }
    static constexpr const int sizeofT = sizeofT_();
    constexpr static const int array_size = block_size / sizeofT;
    struct array {
        T data[array_size];
        char* to_bin();
        void from_bin(char* bin);
        constexpr static const int bin_size();
    };
    BlockRiver<array, block_size> river;
    int len_ = 0;
public:
    int size() {return len_;}
    SeqDB(std::string dbname, int db_id = 0, std::string path = "");
    ~SeqDB();
    void push_back(T &t);
    void push_back(const T &t);
    void resize(int size);
    void update(T &t, const index index);
    void update(const T &t, const index index);
    T operator[](const index index);
    T at(const index index);
    std::vector<T> range(index l, index r);
    void pop();
};

#endif