#include "seqdb.hpp"
#include "binable.hpp"
#include <string>
#include <filesystem>

template<class T, int block_size>
SeqDB<T, block_size>::SeqDB(std::string name, int db_id, std::string path) : river(path + name + ".seqdb") {
    if (!std::filesystem::exists(path + name + ".seqdb")) {
        river.initialise(path + name + ".seqdb");
        river.write_info(db_id, 1);
        river.write_info(sizeofT, 2);
        river.write_info(block_size, 3);
        // river.write_info(size, 4); 默认会写0的
    }
    else {
        river.bind(path + name + ".seqdb");
        int id;
        river.get_info(id, 1);
        if (db_id != 0 && id != 0 && db_id != id) {
            throw DBFileNotMatchException();
        }
        int sizeofT_;
        river.get_info(sizeofT_, 2);
        if (sizeofT_ != sizeofT) {
            throw DBFileNotMatchException();
        }
        int block_size_;
        river.get_info(block_size_, 3);
        if (block_size_ != block_size) {
            throw DBFileNotMatchException();
        }
        river.get_info(len_, 4);
    }
}

template<class T, int block_size>
SeqDB<T, block_size>::~SeqDB() {
    river.write_info(len_, 4);
    river.close();
}

template<class T, int block_size>
char* SeqDB<T, block_size>::array::to_bin() {
    if constexpr (binable<T>) {
        char* bin = new char[sizeofT * array_size];
        char* tmp;
        for (int i = 0; i < array_size; ++i) {
            tmp = data[i].to_bin();
            std::copy(tmp, tmp + sizeofT, bin + i * sizeofT);
            delete[] tmp;
        }
        return bin;
    }
    else {
        char* bin = new char[bin_size()];
        for (int i = 0; i < array_size; ++i) {
            std::copy(reinterpret_cast<char*>(&data[i]), reinterpret_cast<char*>(&data[i]) + sizeofT, bin + i * sizeofT);
        }
        return bin;
    }
}

template<class T, int block_size>
void SeqDB<T, block_size>::array::from_bin(char* bin) {
    if constexpr (binable<T>) {
        char* ptr = bin;
        for (int i = 0; i < array_size; ++i) {
            data[i].from_bin(ptr);
            ptr += sizeofT;
        }
    }
    else {
        std::copy(bin, bin + bin_size(), reinterpret_cast<char*>(data));
    }
}

template<class T, int block_size>
constexpr const int SeqDB<T, block_size>::array::bin_size() {
    return sizeofT * array_size;
}

template<class T, int block_size>
void SeqDB<T, block_size>::push_back(T &t) {
    int idx = len_ / array_size + 1; // 0号是block_river用来存元数据的
    if (len_ % array_size == 0) {
        array arr{{t}};
        int new_idx = river.write(arr);
        if (new_idx % block_size != 0) {
            throw FileBroken();
        }
        if (new_idx / block_size != idx) {
            throw FileBroken();
        }
    }
    else {
        array arr;
        river.read(arr, idx * block_size);
        arr.data[len_ % array_size] = t;
        river.update(arr, idx * block_size);
    }
    len_++;
    river.write_info(len_, 4);
}

template<class T, int block_size>
void SeqDB<T, block_size>::push_back(const T &t) {
    int idx = len_ / array_size + 1; // 0号是block_river用来存元数据的
    if (len_ % array_size == 0) {
        array arr{{t}};
        int new_idx = river.write(arr);
        if (new_idx % block_size != 0) {
            throw FileBroken();
        }
        if (new_idx / block_size != idx) {
            throw FileBroken();
        }
    }
    else {
        array arr;
        river.read(arr, idx * block_size);
        arr.data[len_ % array_size] = t;
        river.update(arr, idx * block_size);
    }
    len_++;
    river.write_info(len_, 4);
}

template<class T, int block_size>
void SeqDB<T, block_size>::resize(int size) {
    if (size < 0) {
        throw std::out_of_range("size out of range");
    }
    int idx = (len_ + array_size - 1) / array_size; // 收缩时向上取整，再减一，再加一
    len_ = size;
    river.write_info(len_, 4);
    int new_idx =  (len_ + array_size - 1) / array_size;
    if (idx <= new_idx) {
        return;
    }
    // 收缩
    river.Delete(idx - new_idx);
}

template<class T, int block_size>
void SeqDB<T, block_size>::update(T &t, const int index) {
    int idx = index / array_size + 1;
    array arr;
    river.read(arr, idx * block_size);
    arr.data[index % array_size] = t;
    river.update(arr, idx * block_size);
}

template<class T, int block_size>
void SeqDB<T, block_size>::update(const T &t, const int index) {
    int idx = index / array_size + 1;
    array arr;
    river.read(arr, idx * block_size);
    arr.data[index % array_size] = t;
    river.update(arr, idx * block_size);
}

template<class T, int block_size>
T SeqDB<T, block_size>::operator[](const int index) {
    int idx = index / array_size + 1;
    array arr;
    river.read(arr , idx * block_size);
    return arr.data[index % array_size];
}

template<class T, int block_size>
T SeqDB<T, block_size>::at(const int index) {
    if (index >= len_) {
        throw std::out_of_range("index out of range");
    }
    return operator[](index);
}

template<class T, int block_size>
std::vector<T> SeqDB<T, block_size>::range(int l, int r) {
    if (l < 0 || r > len_ || l > r) {
        throw std::out_of_range("range out of range");
    }
    std::vector<T> res;
    int l_idx = l / array_size + 1;
    int r_idx = r / array_size + 1;
    if (l_idx == r_idx) {
        array arr;
        river.read(arr, l_idx * block_size);
        for (int i = l % array_size; i < r % array_size; ++i) {
            res.push_back(arr.data[i]);
        }
    }
    else {
        array arr;
        river.read(arr, l_idx * block_size);
        for (int i = l % array_size; i < array_size; ++i) {
            res.push_back(arr.data[i]);
        }
        for (int i = l_idx + 1; i < r_idx; ++i) {
            river.read(arr, i * block_size);
            for (int j = 0; j < array_size; ++j) {
                res.push_back(arr.data[j]);
            }
        }
        river.read(arr,r_idx * block_size);
        for (int i = 0; i < r % array_size; ++i) {
            res.push_back(arr.data[i]);
        }
    }
    return res;
}

template<class T, int block_size>
void SeqDB<T, block_size>::pop() {
    if (len_ == 0) {
        throw std::out_of_range("pop from empty SeqDB");
    }
    resize(len_ - 1);
}