//
// Created by anarsiel on 16.06.19.
//

#include <iostream>
#include "vector.h"

int main() {
    vector<int> c, c2;

    c.push_back(4);
    c2.push_back(1);
    c2.push_back(2);
    c2.push_back(3);

    std::cout << c.size() << ' ' << c2.size() << std::endl;

    swap(c, c2);

    std::cout << c.size() << ' ' << c2.size() << std::endl;

    for (size_t i = 0; i < c.size(); ++i)
        std::cout << c[i] << ' ';
    std::cout << std::endl;

    for (size_t i = 0; i < c2.size(); ++i)
        std::cout << c2[i] << ' ';


//    for (size_t i = 0; i < c2.size(); ++i)
//        std::cout << c2[i] << ' ';
//    std::cout << std::endl;
    return 0;
}