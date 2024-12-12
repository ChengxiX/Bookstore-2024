#ifndef _block_river_hpp_
#define _block_river_hpp_

#include <exception>
#include <fstream>
#include "binable.hpp"

#include <iostream>

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

template<class T, int block_size = 4096>
class BlockRiver {
private:
    struct InfoOverflow : std::exception {};
    fstream file;
    string file_name;
    constexpr static const int sizeofT_() {
        if constexpr (binable<T>) {
            return T::bin_size();
        }
        else {
            return sizeof(T);
        }
    }
    static constexpr const int sizeofT = sizeofT_();
    static_assert(sizeofT <= block_size);
    unsigned int empty_tail_count = 0;
public:
    struct BiggerThanABlock : std::exception {};
    BlockRiver() = default;
    
    BlockRiver(const string& file_name) {
        this->bind(file_name);
    }

    void set_empty_tail(int n) {
        empty_tail_count = n;
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
        file.seekp(std::ios::beg);
        for (int i = 0; i < block_size; ++i) // 留出一个头块来存info
            file.put(0);
        file.close();
        this->bind(file_name);
    }

    //读出第n个int的值赋给tmp，1_base
    void get_info(int &tmp, int n) {
        if (n > block_size/sizeof(int)) throw InfoOverflow();
        file.seekg((n - 1) * sizeof(int));
        file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
    }

    //将tmp写入第n个int的位置，1_base
    void write_info(int tmp, int n) {
        if (n > block_size/sizeof(int)) throw InfoOverflow();
        file.seekp((n - 1) * sizeof(int));
        file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
    }

    //在文件合适位置写入类对象t，并返回写入的位置索引index
    //位置索引意味着当输入正确的位置索引index，在以下三个函数中都能顺利的找到目标对象进行操作
    //位置索引index可以取为对象写入的起始位置
    int write(T &t) {
        file.seekp(-(empty_tail_count * block_size), std::ios::end);
        if (empty_tail_count) { // != 0
            empty_tail_count--;
        }
        int init = file.tellp();
        if (init == -1) {throw std::exception();}
        if constexpr (binable<T>) {
            char* data = t.to_bin();
            file.seekp(init + block_size - 1);
            file.put(0);
            file.seekp(init);
            file.write(data, sizeofT);
            delete []data;
        }
        else {
            file.seekp(init + block_size - 1);
            file.put(0);
            file.seekp(init);
            file.write(reinterpret_cast<const char *>(&t), sizeofT);
        }
        return init;
    }

    //用t的值更新位置索引index对应的对象，保证调用的index都是由write函数产生
    void update(T &t, const int index) {
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

    //删除最后的n个对象
    void Delete(int count) {
        empty_tail_count += count;
    }
};


#endif