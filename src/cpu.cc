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
    if (offset_a_ >= array_size_ ||clk_ == 0) {
        addr_a_ = gen();
        addr_b_ = gen();
        addr_c_ = gen();
        offset_a_ = 0;
        offset_b_ = 0;
        offset_c_ = 0;
    }
    int idx = freeUnit();
    if(!inserted_a_ &&
        !systolic_array.endGetInput(1)){
        if(memory_system_.WillAcceptTransaction(addr_a_ + offset_a_, false)){
            memory_system_.AddTransaction(addr_a_ + offset_a_, false);
            systolic_array.getInput(1, addr_a_, offset_a_);
            allUnits[idx].occupied = true;
            allUnits[idx].addr = addr_a_ + offset_a_;
            offset_a_ += stride_;
        }
    }
    else{
        if(inserted_b_ && !inserted_a_){counter_++;}
        inserted_a_ = true;
    }

    idx = freeUnit();
    if(!inserted_b_ &&
        !systolic_array.endGetInput(2)){
        if(memory_system_.WillAcceptTransaction(addr_b_ + offset_b_, false)){
            memory_system_.AddTransaction(addr_b_ + offset_b_, false);
            systolic_array.getInput(2, addr_b_, offset_b_);
            allUnits[idx].occupied = true;
            allUnits[idx].addr = addr_b_ + offset_b_;
            offset_b_ += stride_;
        }
    }
    else{
        if(inserted_a_ && !inserted_b_){counter_++;}
        inserted_b_ = true;
    }


    auto iter = readCallBacks.begin();
    std::vector<uint64_t> temp = systolic_array.getAddr(1);
    std::vector<uint64_t> temp2 = systolic_array.getAddr(2);
    while(iter != readCallBacks.end()){
        auto iterator = temp.begin();
        auto iterator1 = temp2.begin();
        while(iterator != temp.end()){
            if(*iterator == *iter){
                temp.erase(iterator);
                systolic_array.addBuffer(1);
                readCallBacks.erase(iter);
            }
            else{iterator++;}
        }
        while(iterator1 != temp2.end()){
            if(*iterator1 == *iter){
                temp2.erase(iterator1);
                systolic_array.addBuffer(2);
                readCallBacks.erase(iter);
            }
            else{iterator1++;}
        }
    }
    systolic_array.setAddr(1, temp);
    systolic_array.setAddr(2, temp2);

    bool endCal = false;
    if(systolic_array.fullBuffer(1) && systolic_array.fullBuffer(2)){
        // calculate matrix mul
        endCal = systolic_array.matrixMultiple();
    }

    if(endCal && !inserted_c_){
        // write
        // offset, buffer num = 0, ..initialization
        while(systolic_array.getNums(3) != 0){
            if(memory_system_.WillAcceptTransaction(addr_c_ + offset_c_, true)){
                memory_system_.AddTransaction(addr_c_ + offset_c_, true);
                systolic_array.subNums(3);
                offset_c_ += stride_;
            }
        }
        if(systolic_array.getNums(3) == 0){
            inserted_c_ = true;
            systolic_array.bufferReset(3);
        }
    }

    if(writeCallBacks.size() == row_){writeCallBacks.clear();}

    /*
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
*/
    // moving on to next element
    if (inserted_a_ && inserted_b_ && inserted_c_ && readCallBacks.empty() && writeCallBacks.empty() && writeStart_) {
        //offset_ += stride_;
        writeStart_ = false;
        inserted_a_ = false;
        inserted_b_ = false;
        inserted_c_ = false;
        if(16/col_ > counter_){
            offset_a_ -= row_ * stride_;
            offset_b_ -= row_ * stride_;
        }
        else{counter_ = 0;}
        systolic_array.bufferReset(1);
        systolic_array.bufferReset(2);
        //memory_system_.ResetBuffer(1);
        //memory_system_.ResetBuffer(2);
        //add_.resetWorking();
        //multiple_.resetWorking();
        //add_.resetCounter();
        //multiple_.resetCounter();
        if(offset_a_ >= array_size_){
            return true;
        }
    }
    else{
        stall_counter_++;
    }
    std::cout<<clk_<<std::endl;
    clk_++;


    return false;
}

TraceBasedCPU::TraceBasedCPU(const std::string& config_file,
                             const std::string& output_dir,
                             const std::string& trace_file, const int32_t units_, const std::int32_t row_, const std::int32_t column_)
    : CPU(config_file, output_dir, units_, row_, column_) {
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
