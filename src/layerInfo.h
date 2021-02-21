//
// Created by Jisoo on 2021/01/29.
//

#ifndef DRAMSIM3_LAYERINFO_H
#define DRAMSIM3_LAYERINFO_H

#include <vector>
#include <string>

class layerInfo {
public:
    layerInfo();
    layerInfo(std::string type_, int minibatch_, int aRow_, int aCol_, int k_, int filterNum_, int bRow_, int bCol_, std::string optimal_);
    // getter
    int getaRow(){return aRow;}
    int getaCol(){return aCol;}
    int getbRow(){return bRow;}
    int getbCol(){return bCol;}
    int getK(){return k;}
    int getMinibatch(){return minibatch;}
    int getFilterNum(){return filterNum;}
    void setAll(std::string type_, int minibatch_, int aRow_, int aCol_, int k_, int filterNum_, int bRow_, int bCol_, std::string optimal_);
    std::vector<std::vector<std::vector<std::vector<double>>>> getarrayA(){return arrayA;}
    std::vector<std::vector<std::vector<std::vector<double>>>> getarrayB(){return arrayB;}
    std::string getType(){return type;}
    std::string getOptimal(){return optimal;}
    void setarrayA(std::vector<std::vector<std::vector<std::vector<double>>>> a);
    void setarrayB(std::vector<std::vector<std::vector<std::vector<double>>>> b);

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
    std::vector<std::vector<std::vector<std::vector<double>>>> arrayA; // minibatch *(aRow*aCol*k)
    std::vector<std::vector<std::vector<std::vector<double>>>> arrayB; // filterNum *(bRow*bCol*k)
};




#endif //DRAMSIM3_LAYERINFO_H
