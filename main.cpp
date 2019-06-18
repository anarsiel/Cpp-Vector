//
// Created by anarsiel on 16.06.19.
//

#include <iostream>
#include "vector.h"

struct point {
    point(){}
    ~point(){}

    point(int a, int b) : a(a), b(b) {}

    int a, b;
};

int main() {
//    int x = 239, y= 1488;
//    vector<point> v;
//    v.push_back({x, y});
//    v.push_back({y, x});
//    v.push_back({x - 20, x + 239});
//    v.push_back({2, y});
//
//    for (size_t i = 0; i < v.size(); ++i)
//        std::cout << v[i].a << ' ' << v[i].b << std::endl;

    vector<int> a;
    a.push_back(2);
    a.push_back(9);
    a.push_back(3);
    for (size_t i = 0; i < a.size(); ++i)
        std::cout << a[i] << ' ';
    std::cout << std::endl;
    return 0;
}