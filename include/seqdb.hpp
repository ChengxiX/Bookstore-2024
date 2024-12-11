#include "binable.hpp"
#include "BlockRiver.hpp"
#include <vector>

template<class T, int block_size = 4096>
class SeqDB {
    using index = int;
    constexpr static const int sizeofT = (binable<T>) ? T::bin_size() : sizeof(T);
    constexpr static const int array_size = block_size / sizeofT;
    struct array {
        T data[array_size];
    };
    BlockRiver<array, block_size> river;
public:
    int size = 0;
    SeqDB(std::string dbname) : river(dbname) {}
    void push_back(T &t);
    void resize(int size);
    void update(T &t, const index index);
    T operator[](const index index);
    T at(const index index);
    void read(T &t, const index index);
    std::vector<T> range(index l, index r);
};