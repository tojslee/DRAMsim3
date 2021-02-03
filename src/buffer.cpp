//
// Created by Jisoo on 2020/08/16.
//

#include "buffer.h"
#include <iostream>

void buffer::reset(){
    nums = 0;
    isIn = false;
    values.clear();
    waitAddr.clear();
}
