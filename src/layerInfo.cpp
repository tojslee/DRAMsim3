//
// Created by Jisoo on 2021/01/29.
//

#include "layerInfo.h"
#include <iostream>

layerInfo::layerInfo(std::string type_, int minibatch_, int aRow_, int aCol_, int k_, int filterNum_, int bRow_, int bCol_, std::string optimal_) {
    type = type_;
    minibatch = minibatch_;
    aRow = aRow_;
    aCol = aCol_;
    k = k_;
    filterNum = filterNum_;
    bRow = bRow_;
    bCol = bCol_;
    optimal = optimal_;
}

void layerInfo::setarrayA(std::vector<std::vector<std::vector<std::vector<int>>>> a) {
    arrayA.clear();
    arrayA.assign(a.begin(), a.end());
}

void layerInfo::setarrayB(std::vector<std::vector<std::vector<std::vector<int>>>> b) {
    arrayB.clear();
    arrayB.assign(b.begin(), b.end());
}