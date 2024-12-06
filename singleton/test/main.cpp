#include "singleton_template.hpp"
#include <string>
#include <iostream>

class CSingleton {
public:
    CSingleton(std::string str) {
        std::cout << str << std::endl;
    }
    CSingleton() {
        std::cout << "no arg" << std::endl;
    }
    ~CSingleton() {
        std::cout << "~CSingleton" << std::endl;
    }
};

int main () {
    CSingleton* p = CSingletonTemplate<CSingleton, std::string>::getInstance("hello");
    CSingleton* p1 = CSingletonTemplate<CSingleton>::getInstance();
    return 0;
}