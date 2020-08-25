//
// Created by Jisoo on 2020/08/20.
//

#ifndef DRAMSIM3_CALCULATOR_H
#define DRAMSIM3_CALCULATOR_H
#include <iostream>
#include <vector>
#include "buffer.h"


class calculator {
public:
    calculator(int row_, int column_):row(row_), column(column_){};
    void setRowColumn(int row, int column){this->row = row;this->column = column;}
    bool endGetInput(int idx);
    void getInput(int idx, uint64_t addr_, uint64_t offset_);
    std::vector<uint64_t> getAddr(int idx);
    void setAddr(int idx, std::vector<uint64_t> temp);
    void addBuffer(int idx);
    bool fullBuffer(int idx);
    bool matrixMultiple();
    void bufferReset(int idx);
    int getNums(int idx);
    void subNums(int idx);
    int getUsage(){return usage;}
protected:
    int row;
    int column;
    int inputCal = 0;
    int filterCal = 0;
    int usage = 0;
    buffer inputBuffer; // idx = 1
    buffer filterBuffer; // idx = 2
    buffer outputBuffer; // idx = 3
};



#endif //DRAMSIM3_CALCULATOR_H
