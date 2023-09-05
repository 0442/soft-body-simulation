#include<bits/stdc++.h>
#include<random>
#ifndef ID_HPP
#define ID_HPP

using namespace std;

// ascii char range from 0x20 to 0x7E
#define ID_START 0x20
#define ID_END 0x7E
#define ID_LEN 10

string gen_id() {
    string id;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(ID_START, ID_END);

    for (int i = 0; i < ID_LEN; i++) {
        id.push_back((char)dist(gen));
    }

    return id;
}

#endif