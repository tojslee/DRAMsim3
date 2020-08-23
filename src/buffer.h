//
// Created by Jisoo on 2020/08/16.
//

#ifndef DRAMSIM3_BUFFER_H
#define DRAMSIM3_BUFFER_H
#include <iostream>
#include <vector>

class buffer {
public:
    std::vector<std::uint64_t> waitAddr;
    int nums = 0;
    bool isIn = false;
    bool getisIn(){return isIn;}
};



#endif //DRAMSIM3_BUFFER_H
