#include "memory_system.h"

namespace dramsim3 {
MemorySystem::MemorySystem(const std::string &config_file,
                           const std::string &output_dir,
                           std::function<void(uint64_t)> read_callback,
                           std::function<void(uint64_t)> write_callback, int row, int array, int sys_row)
    : config_(new Config(config_file, output_dir)) {
    // TODO: ideal memory type?
    if (config_->IsHMC()) {
        dram_system_ = new HMCMemorySystem(*config_, output_dir, read_callback,
                                           write_callback, row, array, sys_row);
    } else {
        dram_system_ = new JedecDRAMSystem(*config_, output_dir, read_callback,
                                           write_callback, row, array, sys_row);
    }
}

MemorySystem::~MemorySystem() {
    delete (dram_system_);
    delete (config_);
}

void MemorySystem::ClockTick() { dram_system_->ClockTick(); }

double MemorySystem::GetTCK() const { return config_->tCK; }

int MemorySystem::GetBusBits() const { return config_->bus_width; }

int MemorySystem::GetBurstLength() const { return config_->BL; }

int MemorySystem::GetQueueSize() const { return config_->trans_queue_size; }

void MemorySystem::RegisterCallbacks(
    std::function<void(uint64_t)> read_callback,
    std::function<void(uint64_t)> write_callback) {
    dram_system_->RegisterCallbacks(read_callback, write_callback);
}

std::vector<std::vector<int>> MemorySystem::getBuffer(std::pair<int, int> index){
    return dram_system_->getBuffer(index);
}

void MemorySystem::newBuffer(std::vector<std::vector<int>> r, std::pair<int, int> index){
    dram_system_->newBuffer(r, index);
}

bool MemorySystem::WillAcceptTransaction(uint64_t hex_addr,
                                         bool is_write) const {
    return dram_system_->WillAcceptTransaction(hex_addr, is_write);
}

bool MemorySystem::AddTransaction(uint64_t hex_addr, bool is_write) {
    return dram_system_->AddTransaction(hex_addr, is_write);
}

void MemorySystem::printBuff(){
    dram_system_->printBuff();
}

void MemorySystem::modifyInfo(int row, int array){
    dram_system_->modifyInfo(row, array);
}

bool MemorySystem::SetBuffer(uint64_t hex_addr, bool is_write, int flag){
    return dram_system_->SetBuffer(hex_addr, is_write, flag);
}

bool MemorySystem::ResetBuffer(int flag){
    return dram_system_->ResetBuffer(flag);
}

bool MemorySystem::getisIn(int flag){
    return dram_system_->getisIn(flag);
}

uint64_t MemorySystem::getAddr(int flag){
    return dram_system_->getAddr(flag);
}

std::vector<std::vector<int>> MemorySystem::getallBuffer() {
    return dram_system_->getallBuffer();
}

void MemorySystem::PrintStats() const { dram_system_->PrintStats(); }

void MemorySystem::ResetStats() { dram_system_->ResetStats(); }

MemorySystem* GetMemorySystem(const std::string &config_file, const std::string &output_dir,
                 std::function<void(uint64_t)> read_callback,
                 std::function<void(uint64_t)> write_callback, int row, int array, int sys_row) {
    return new MemorySystem(config_file, output_dir, read_callback, write_callback, row, array, sys_row);
}
}  // namespace dramsim3

// This function can be used by autoconf AC_CHECK_LIB since
// apparently it can't detect C++ functions.
// Basically just an entry in the symbol table
extern "C" {
void libdramsim3_is_present(void) { ; }
}
