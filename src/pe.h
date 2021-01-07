//
// Created by Jisoo on 20. 12. 28..
//

#ifndef DRAMSIM3_PE_H
#define DRAMSIM3_PE_H
#include <vector>
#include <utility>


class pe {
public:
    void fixWeight(std::vector<std::vector<int>> v, pe* firstCol);
    void matrixMultiple();
    void propagation(std::vector<pe *> c, int idx);
    int aReg = 0;
    int bReg;
    int psumReg = 0;
    bool isValue = false;
    pe *rightPE;
    pe *downPE;
    std::pair<int, bool> upPE = std::make_pair(0, true);
    std::pair<int, bool> leftPE = std::make_pair(0, true);
};



#endif //DRAMSIM3_PE_H
