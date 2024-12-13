#include "operations.cpp"
#include "models.hpp"
#include <stack>
#include <iostream>
#include <sstream>
#include <string>

extern const int INSTANCE_ID;

int main() {
    if (User::db.init("user", INSTANCE_ID, "data/", false)) {
        User::useradd("root", "sjtu", "admin", 7, 8, "root");
    }
    std::stack<int> selected_books;
    std::string cmd;
    std::string current_user;
    int current_pri = 0;
    while (std::getline(std::cin, cmd)) {
        std::stringstream ss(cmd);
        std::string op;
        ss >> op;
        if (op == "exit" || op == "quit") {
            break;
        }
        else if (op == "su") {
            std::string id, password;
            ss >> id >> password;
            if (User::su(id, password, current_pri)) {
                current_user = id;
                current_pri = 0;
            }
            else {
                std::cout << "Invalid" << std::endl;
            }
        }
    }
}