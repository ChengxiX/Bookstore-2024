#ifndef BPT_MEMORYRIVER_HPP
#define BPT_MEMORYRIVER_HPP

#include <fstream>
#include "binable.hpp"

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

template<class T, int info_len = 2>
class MemoryRiver {
private:
    /* your code here */
    struct InfoOverflow : std::exception {};
    fstream file;
    string file_name;
    static const int sizeofT = binable<T> ? T::bin_size() : sizeof(T);
public:
    MemoryRiver() = default;
    
    MemoryRiver(const string& file_name) {
        this->bind(file_name);
    }

    void bind(const string& file_name) {
        if (file.is_open()) file.close();
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
        this->file_name = file_name;
    }

    ~MemoryRiver() {
        close();
    }

    void close() {
        file.close();
    }

    void initialise(string FN = "") {
        if (FN != "") file_name = FN;
        if (file.is_open()) file.close();
        file.open(file_name, std::ios::out | std::ios::binary);
        int tmp = 0;
        file.seekp(std::ios::beg);
        for (int i = 0; i < info_len; ++i)
            file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
        file.close();
        this->bind(file_name);
    }

    //读出第n个int的值赋给tmp，1_base
    void get_info(int &tmp, int n) {
        if (n > info_len) throw InfoOverflow();
        /* your code here */
        file.seekg((n - 1) * sizeof(int));
        file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
    }

    //将tmp写入第n个int的位置，1_base
    void write_info(int tmp, int n) {
        if (n > info_len) throw InfoOverflow();
        /* your code here */
        file.seekp((n - 1) * sizeof(int));
        file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
    }

    //在文件合适位置写入类对象t，并返回写入的位置索引index
    //位置索引意味着当输入正确的位置索引index，在以下三个函数中都能顺利的找到目标对象进行操作
    //位置索引index可以取为对象写入的起始位置
    int write(T &t) {
        /* your code here */
        file.seekp(0, std::ios::end);
        int index = file.tellp();
        if constexpr (binable<T>) {
            char* data = t.to_bin();
            file.write(data, sizeofT);
        } else {
            file.write(reinterpret_cast<const char *>(&t), sizeofT);
        }
        return index;
    }

    //用t的值更新位置索引index对应的对象，保证调用的index都是由write函数产生
    void update(T &t, const int index) {
        /* your code here */
        file.seekp(index);
        if constexpr (binable<T>) {
            char* data = t.to_bin();
            file.write(data, sizeofT);
        } else {
            file.write(reinterpret_cast<const char *>(&t), sizeofT);
        }
    }

    //读出位置索引index对应的T对象的值并赋值给t，保证调用的index都是由write函数产生
    void read(T &t, int index) {
        /* your code here */
        file.seekg(index);
        if constexpr (binable<T>) {
            char *data = new char[sizeofT];
            file.read(data, sizeofT);
            t.from_bin(data);
            delete []data;
        } else {
            file.read(reinterpret_cast<char *>(&t), sizeofT);
        }
    }

    //删除位置索引index对应的对象(不涉及空间回收时，可忽略此函数)，保证调用的index都是由write函数产生
    void Delete(int index) {
        /* your code here */
    }
};


#endif //BPT_MEMORYRIVER_HPP