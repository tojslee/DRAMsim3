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
}

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

bool calculator::matrixMultiple(){ // real calculation
    for(int i=0;i<row;i++){
        for(int j=0;j<row;j++){
            if(cal[i][j].isValue){
                int aReg = cal[i][j].aReg;
                int bReg = cal[i][j].bReg;
                int psumReg = cal[i][j].psumReg;
                cal[i][j].psumReg = aReg*bReg + psumReg;
            }
        }
    }
    return true;
}

bool calculator::propagation(){ // propagate value to next pe
    for(int i=row-1;i>=0;i--){
        for(int j=row-1;j>=0;j--){
            if(j!=row-1){ // except last column pe
                cal[i][j+1].aReg = cal[i][j].aReg; // propagate a array
            }

            if(i!=0 && i!=row-1){
                cal[i][j].psumReg = cal[i-1][j].psumReg;
            }
            else if(i==row-1){
                if(cal[i][j].isValue){
                    if(resIdx[j] < row){
                        arrayR[resIdx[j]][j] = cal[i][j].psumReg;
                        resIdx[j]++;
                    }
                }
                cal[i][j].psumReg = cal[i-1][j].psumReg;
            }

            if(i!=0){
                if(j!=0 && cal[i][j-1].isValue){
                    if(cal[i-1][j].isValue){
                        cal[i][j].isValue = true;
                    }
                }
            }
            else{
                if(j!=0 && cal[i][j-1].isValue){
                    cal[i][j].isValue = true;
                }
            }
        }
    }
    if(idx < row+column){
        for(int i=0;i<row;i++){
            cal[i][0].aReg = arrayA[i][idx];
        }
    }
    if(idx<row){cal[idx][0].isValue = true;}
    for(int i=0;i<row;i++){
        cal[0][i].psumReg = 0;
    }
    idx++;

    for(int i=0;i<row;i++){
        for(int j=0;j<row;j++){
            std::cout<<cal[i][j].aReg<<" "<<cal[i][j].bReg<<" "<<cal[i][j].psumReg<<" "<<cal[i][j].isValue<<std::endl;
        }
    }
    std::cout<<std::endl;
    for(int i=0;i<row;i++){
        for(int j=0;j<row;j++){
            std::cout<<arrayR[i][j]<<" ";
        }
        std::cout<<std::endl;
    }

    for(int i=0;i<row;i++){
        std::cout<<resIdx[i];
    }
    std::cout<<std::endl;

    if(idx >= row+column-1){
        bool flag = false;
        for(int i=0;i<row;i++){
            if(resIdx[i] != row){return false;}
        }
        outputBuffer.nums = row*column;
        return true;
    }
    return false;
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