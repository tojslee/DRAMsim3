//
// Created by Jisoo on 2021/01/28.
//

#ifndef DRAMSIM3_LAYERINFO_H
#define DRAMSIM3_LAYERINFO_H

#include <string>
#include <vector>


class layerInfo {
public:
    layerInfo(string type_, int minibatch_, int aRow_, int aCol_, int k_,
              int bRow_, int bCol_,  int filterNum_, string optimal_):
    type(type_), aRow(aRow_), aCol(aCol_), k(k_), bRow(bRow_), bCol(bCol_),
    minibatch(minibatch_), filterNum(filterNum_), optimal(optimal_){}
    // getter
    int getaRow(){return aRow;}
    int getaCol(){return aCol;}
    int getbRow(){return bRow;}
    int getbCol(){return bCol;}
    int getK(){return k;}
    int getMinibatch(){return minibatch;}
    int getFilterNum(){return filterNum;}
    string getType(){return type;}
    string getOptimal(){return optimal;}
    void setarrayA(std::vector<std::vector<std::vector<std::vector<int>>>> a);
    void setarrayB(std::vector<std::vector<std::vector<std::vector<int>>>> b);

private:
    int aRow;
    int aCol;
    int bRow;
    int bCol;
    int k;
    std::string type; // Conv, FC
    int minibatch;
    int filterNum;
    std::string optimal; // ReLU
    std::vector<std::vector<std::vector<std::vector<int>>>> arrayA; // minibatch *(aRow*aCol*k)
    std::vector<std::vector<std::vector<std::vector<int>>>> arrayB; // filterNum *(bRow*bCol*k)
};



#endif //DRAMSIM3_LAYERINFO_H
