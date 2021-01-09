//
// Created by Jisoo on 2020/08/20.
//

#include "calculator.h"
#include <iostream>

calculator::calculator(int row_, int column_){
    row = row_;
    column = column_;
    for(int i=0;i<row;i++){
        std::vector<int> v;
        for(int j=0;j<row;j++){
            v.push_back(0);
        }
        arrayR.push_back(v);
        resIdx.push_back(0);
    }
    firstPE = new pe();
    firstColumn.push_back(firstPE);
    pe *iterator = firstPE;
    pe *upiter = firstPE;
    for(int i=0;i<row-1;i++){
        iterator->rightPE = new pe();
        iterator = iterator->rightPE;
    }
    iterator = firstPE;
    for(int i=0;i<row-1;i++){
        iterator->downPE = new pe();
        upiter = iterator;
        iterator = iterator->downPE;
        firstColumn.push_back(iterator);
        pe *rightiter = iterator;
        upiter = upiter->rightPE;
        for(int j=0;j<row-1;j++){
            rightiter->rightPE = new pe();
            rightiter = rightiter->rightPE;
            upiter->downPE = rightiter;
            upiter = upiter->rightPE;
        }
    }

    int temp = 0;
    for(int i=0;i<row;i++){
        aIdx.push_back(temp);
        temp -= 1;
    }
}

bool calculator::endGetInput(int idx) {
    if(idx == 1){
        return row*column == inputBuffer.nums + inputBuffer.waitAddr.size();
    }
    else if(idx == 2){
        return row*column == filterBuffer.nums + filterBuffer.waitAddr.size();
    }
    else{
        return row*column == outputBuffer.nums + outputBuffer.waitAddr.size();
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

bool calculator::matrixMultiple(){ // real calculation
    for(auto iter = firstColumn.begin();iter != firstColumn.end();iter++){
        pe *iterator = *iter;
        while(iterator != NULL){
            iterator->matrixMultiple();
            iterator = iterator->rightPE;
        }
    }
    return true;
}

bool calculator::propagation(){ // propagate value to next pe
    // get result from the PE array
    pe *iterator = firstColumn[firstColumn.size()-1];
    int idx = 0;
    while(iterator != NULL){
        if(iterator->isValue){
            // is real value
            if(resIdx[idx] < row){
                arrayR[resIdx[idx]][idx] = iterator->psumReg;
                resIdx[idx]++;
            }
        }
        iterator = iterator->rightPE;
        idx += 1;
    }

    // push new array of A to PE array
    iterator = firstPE;
    for(int i=0;i<row;i++){
        if(aIdx[i] < 0){
            iterator->leftPE = std::make_pair(0, false);
        }
        else if(aIdx[i] >= row){
            iterator->leftPE = std::make_pair(0, true);
        }
        else{
            iterator->leftPE = std::make_pair(arrayA[aIdx[i]][i], true);
        }
        aIdx[i]++;
        iterator = iterator->downPE;
    }

    // inside propagation of psum & array A
    firstPE->propagation(firstColumn, 0);

    // all calculated
    for(auto iter = resIdx.begin();iter != resIdx.end();iter++){
        if(*iter < row){
            return false;
        }
    }
    for(auto iter = arrayR.begin();iter != arrayR.end();iter++){
        std::vector<int> eachline = *iter;
        for(auto iter2 = eachline.begin(); iter2 != eachline.end();iter2++){
            std::cout<< *iter2<<" ";
        }
        std::cout<<std::endl;
    }
    outputBuffer.nums = row*column;
    return true;
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