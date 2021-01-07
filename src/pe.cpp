//
// Created by Jisoo on 20. 12. 28..
//

#include "pe.h"
#include <iostream>
#include <vector>

void pe::fixWeight(std::vector<std::vector<int> > v, pe *firstCol) {
    this->bReg = v[0][0];
    if(this->rightPE != NULL){
        v[0].erase(v[0].begin());
        this->rightPE->fixWeight(v, firstCol);
    }
    else{
        if(firstCol->downPE != NULL){
            v.erase(v.begin());
            firstCol->downPE->fixWeight(v, firstCol->downPE);
        }
    }
}

void pe::matrixMultiple() {
    this->psumReg = this->psumReg + this->aReg * this->bReg;
}

void pe::propagation(std::vector<pe *> c, int idx) {
    // propagation
    if(this->rightPE != NULL){
        this->rightPE->leftPE = std::make_pair(aReg, isValue);
    }
    if(this->downPE != NULL){
        this->downPE->upPE = std::make_pair(psumReg, isValue);
    }

    // propagate propagation to all PEs in order
    if(this->rightPE != NULL){
        this->rightPE->propagation(c, idx);
    }
    else{
        idx += 1;
        if(c.size()>idx){
            c[idx]->propagation(c, idx);
        }
    }

    // updating values of each PEs
    this->aReg = leftPE.first;
    this->psumReg = upPE.first;
    if(leftPE.second && upPE.second){
        this->isValue = true;
    }
}