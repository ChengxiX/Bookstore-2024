#include "../include/seqdb.cpp"
#include <iostream>

int main () {
    SeqDB<long long> db("1");
    std::string op;
    while (std::cin >> op) {
        if (op == "push") {
            long long x;
            std::cin >> x;
            db.push_back(x);
        } else if (op == "pop") {
            db.pop();
        } else if (op == "size") {
            std::cout << db.size() << std::endl;
        } else if (op == "resize") {
            int x;
            std::cin >> x;
            db.resize(x);
        } else if (op == "update") {
            int x, y;
            std::cin >> x >> y;
            db.update(y, x);
        } else if (op == "at") {
            int x;
            std::cin >> x;
            std::cout << db.at(x) << std::endl;
        } else if (op == "range") {
            int l, r;
            std::cin >> l >> r;
            auto res = db.range(l, r);
            for (auto x : res) {
                std::cout << x << ' ';
            }
            std::cout << std::endl;
        }
        // std::cout << ": " << db.size() << std::endl;
    }
}