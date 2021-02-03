#include "dram_system.h"

#include <assert.h>

namespace dramsim3 {

// alternative way is to assign the id in constructor but this is less
// destructive
int BaseDRAMSystem::total_channels_ = 0;

BaseDRAMSystem::BaseDRAMSystem(Config &config, const std::string &output_dir,
                               std::function<void(uint64_t)> read_callback,
                               std::function<void(uint64_t)> write_callback, int row, int array, int sys_row)
    : read_callback_(read_callback),
      write_callback_(write_callback),
      last_req_clk_(0),
      config_(config),
      timing_(config_),
      row_(row),
      array_size_(array),
      sys_row_(sys_row),
#ifdef THERMAL
      thermal_calc_(config_),
#endif  // THERMAL
      clk_(0) {
    total_channels_ += config_.channels;

#ifdef ADDR_TRACE
    std::string addr_trace_name = config_.output_prefix + "addr.trace";
    address_trace_.open(addr_trace_name);
#endif
    for(int i=0;i<array_size_;i++){
        std::vector<int> temp;
        for(int j=0;j<row_;j++){
            temp.push_back(0);
        }
        resArray.push_back(temp);
    }
}

void BaseDRAMSystem::printBuff(){
    for(int i=0;i<row_;i++){
        for(int j=0;j<array_size_;j++){
            std::cout<<resArray[j][i]<<" ";
        }
        std::cout<<std::endl;
    }
}

void BaseDRAMSystem::modifyInfo(int row, int array) {
    row_ = row;
    array_size_ = array;
    resArray.clear();
    for(int i=0;i<array;i++){
        std::vector<int> temp;
        for(int j=0;j<row;j++){
            temp.push_back(0);
        }
        resArray.push_back(temp);
    }
}

int BaseDRAMSystem::GetChannel(uint64_t hex_addr) const {
    hex_addr >>= config_.shift_bits;
    return (hex_addr >> config_.ch_pos) & config_.ch_mask;
}

void BaseDRAMSystem::PrintEpochStats() {
    // first epoch, print bracket
    if (clk_ - config_.epoch_period == 0) {
        std::ofstream epoch_out(config_.json_epoch_name, std::ofstream::out);
        epoch_out << "[";
    }
    for (size_t i = 0; i < ctrls_.size(); i++) {
        ctrls_[i]->PrintEpochStats();
        std::ofstream epoch_out(config_.json_epoch_name, std::ofstream::app);
        epoch_out << "," << std::endl;
    }
#ifdef THERMAL
    thermal_calc_.PrintTransPT(clk_);
#endif  // THERMAL
    return;
}

bool BaseDRAMSystem::SetBuffer(uint64_t hex_addr, bool is_write, int flag) {
    if(flag == 1){
        buffer_a.waitAddr.push_back(hex_addr);
        buffer_a.isIn = true;
    }
    else if(flag == 2){
        buffer_b.waitAddr.push_back(hex_addr);
        buffer_b.isIn = true;
    }
    else{
        buffer_c.waitAddr.push_back(hex_addr);
        buffer_c.isIn = true;
    }
}

bool BaseDRAMSystem::ResetBuffer(int flag) {
    if(flag == 1){buffer_a.isIn = false;}
    else if(flag == 2){buffer_b.isIn = false;}
    else{buffer_c.isIn = false;}
}

uint64_t BaseDRAMSystem::getAddr(int flag){
    if(flag == 1){return buffer_a.waitAddr.front();}
    else if(flag == 2){return buffer_b.waitAddr.front();}
    else{return buffer_c.waitAddr.front();}
}

std::vector<std::vector<int>> BaseDRAMSystem::getallBuffer() {
    return resArray;
}

bool BaseDRAMSystem::getisIn(int flag){
    if(flag == 1){return buffer_a.getisIn();}
    else if(flag == 2){return buffer_b.getisIn();}
    else{return buffer_c.getisIn();}
}
/*
int BaseDRAMSystem::isEnd(){
    for(int i=0;i<ctrls_.size();i++){
        if(ctrls_[i]->freeUnit() != -1){
            return 0;
        }
    }
    return 1;
}*/

void BaseDRAMSystem::PrintStats() {
    // Finish epoch output, remove last comma and append ]
    std::ofstream epoch_out(config_.json_epoch_name, std::ios_base::in |
                                                         std::ios_base::out |
                                                         std::ios_base::ate);
    epoch_out.seekp(-2, std::ios_base::cur);
    epoch_out.write("]", 1);
    epoch_out.close();

    std::ofstream json_out(config_.json_stats_name, std::ofstream::out);
    json_out << "{";

    // close it now so that each channel can handle it
    json_out.close();
    for (size_t i = 0; i < ctrls_.size(); i++) {
        ctrls_[i]->PrintFinalStats();
        if (i != ctrls_.size() - 1) {
            std::ofstream chan_out(config_.json_stats_name, std::ofstream::app);
            chan_out << "," << std::endl;
        }
    }
    json_out.open(config_.json_stats_name, std::ofstream::app);
    json_out << "}";

#ifdef THERMAL
    thermal_calc_.PrintFinalPT(clk_);
#endif  // THERMAL
    for(int i=0;i<ctrls_.size();i++){
        ctrls_[i]->print_stall();
    }
}

void BaseDRAMSystem::ResetStats() {
    for (size_t i = 0; i < ctrls_.size(); i++) {
        ctrls_[i]->ResetStats();
    }
}

void BaseDRAMSystem::RegisterCallbacks(
    std::function<void(uint64_t)> read_callback,
    std::function<void(uint64_t)> write_callback) {
    // TODO this should be propagated to controllers
    read_callback_ = read_callback;
    write_callback_ = write_callback;
}

std::vector<std::vector<int>> BaseDRAMSystem::getBuffer(std::pair<int, int> index){
    std::vector<std::vector<int>> returning;
    for(int i=0;i<sys_row_;i++){
        if(i+sys_row_*index.second < resArray.size()){
            std::vector<int> temp;
            for(int j=0;j<row_;j++){
                temp.push_back(resArray[i+sys_row_*index.second][j]);
            }
            returning.push_back(temp);
        }
        else{break;}
    }
    return returning;
}

void BaseDRAMSystem::newBuffer(std::vector<std::vector<int>> r, std::pair<int, int> index){
    for(int i=0;i<r.size();i++){
        for(int j=0;j<r[i].size();j++){
            resArray[i+row_*index.second][j] = r[i][j];
        }
    }
}

JedecDRAMSystem::JedecDRAMSystem(Config &config, const std::string &output_dir,
                                 std::function<void(uint64_t)> read_callback,
                                 std::function<void(uint64_t)> write_callback, int row, int array, int sys_row)
    : BaseDRAMSystem(config, output_dir, read_callback, write_callback, row, array, sys_row) {
    if (config_.IsHMC()) {
        std::cerr << "Initialized a memory system with an HMC config file!"
                  << std::endl;
        AbruptExit(__FILE__, __LINE__);
    }

    ctrls_.reserve(config_.channels);
    for (auto i = 0; i < config_.channels; i++) {
#ifdef THERMAL
        ctrls_.push_back(new Controller(i, config_, timing_, thermal_calc_));
#else
        ctrls_.push_back(new Controller(i, config_, timing_));
#endif  // THERMAL
    }
}

JedecDRAMSystem::~JedecDRAMSystem() {
    for (auto it = ctrls_.begin(); it != ctrls_.end(); it++) {
        delete (*it);
    }
}

bool JedecDRAMSystem::WillAcceptTransaction(uint64_t hex_addr,
                                            bool is_write) const {
    int channel = GetChannel(hex_addr);
    return ctrls_[channel]->WillAcceptTransaction(hex_addr, is_write);
}

bool JedecDRAMSystem::AddTransaction(uint64_t hex_addr, bool is_write) {
// Record trace - Record address trace for debugging or other purposes
#ifdef ADDR_TRACE
    address_trace_ << std::hex << hex_addr << std::dec << " "
                   << (is_write ? "WRITE " : "READ ") << clk_ << std::endl;
#endif

    int channel = GetChannel(hex_addr);
    bool ok = ctrls_[channel]->WillAcceptTransaction(hex_addr, is_write);

    assert(ok);
    if (ok) {
        Transaction trans = Transaction(hex_addr, is_write);
        ctrls_[channel]->AddTransaction(trans);
    }
    last_req_clk_ = clk_;
    return ok;
}

void JedecDRAMSystem::ClockTick() {
    for (size_t i = 0; i < ctrls_.size(); i++) {
        // look ahead and return earlier
        while (true) {
            auto pair = ctrls_[i]->ReturnDoneTrans(clk_);
            if (pair.second == 1) {
                write_callback_(pair.first);
            } else if (pair.second == 0) {
                read_callback_(pair.first);
            } else {
                break;
            }
        }
    }
    for (size_t i = 0; i < ctrls_.size(); i++) {
        ctrls_[i]->ClockTick();
    }
    clk_++;

    if (clk_ % config_.epoch_period == 0) {
        PrintEpochStats();
    }
    return;
}

IdealDRAMSystem::IdealDRAMSystem(Config &config, const std::string &output_dir,
                                 std::function<void(uint64_t)> read_callback,
                                 std::function<void(uint64_t)> write_callback, int row, int array, int sys_row)
    : BaseDRAMSystem(config, output_dir, read_callback, write_callback, row, array, sys_row),
      latency_(config_.ideal_memory_latency) {}

IdealDRAMSystem::~IdealDRAMSystem() {}

bool IdealDRAMSystem::AddTransaction(uint64_t hex_addr, bool is_write) {
    auto trans = Transaction(hex_addr, is_write);
    trans.added_cycle = clk_;
    infinite_buffer_q_.push_back(trans);
    return true;
}

void IdealDRAMSystem::ClockTick() {
    for (auto trans_it = infinite_buffer_q_.begin();
         trans_it != infinite_buffer_q_.end();) {
        if (clk_ - trans_it->added_cycle >= static_cast<uint64_t>(latency_)) {
            if (trans_it->is_write) {
                write_callback_(trans_it->addr);
            } else {
                read_callback_(trans_it->addr);
            }
            trans_it = infinite_buffer_q_.erase(trans_it++);
        }
        if (trans_it != infinite_buffer_q_.end()) {
            ++trans_it;
        }
    }

    clk_++;
    return;
}

}  // namespace dramsim3
