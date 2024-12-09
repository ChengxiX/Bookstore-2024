//#include "../include/kvdb.hpp"
#include "kvdb.cpp"
#include <vector>
#include <iostream>

int main() {
    std::string op;
    int n;
    std::cin >> n;
    KVDB<int, std::less<int>> db("db");
    for (int i = 0; i < n; i++) {
        std::cin >> op;
        if (op == "insert") {
            std::string key;
            int value;
            std::cin >> key >> value;
            db.Insert(key, value);
        }
        else if (op == "delete") {
            std::string key;
            int value;
            std::cin >> key >> value;
            db.Del(key, value);
        }
        else if (op == "find") {
            std::string key;
            std::cin >> key;
            std::vector<int> res = db.Find(key);
            for (int i : res) {
                std::cout << i << ' ';
            }
            if (res.empty()) {
                std::cout << "null";
            }
            std::cout << '\n';
        }
    }
}