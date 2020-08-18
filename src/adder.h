//
// Created by Jisoo on 2020/08/16.
//

#ifndef DRAMSIM3_ADDER_H
#define DRAMSIM3_ADDER_H
#include <iostream>


class adder {
public:
    void setWorking(){isWorking = true;}
    void resetWorking(){isWorking = false;}
    int getCounter(){return counter_;}
    void addCounter(){counter_++;}
    void resetCounter(){counter_ = 0;}
    int getCycles(){return cycles_;}
private:
    bool isWorking;
    int counter_;
    int cycles_ = 2;
};



#endif //DRAMSIM3_ADDER_H
