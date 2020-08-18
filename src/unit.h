//
// Created by Jisoo on 2020/08/02.
//

#ifndef DRAMSIM3_UNIT_H
#define DRAMSIM3_UNIT_H

#include <iostream>
#include "common.h"

namespace dramsim3{
    class unit {
    public:
        unit(){occupied = false;}
        bool isOccupied() const {return occupied;}
        bool occupied;
        Transaction trans;
        uint64_t  addr;
    };
}



#endif //DRAMSIM3_UNIT_H
