#include "randomdb.cpp"
#include <array>
#include <functional>
#include <iostream>
#include <cstring>
#include <utility>

struct cmp {
    bool operator()(const std::array<char, 1024> &a, const std::array<char, 1024> &b) const {
        char as[1024];
        char bs[1024];
        for (int i=0; i < 1024; i++) {
            as[i] = a[i];
            bs[i] = b[i];
        }
        return std::strcmp(as, bs) < 0;
    }
};

int integer() {
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
    return 0;
}

int strings() {
    std::string op;
    int n;
    std::cin >> n;
    RandomDB<std::array<char, 1024>, cmp> db("dbstring", 0, "", false);
    for (int i = 0; i < n; i++) {
        std::cin >> op;
        if (op == "insert") {
            char value[1024];
            std::cin >> value;
            std::array<char, 1024> v;
            for (int i = 0; i < 1024; i++){
                v[i] = value[i];
            }
            db.insert(v);
        }
        else if (op == "delete") {
            char value[1024];
            std::cin >> value;
            std::array<char, 1024> v;
            for (int i = 0; i < 1024; i++){
                v[i] = value[i];
            }
            db.erase(v);
        }
        else if (op == "exist") {
            char value[1024];
            std::cin >> value;
            std::array<char, 1024> v;
            for (int i = 0; i < 1024; i++){
                v[i] = value[i];
            }
            std::cout << db.exist(v) <<std::endl;
        }
        else if (op == "exit") {
            return 0;
        }
    }
    return 0;
}

int kv() {
    std::string op;
    int n;
    std::cin >> n;
    RandomDB<int, std::less<int>, char> db("dbkv1");
    for (int i = 0; i < n; i++) {
        std::cin >> op;
        if (op == "insert") {
            int value;
            char v;
            std::cin >> value >>v;
            db.insert(std::pair<int, char>(value, v));
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
        else if (op == "find") {
            int value;
            std::cin >> value;
            auto res = db.find(value);
            for (auto &i : res) {
                std::cout << i.first << i.second << std::endl;
            }
        }
    }
    return 0;
}

int main() {
    return kv();
}