//
// Created by Jisoo on 2021/01/28.
//

#include "layerInfo.h"

void layerInfo::setarrayA(std::vector<std::vector<std::vector<std::vector<int>>>> a) {
    arrayA.clear();
    arrayA.assign(a.begin(), a.end());
}

void layerInfo::setarrayB(std::vector<std::vector<std::vector<std::vector<int>>>> b) {
    arrayB.clear();
    arrayB.assign(b.begin(), b.end());
}