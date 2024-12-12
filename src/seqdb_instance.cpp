#include "../include/seqdb.cpp"
#include <iostream>

int main () {
    std::cout << binable<int> << std::endl;
    SeqDB<long long> db("1");
    long long a = 5;
    db.push_back(a);
}