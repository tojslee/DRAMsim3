//
// Created by Jisoo on 2020/08/20.
//

#include "calculator.h"
#include <iostream>

bool calculator::endGetInput(int idx) {
    if(idx == 1){
        return row == inputBuffer.nums + inputBuffer.waitAddr.size();
    }
    else if(idx == 2){
        return row == filterBuffer.nums + filterBuffer.waitAddr.size();
    }
    else{
        return row == outputBuffer.nums + outputBuffer.waitAddr.size();
    }
}

void calculator::getInput(int idx, uint64_t addr_, uint64_t offset_) {
    if(idx == 1){
        inputBuffer.waitAddr.push_back(addr_ + offset_);
    }
    else if(idx == 2){
        filterBuffer.waitAddr.push_back(addr_ + offset_);
    }
}

std::vector<uint64_t> calculator::getAddr(int idx){
    if(idx == 1){return inputBuffer.waitAddr;}
    else if(idx == 2){return filterBuffer.waitAddr;}
}

void calculator::setAddr(int idx, std::vector<uint64_t> temp){
    if(idx == 1){
        inputBuffer.waitAddr.resize(temp.size());
        std::copy(temp.begin(), temp.end(), inputBuffer.waitAddr.begin());
    }
    else if(idx == 2){
        filterBuffer.waitAddr.resize(temp.size());
        std::copy(temp.begin(), temp.end(), filterBuffer.waitAddr.begin());
    }
}

void calculator::addBuffer(int idx){
    if(idx == 1){
        inputBuffer.nums++;
        if(inputBuffer.nums == row){inputBuffer.isIn = true;}
    }
    else if (idx == 2){
        filterBuffer.nums++;
        if(filterBuffer.nums == row){filterBuffer.isIn = true;}
    }
    else{
        outputBuffer.nums++;
        if(outputBuffer.nums == row){outputBuffer.isIn = true;}
    }
}

bool calculator::fullBuffer(int idx) {
    if(idx == 1){
        return inputBuffer.getisIn();
    }
    else if(idx == 2){
        return filterBuffer.getisIn();
    }
    else{
        return outputBuffer.getisIn();
    }
}

bool calculator::matrixMultiple(){
    if(inputCal <= column + row - 1){inputCal++;}
    if(filterCal <= row){filterCal++;}
    usage += inputCal;
    usage += filterCal;
    if(inputCal > column + row - 1 && filterCal > row){
        outputBuffer.nums = row * column;
        outputBuffer.isIn = true;
        return true;
    }
    else{return false;}
}

void calculator::bufferReset(int idx){
    if(idx == 1){
        inputBuffer.nums = 0;
        inputBuffer.isIn = false;
    }
    else if(idx == 2){
        filterBuffer.nums = 0;
        filterBuffer.isIn = false;
    }
    else{
        outputBuffer.nums = 0;
        outputBuffer.isIn = false;
    }
}

int calculator::getNums(int idx){
    if(idx == 1){return inputBuffer.nums;}
    else if(idx == 2){return filterBuffer.nums;}
    else{return outputBuffer.nums;}
}

void calculator::subNums(int idx){
    if(idx == 1){inputBuffer.nums--;}
    else if(idx == 2){filterBuffer.nums--;}
    else{outputBuffer.nums--;}
}