#ifndef __DRAM_SYSTEM_H
#define __DRAM_SYSTEM_H

#include <fstream>
#include <string>
#include <vector>
#include <utility>

#include "common.h"
#include "configuration.h"
#include "controller.h"
#include "timing.h"
#include "buffer.h"

#ifdef THERMAL
#include "thermal.h"
#endif  // THERMAL

namespace dramsim3 {

class BaseDRAMSystem {
   public:
    BaseDRAMSystem(Config &config, const std::string &output_dir,
                   std::function<void(uint64_t)> read_callback,
                   std::function<void(uint64_t)> write_callback, int row, int array, int sys_row);
    virtual ~BaseDRAMSystem() {}
    void RegisterCallbacks(std::function<void(uint64_t)> read_callback,
                           std::function<void(uint64_t)> write_callback);
    void PrintEpochStats();
    void PrintStats();
    void ResetStats();
    int isEnd();



    virtual bool WillAcceptTransaction(uint64_t hex_addr,
                                       bool is_write) const = 0;
    virtual bool AddTransaction(uint64_t hex_addr, bool is_write) = 0;
    bool SetBuffer(uint64_t hex_addr, bool is_write, int flag);
    bool ResetBuffer(int flag);
    uint64_t getAddr(int flag);
    virtual void ClockTick() = 0;
    int GetChannel(uint64_t hex_addr) const;
    std::vector<std::vector<double>> getBuffer(std::pair<int, int> index);
    void newBuffer(std::vector<std::vector<double>> r, std::pair<int, int> index);

    std::function<void(uint64_t req_id)> read_callback_, write_callback_;
    static int total_channels_;
    bool getisIn(int flag);
    buffer buffer_a;
    buffer buffer_b;
    buffer buffer_c;
    void printBuff();
    void modifyInfo(int row, int array); // array, bcol
    std::vector<std::vector<double>> getallBuffer();

   protected:
    uint64_t id_;
    uint64_t last_req_clk_;
    Config &config_;
    Timing timing_;
    uint64_t parallel_cycles_;
    uint64_t serial_cycles_;
    std::vector<std::vector<double>> resArray;
    int row_; // arrayA length
    int array_size_; // b col size
    int sys_row_; // systolic array size

#ifdef THERMAL
    ThermalCalculator thermal_calc_;
#endif  // THERMAL

    uint64_t clk_;
    std::vector<Controller*> ctrls_;

#ifdef ADDR_TRACE
    std::ofstream address_trace_;
#endif  // ADDR_TRACE
};

// hmmm not sure this is the best naming...
class JedecDRAMSystem : public BaseDRAMSystem {
   public:
    JedecDRAMSystem(Config &config, const std::string &output_dir,
                    std::function<void(uint64_t)> read_callback,
                    std::function<void(uint64_t)> write_callback, int row, int array, int sys_row);
    ~JedecDRAMSystem();
    bool WillAcceptTransaction(uint64_t hex_addr, bool is_write) const override;
    bool AddTransaction(uint64_t hex_addr, bool is_write) override;
    void ClockTick() override;
};

// Model a memorysystem with an infinite bandwidth and a fixed latency (possibly
// zero) To establish a baseline for what a 'good' memory standard can and
// cannot do for a given application
class IdealDRAMSystem : public BaseDRAMSystem {
   public:
    IdealDRAMSystem(Config &config, const std::string &output_dir,
                    std::function<void(uint64_t)> read_callback,
                    std::function<void(uint64_t)> write_callback, int row, int array, int sys_row);
    ~IdealDRAMSystem();
    bool WillAcceptTransaction(uint64_t hex_addr,
                               bool is_write) const override {
        return true;
    };
    bool AddTransaction(uint64_t hex_addr, bool is_write) override;
    void ClockTick() override;

   private:
    int latency_;
    std::vector<Transaction> infinite_buffer_q_;
};

}  // namespace dramsim3
#endif  // __DRAM_SYSTEM_H
