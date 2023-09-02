#include<bits/stdc++.h>
#include<random>
#ifndef ID_HPP
#define ID_HPP

using namespace std;

string gen_id() {
    int start = 0x20; // ascii chars from 0x20 to 0x7E
    int end = 0x7E;
    int id_len = 10;

    string id = "";

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(start, end);

    for (int i = 0; i < id_len; i++) {
        int r = dist(gen);
        id.push_back((char)r);
    }

    return id;
}

#endif