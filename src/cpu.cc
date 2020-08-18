#include "cpu.h"

namespace dramsim3 {

bool RandomCPU::ClockTick() {
    // Create random CPU requests at full speed
    // this is useful to exploit the parallelism of a DRAM protocol
    // and is also immune to address mapping and scheduling policies
    memory_system_.ClockTick();
    if (get_next_) {
        last_addr_ = gen();
        last_write_ = (gen() % 3 == 0);
    }
    get_next_ = memory_system_.WillAcceptTransaction(last_addr_, last_write_);
    if (get_next_) {
        memory_system_.AddTransaction(last_addr_, last_write_);
    }
    clk_++;
    if(!get_next_){return true;}
    return false;
}

void CPU::ReadCallBack(uint64_t addr) {
    readCallBacks.push_back(addr);
    for(auto iter = allUnits.begin();iter != allUnits.end();iter++){
        if(iter->addr == addr){
            iter->occupied = false;
        }
    }
    return;
}

int CPU::freeUnit(){
    for(int i=0;i<units_;i++){
        if(!allUnits[i].isOccupied()){
            return i;
        }
    }
    return -1;
}

bool StreamCPU::ClockTick() {
    // stream-add, read 2 arrays, add them up to the third array
    // this is a very simple approximate but should be able to produce
    // enough buffer hits

    // moving on to next set of arrays
    memory_system_.ClockTick();
    if (offset_ >= array_size_ ||clk_ == 0) {
        addr_a_ = gen();
        addr_b_ = gen();
        addr_c_ = gen();
        offset_ = 0;
    }

    int idx = freeUnit();
    if (!inserted_a_ && idx != -1 &&
        memory_system_.WillAcceptTransaction(addr_a_ + offset_, false)) {
        memory_system_.AddTransaction(addr_a_ + offset_, false);
        inserted_a_ = true;
        allUnits[idx].occupied = true;
        allUnits[idx].addr = addr_a_ + offset_;
    }

    idx = freeUnit();
    if (!inserted_b_ && idx != -1 &&
        memory_system_.WillAcceptTransaction(addr_b_ + offset_, false)) {
        memory_system_.AddTransaction(addr_b_ + offset_, false);
        inserted_b_ = true;
        allUnits[idx].occupied = true;
        allUnits[idx].addr = addr_b_ + offset_;
    }

    auto iter = readCallBacks.begin();
    while(iter != readCallBacks.end()){
        if(*iter == addr_a_ + offset_){
            memory_system_.SetBuffer(addr_a_ + offset_, false, 1);
            readCallBacks.erase(iter);
        }
        else if(*iter == addr_b_ + offset_){
            memory_system_.SetBuffer(addr_b_ + offset_, false, 2);
            readCallBacks.erase(iter);
        }
        else{iter++;}
    }

    // adder, multiplier
    if(memory_system_.getisIn(1) && add_.getCounter() == 0){
        // can add in 1 cycle
        // adder class isWorking true
        add_.setWorking();
        add_.addCounter();
    }
    else if(memory_system_.getisIn(1) && add_.getCounter() < add_.getCycles()){add_.addCounter();}

    if(memory_system_.getisIn(2) && multiple_.getCounter() == 0 && add_.getCounter() == add_.getCycles()){
        // can multiply in 8 cycle
        // multiplier class isWorking true
        multiple_.addCounter();
    }
    else if(memory_system_.getisIn(2) && add_.getCounter() == add_.getCycles() && multiple_.getCounter() < multiple_.getCycles()){multiple_.addCounter();}

    // write in buffer if all adder, multiplier done
    if(multiple_.getCounter() == multiple_.getCycles() && add_.getCounter() == add_.getCycles()){
        memory_system_.SetBuffer(addr_c_ + offset_, true, 3);
    }

    // if there is data ready in the buffer
    if (!inserted_c_ && memory_system_.getisIn(3) &&
        memory_system_.WillAcceptTransaction(addr_c_ + offset_, true)) {
        memory_system_.AddTransaction(addr_c_ + offset_, true);
        inserted_c_ = true;
        memory_system_.ResetBuffer(3);
    }

    iter = writeCallBacks.begin();
    while(iter != writeCallBacks.end()){
        if(memory_system_.getisIn(3) && *iter == memory_system_.getAddr(3)){
            writeCallBacks.erase(iter);
            memory_system_.ResetBuffer(3);
        }
        else{iter++;}
    }
    std::cout<<clk_<<" "<<add_.getCounter()<<" "<<multiple_.getCounter()<<" "<<offset_<<std::endl;

    // moving on to next element
    if (inserted_a_ && inserted_b_ && inserted_c_ && readCallBacks.empty()) {
        offset_ += stride_;
        inserted_a_ = false;
        inserted_b_ = false;
        inserted_c_ = false;
        memory_system_.ResetBuffer(1);
        memory_system_.ResetBuffer(2);
        add_.resetWorking();
        multiple_.resetWorking();
        add_.resetCounter();
        multiple_.resetCounter();
    }
    else{
        stall_counter_++;
    }
    clk_++;

    if(offset_ >= array_size_){
        return true;
    }
    return false;
}

TraceBasedCPU::TraceBasedCPU(const std::string& config_file,
                             const std::string& output_dir,
                             const std::string& trace_file, const int32_t units_)
    : CPU(config_file, output_dir, units_) {
    trace_file_.open(trace_file);
    if (trace_file_.fail()) {
        std::cerr << "Trace file does not exist" << std::endl;
        AbruptExit(__FILE__, __LINE__);
    }
}

bool TraceBasedCPU::ClockTick() {
    //std::cout<<clk_<<" ";
    memory_system_.ClockTick();
    bool end = isEnd();
    //std::cout<<clk_;
    if (!trace_file_.eof()) {
        if (get_next_ && !stalled_) {
            get_next_ = false;
            trace_file_ >> trans_;
            trans_.added_cycle = trans_.added_cycle/10;
        }
        //std::cout<<" "<<trans_.added_cycle;
        if (trans_.added_cycle <= clk_ + stall_counter_) {
            get_next_ = memory_system_.WillAcceptTransaction(trans_.addr,
                                                             trans_.is_write);
            int idx = freeUnit();
            if(trans_.is_write){
                if(get_next_){
                    memory_system_.AddTransaction(trans_.addr, trans_.is_write);
                }
                else{stall_counter_++;}
            }
            else{
                if (get_next_ && idx != -1) {
                    memory_system_.AddTransaction(trans_.addr, trans_.is_write);
                    allUnits[idx].addr = trans_.addr;
                    allUnits[idx].occupied = true;
                    stalled_ = false;
                    std::cout<<clk_<<" "<<idx<<" "<<trans_.added_cycle<<std::endl;
                }
                else{
                    stalled_ = true;
                    stall_counter_++;
                }
            }
            //std::cout<<trans_.added_cycle<<" "<<idx<<" ";
            //std::cout<<clk_<<" "<<idx<<" "<<trans_.added_cycle<<std::endl;
            //std::cout<<" "<<idx;
        }
        //std::cout<<std::endl;
    }
    else{
        if(end){return true;}
    }
    //std::cout<<clk_<<" "<<stalled_<<std::endl;
    clk_++;
    //std::cout<<std::endl;
    return false;
}

bool CPU::isEnd(){
    for(auto iter = allUnits.begin();iter!=allUnits.end();iter++){
        if(iter->isOccupied()){return false;}
    }
    return true;
}

}  // namespace dramsim3
