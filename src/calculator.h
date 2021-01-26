//
// Created by Jisoo on 2020/08/20.
//

#ifndef DRAMSIM3_CALCULATOR_H
#define DRAMSIM3_CALCULATOR_H
#include <iostream>
#include <vector>
#include <utility>
#include "buffer.h"
#include "pe.h"


class calculator {
public:
    calculator(int row_, int column_, int array_, int array_height);
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
    bool propagation(std::pair<int, int> index);
    pe *firstPE;
    std::vector<pe *> firstColumn;
    std::vector<std::vector<int>> arrayA;
    std::vector<std::vector<int>> arrayB;
    std::vector<std::vector<int>> getR(){return arrayR;}
    void setR(std::vector<std::vector<int>> r);
    void setUsage(int a, int b){row_usage = a; col_usage = b;}

protected:
    double row;
    double column;
    int idx = 0;
    int usage = 0;
    double array_length;
    double array_height;
    std::vector<int> resIdx;
    std::vector<std::vector<int>> arrayR;
    std::vector<int> aIdx;
    buffer inputBuffer; // idx = 1
    buffer filterBuffer; // idx = 2
    buffer outputBuffer; // idx = 3
    int row_usage;
    int col_usage;
};


#endif //DRAMSIM3_CALCULATOR_H
