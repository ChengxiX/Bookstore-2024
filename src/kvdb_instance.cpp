#include "../include/kvdb.cpp"
#include <vector>
#include <iostream>

int main() {
    std::string op;
    int n;
    std::cin >> n;
    KVDB<int, std::less<int>> db("db2");
    for (int i = 0; i < n; i++) {
        std::cin >> op;
        if (op == "insert") {
            std::string key;
            int value;
            std::cin >> key >> value;
            db.insert(key, value);
        }
        else if (op == "delete") {
            std::string key;
            int value;
            std::cin >> key >> value;
            db.erase(key, value);
        }
        else if (op == "find") {
            std::string key;
            std::cin >> key;
            if (key == "23"){
                int x = 2;
            }
            std::vector<int> res = db.find(key);
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