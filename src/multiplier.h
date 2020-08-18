//
// Created by Jisoo on 2020/08/17.
//

#ifndef DRAMSIM3_MULTIPLIER_H
#define DRAMSIM3_MULTIPLIER_H



class multiplier {
public:
    void setWorking(){isWorking = true;}
    void resetWorking(){isWorking = false;}
    int getCounter(){return counter_;}
    void addCounter(){counter_++;}
    void resetCounter(){counter_ = 0;}
    int getCycles(){return cycles_;}
protected:
    bool isWorking;
    int counter_ = 0;
    int cycles_ = 9;
};



#endif //DRAMSIM3_MULTIPLIER_H
