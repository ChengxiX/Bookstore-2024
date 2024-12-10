#ifndef _block_river_hpp_
#define _block_river_hpp_

#include <exception>
#include <fstream>
#include "binable.hpp"

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