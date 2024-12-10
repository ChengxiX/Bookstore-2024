#include "../include/randomdb.cpp"
#include <ostream>
#include <vector>
#include <iostream>

int main() {
    std::string op;
    int n;
    std::cin >> n;
    RandomDB<int> db("db");
    for (int i = 0; i < n; i++) {
        std::cin >> op;
        if (op == "insert") {
            int value;
            std::cin >> value;
            db.insert(value);
        }
        else if (op == "delete") {
            int value;
            std::cin >> value;
            db.erase(value);
        }
        else if (op == "exist") {
            int value;
            std::cin >> value;
            std::cout << db.exist(value) <<std::endl;
        }
    }
}