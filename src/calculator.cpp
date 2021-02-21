//
// Created by Jisoo on 2020/08/20.
//

#include "calculator.h"
#include <iostream>
#include <cmath>

calculator::calculator(int row_, int column_, int array_, int array_height){
    row = row_;
    column = column_;
    array_length = array_;
    this->array_height = array_height;
    for(int i=0;i<std::min(array_height, row_);i++){
        std::vector<double> v;
        //for(int j=0;j<array_;j++){
        //    v.push_back(0);
        //}
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

    if(ceil(this->array_height/row) == 0){
        int temp = 0;
        for(int i=0;i<row_;i++){
            aIdx.push_back(temp);
            temp -= 1;
        }
    }
    for(int j=0;j<ceil(this->array_height/this->row);j++){
        int temp = 0;
        for(int i=0;i<row_;i++){
            aIdx.push_back(temp);
            temp -= 1;
        }
    }
}

bool calculator::endGetInput(int idx) {
    if(idx == 1){
        return array_length*array_height == inputBuffer.nums + inputBuffer.waitAddr.size();
    }
    else if(idx == 2){
        return array_length*array_height == filterBuffer.nums + filterBuffer.waitAddr.size();
    }
    else{
        return array_length*array_height == outputBuffer.nums + outputBuffer.waitAddr.size();
    }
}

void calculator::reset(){
    inputBuffer.reset();
    filterBuffer.reset();
    outputBuffer.reset();
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
        inputBuffer.nums = array_length*array_height;
        inputBuffer.isIn = true;
        //inputBuffer.nums++;
        //if(inputBuffer.nums == row){inputBuffer.isIn = true;}
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

bool calculator::propagation(std::pair<int, int> index){ // propagate value to next pe
    // get result from the PE array
    pe *iterator = firstColumn[firstColumn.size()-1];
    int idx = 0;
    while(iterator != NULL){
        if(iterator->isValue){
            // is real value
            if(resIdx[idx] < array_length){
                if(arrayR[idx].size() == array_length){
                    int tem = arrayR[idx][0];
                    arrayR[idx].erase(arrayR[idx].begin());
                    arrayR[idx].push_back(tem + iterator->psumReg);
                }
                else{
                    arrayR[idx].push_back(iterator->psumReg);
                }
                resIdx[idx]++;
            }
        }
        iterator = iterator->rightPE;
        idx += 1;
        if(idx == col_usage){break;}
    }

    // push new array of A to PE array
    iterator = firstPE;
    for(int i=0;i<row;i++){
        if(aIdx[i+index.first*4] < 0){
            iterator->leftPE = std::make_pair(0, false);
        }
        else if(aIdx[i+index.first*4] >= arrayA.size()){
            iterator->leftPE = std::make_pair(0, true);
        }
        else{
            if(aIdx[i+index.first*4] < arrayA.size() && arrayA[0].size() > i+index.first*4){
                iterator->leftPE = std::make_pair(arrayA[aIdx[i+index.first*4]][i+index.first*4], true);
            }
        }
        aIdx[i+index.first*4]++;
        iterator = iterator->downPE;
    }

    // inside propagation of psum & array A
    firstPE->propagation(firstColumn, 0);

    for(auto iter = firstColumn.begin();iter != firstColumn.end();iter++){
        auto temp = *iter;
        if(temp->upPE.second){
            temp->isValue = true;
        }
        while(temp != NULL){
            std::cout<<temp->aReg<<" "<<temp->bReg<<" "<<temp->psumReg<<" "<<temp->isValue<<std::endl;
            temp = temp->rightPE;
        }
    }
    std::cout<<std::endl;
    // all calculated
    for(int i=0;i<col_usage;i++){
        if(resIdx[i] < array_length){
            return false;
        }
    }

    for(auto iter = arrayR.begin();iter != arrayR.end();iter++){
        std::vector<double> eachline = *iter;
        for(auto iter2 = eachline.begin(); iter2 != eachline.end();iter2++){
            if(*iter2 < 0){*iter2 = 0;}
            std::cout<< *iter2<<" ";
        }
        std::cout<<std::endl;
    }
    outputBuffer.nums += array_length * col_usage;
    for(auto iter = firstColumn.begin();iter != firstColumn.end();iter++){
        auto it = *iter;
        while(it!=NULL){
            it->isValue = false;
            it->bReg = 0;
            it = it->rightPE;
        }
    }
    resIdx.clear();
    for(int i=0;i<row;i++){
        resIdx.push_back(0);
    }
    aIdx.clear();
    if(ceil(array_height/row) == 0){
        int temp = 0;
        for(int i=0;i<row;i++){
            aIdx.push_back(temp);
            temp -= 1;
        }
    }
    for(int j=0;j<ceil(array_height/row);j++){
        int temp = 0;
        for(int i=0;i<row;i++){
            aIdx.push_back(temp);
            temp -= 1;
        }
    }
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

void calculator::setR(std::vector<std::vector<double>> r){
    arrayR.clear();
    arrayR.assign(r.begin(), r.end());
}

void calculator::modifyInfo(int array_length_, int array_height_) {
    array_length = (double)array_length_;
    array_height = (double)array_height_;

    arrayR.clear();
    resIdx.clear();
    for(int i=0;i<std::min(array_height, row);i++){
        std::vector<double> v;
        arrayR.push_back(v);
        resIdx.push_back(0);
    }

    aIdx.clear();
    if(ceil(this->array_height/row) == 0){
        int temp = 0;
        for(int i=0;i<row;i++){
            aIdx.push_back(temp);
            temp -= 1;
        }
    }
    for(int j=0;j<ceil(this->array_height/this->row);j++){
        int temp = 0;
        for(int i=0;i<row;i++){
            aIdx.push_back(temp);
            temp -= 1;
        }
    }
}