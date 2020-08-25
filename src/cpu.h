#ifndef __CPU_H
#define __CPU_H

#include <fstream>
#include <functional>
#include <random>
#include <string>
#include "memory_system.h"
#include "unit.h"
#include "adder.h"
#include "multiplier.h"
#include "calculator.h"

namespace dramsim3 {

class CPU {
   public:
    CPU(const std::string& config_file, const std::string& output_dir, const std::int32_t units_, const std::int32_t row_, const std::int32_t column_)
        : memory_system_(
              config_file, output_dir,
              std::bind(&CPU::ReadCallBack, this, std::placeholders::_1),
              std::bind(&CPU::WriteCallBack, this, std::placeholders::_1)),
          clk_(0), stall_counter_(0), units_(units_), col_(column_), row_(row_), systolic_array(row_, column_){
        for(int i=0;i<units_;i++){unit temp = unit(); allUnits.push_back(temp);
        }
    }
    virtual bool ClockTick() = 0;
    void ReadCallBack(uint64_t addr);
    void WriteCallBack(uint64_t addr) { writeCallBacks.push_back(addr);writeStart_ = true; return; }
    void PrintStats() { memory_system_.PrintStats(); }
    void PrintStall(){std::cout<<stall_counter_<<std::endl;}
    bool isEnd();
    int freeUnit();

   protected:
    MemorySystem memory_system_;
    uint64_t clk_;
    uint64_t stall_counter_;
    std::vector<uint64_t> readCallBacks;
    int units_;
    std::vector<unit> allUnits;
    adder add_;
    multiplier multiple_;
    std::vector<uint64_t> writeCallBacks;
    calculator systolic_array;
    int col_;
    int row_;
    bool writeStart_ = false;
};

class RandomCPU : public CPU {
   public:
    using CPU::CPU;
    bool ClockTick() override;

   private:
    uint64_t last_addr_;
    bool last_write_ = false;
    std::mt19937_64 gen;
    bool get_next_ = true;
};

class StreamCPU : public CPU {
   public:
    using CPU::CPU;
    bool ClockTick() override;

   private:
    uint64_t addr_a_, addr_b_, addr_c_, offset_a_, offset_b_, offset_c_ = 0;
    std::mt19937_64 gen;
    bool inserted_a_ = false;
    bool inserted_b_ = false;
    bool inserted_c_ = false;
    const uint64_t array_size_ = 64;  // elements in array
    const int stride_ = 4;                // stride in bytes
    const int elements_ = 16;
    int counter_ = 0;
    bool endCal = false;
};

class TraceBasedCPU : public CPU {
   public:
    TraceBasedCPU(const std::string& config_file, const std::string& output_dir,
                  const std::string& trace_file, const::int32_t units_, const std::int32_t row_, const std::int32_t column_);
    ~TraceBasedCPU() { trace_file_.close(); }
    bool ClockTick() override;

   private:
    std::ifstream trace_file_;
    Transaction trans_;
    bool get_next_ = true;
    bool stalled_ = false;
    std::vector<Transaction> trace_;
};

}  // namespace dramsim3
#endif
