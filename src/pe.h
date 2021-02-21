//
// Created by Jisoo on 20. 12. 28..
//

#ifndef DRAMSIM3_PE_H
#define DRAMSIM3_PE_H
#include <vector>
#include <utility>


class pe {
public:
    void fixWeight(std::vector<std::vector<double>> v, pe* firstCol);
    void matrixMultiple();
    void propagation(std::vector<pe *> c, int idx);
    double aReg = 0;
    double bReg;
    double psumReg = 0;
    bool isValue = false;
    pe *rightPE;
    pe *downPE;
    std::pair<double, bool> upPE = std::make_pair(0, true);
    std::pair<double, bool> leftPE = std::make_pair(0, true);
};



#endif //DRAMSIM3_PE_H
