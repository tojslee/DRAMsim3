//
// Created by Jisoo on 2020/08/16.
//

#ifndef DRAMSIM3_BUFFER_H
#define DRAMSIM3_BUFFER_H
#include <iostream>


class buffer {
public:
    std::uint64_t addr;
    bool isIn = false;
    bool getisIn(){return isIn;}
};



#endif //DRAMSIM3_BUFFER_H
