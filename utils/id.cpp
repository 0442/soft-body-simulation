#include<bits/stdc++.h>
#include<random>

#ifndef UTILS_ID_CPP_
#define UTILS_ID_CPP_

// ascii char range from 0x20 to 0x7E
#define ID_START 0x20
#define ID_END 0x7E
#define ID_LEN 10

namespace utils {
    std::string a_gen_id()
    {
        std::string id;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(ID_START, ID_END);

        for (int i = 0; i < ID_LEN; i++) {
            id.push_back((char)dist(gen));
        }

        return id;
    }
}

#endif
