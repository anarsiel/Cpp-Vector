//
// Created by anarsiel on 16.06.19.
//

#include <iostream>
#include "vector.h"

int main() {
    vector<int> c;
    c.push_back(1);
    c.push_back(2);
    c.push_back(3);

    vector<int> d;
    d.push_back(4);
    d.push_back(5);

    d = c;
    return 0;
}