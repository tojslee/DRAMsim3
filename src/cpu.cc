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

void CPU::fixA(std::vector<std::vector<int>> v){
    systolic_array.arrayA.clear();
    systolic_array.arrayA.assign(v.begin(), v.end());
}

void CPU::fixB(std::vector<std::vector<int>> v){
    systolic_array.arrayB.clear();
    systolic_array.arrayB.assign(v.begin(), v.end());
}

void CPU::fixWeight(std::vector<std::vector<int>> v){
    if(v.size() < 1){
        systolic_array.setUsage(0, 0);
    }
    else{
        systolic_array.setUsage(v.size(), v[0].size());
    }
    systolic_array.firstPE->fixWeight(v, systolic_array.firstPE);
}

void StreamCPU::im2Col(){
    std::vector<std::vector<int>> FCarrayA;
    std::vector<std::vector<int>> FCarrayB;
    std::vector<std::vector<std::vector<std::vector<int>>>> arrayA = DNNLayers[currentLayer].getarrayA();
    std::vector<std::vector<std::vector<std::vector<int>>>> arrayB = DNNLayers[currentLayer].getarrayB();

    // arrayA
    for(int i=0;i<DNNLayers[currentLayer].getMinibatch();i++){ // minibatch
        for(int j=0;j<DNNLayers[currentLayer].getaRow()-DNNLayers[currentLayer].getbRow()+1;j++){ // arow
            for(int k=0;k<DNNLayers[currentLayer].getaCol()-DNNLayers[currentLayer].getbCol()+1;k++){ // acol
                std::vector<int> temp;
                for(int l=0;l<DNNLayers[currentLayer].getK();l++){
                    for(int m=0;m<DNNLayers[currentLayer].getbRow();m++){
                        for(int n=0;n<DNNLayers[currentLayer].getbCol();n++){
                            temp.push_back(arrayA[i][l][j+m][k+n]);
                        }
                    }
                }
                FCarrayA.push_back(temp);
            }
        }
    }

    // arrayB x, y대칭
    std::vector<std::vector<int>> FCtilted;
    for(auto i=arrayB.begin();i!=arrayB.end();i++){ // filter Num
        auto j = *i;
        std::vector<int> temp;
        for(auto k=j.begin();k!=j.end();k++){ // k
            auto l=*k;
            for(auto m=l.begin();m!=l.end();m++){ // brow
                auto n=*m;
                for(auto o=n.begin();o!=n.end();o++){ // bcol
                    temp.push_back(*o);
                }
            }
        }
        FCtilted.push_back(temp);
    }

    // FCtilted -> FCarrayB
    for(int i=0;i<FCtilted[0].size();i++){
        std::vector<int> temp;
        for(int j=0;j<FCtilted.size();j++){
            temp.push_back(FCtilted[j][i]);
        }
        FCarrayB.push_back(temp);
    }

    for(int i=0;i<FCarrayB.size();i++){
        for(int j=0;j<FCarrayB[0].size();j++){
            std::cout<<FCarrayB[i][j]<<" ";
        }
        std::cout<<std::endl;
    }
    std::cout<<std::endl;

    fixA(FCarrayA);
    fixB(FCarrayB);

    // array_length, array_height, brow, bcol 수정
    // systolic array, cpu, memory
    array_length = FCarrayA.size();
    array_height = FCarrayA[0].size();
    brow = FCarrayB.size();
    bcol = FCarrayB[0].size();
    systolic_array.modifyInfo(array_length, array_height);
    memory_system_.modifyInfo(array_length, bcol);
}

void StreamCPU::FCLayer(){
    std::vector<std::vector<int>> FCarrayA;
    std::vector<int> temp;
    FCarrayA.push_back(temp);
    std::vector<std::vector<std::vector<std::vector<int>>>> arrayA = DNNLayers[currentLayer].getarrayA();
    std::vector<std::vector<std::vector<std::vector<int>>>> arrayB = DNNLayers[currentLayer].getarrayB();

    // arrayA -> 1*N
    for(auto i=arrayA.begin();i!=arrayA.end();i++){
        auto j = *i;
        for(auto k=j.begin();k!=j.end();k++){
            auto l=*k;
            for(auto m=l.begin();m!=l.end();m++){
                auto n=*m;
                for(auto o=n.begin();o!=n.end();o++){
                    FCarrayA[0].push_back(*o);
                }
            }
        }
    }

    // araryB(1*1*N*P) -> N*P
    std::vector<std::vector<int>> FCarrayB;
    FCarrayB.assign(arrayB[0][0].begin(), arrayB[0][0].end());

    for(int i=0;i<FCarrayA.size();i++){
        for(int j=0;j<FCarrayA[0].size();j++){
            std::cout<<FCarrayA[i][j]<<" ";
        }
        std::cout<<std::endl;
    }
    std::cout<<std::endl;

    for(int i=0;i<FCarrayB.size();i++){
        for(int j=0;j<FCarrayB[0].size();j++){
            std::cout<<FCarrayB[i][j]<<" ";
        }
        std::cout<<std::endl;
    }
    std::cout<<std::endl;

    array_length = FCarrayA.size();
    array_height = FCarrayA[0].size();
    brow = FCarrayB.size();
    bcol = FCarrayB[0].size();
    systolic_array.modifyInfo(array_length, array_height);
    memory_system_.modifyInfo(array_length, bcol);

    // input Filter
    fixA(FCarrayA);
    fixB(FCarrayB);
}

bool StreamCPU::lastIdx(){
    return (index.first >= (brow-1)/row_ && index.second >= (bcol-1)/row_);
}

bool StreamCPU::ClockTick() {
    // stream-add, read 2 arrays, add them up to the third array
    // this is a very simple approximate but should be able to produce
    // enough buffer hits

    // moving on to next set of arrays
    memory_system_.ClockTick();
    if (offset_a_ > array_size_ ||clk_ == 0 || newLayer) {
        addr_a_ = gen();
        addr_b_ = gen();
        addr_c_ = gen();
        offset_a_ = 0;
        offset_b_ = 0;
        offset_c_ = 0;
        systolic_array.reset();

        if(DNNLayers[currentLayer].getType().compare("FC") == 0){
            FCLayer();
        }
        else if(DNNLayers[currentLayer].getType().compare("Conv") == 0){
            im2Col();
        }

        array_size_ = array_length *array_height;

        // first 4*4 fix weight
        std::vector<std::vector<int>> v;
        for(int i=0;i<row_;i++){
            if(i+index.first*row_ < systolic_array.arrayB.size()){
                std::vector<int> temp;
                for(int j=0;j<row_;j++){
                    if(j+index.second*row_ < bcol){
                        temp.push_back(systolic_array.arrayB[i+index.first*row_][j+index.second*row_]);
                    }
                }
                v.push_back(temp);
            }
        }
        fixWeight(v);

        newLayer = false;
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

    // finishing all reading
    if(readCallBacks.size() == array_length * array_height){
        auto iter = readCallBacks.begin();
        std::vector<uint64_t> temp = systolic_array.getAddr(1);
        while(iter != readCallBacks.end()){
            auto iterator = temp.begin();
            while(iterator != temp.end()){
                if(*iterator == *iter){
                    temp.erase(iterator);
                    systolic_array.addBuffer(1);
                    readCallBacks.erase(iter);
                }
                else{iterator++;}
            }
        }
    }


    if(systolic_array.fullBuffer(1) && !endprop){
        // calculate matrix mul
        endCal = systolic_array.matrixMultiple();
        endprop = systolic_array.propagation(index);
    }

    // next 4*4 weight of B && get buffered calculated array from memory
    if(endprop && !lastIdx()){
        std::vector<std::vector<int>> buffering = systolic_array.getR();
        memory_system_.newBuffer(buffering, index);

        if(index.second >= (bcol-1)/row_){
            index.first += 1;
            index.second = 0;
            endprop = false;
        }
        else{
            index.second += 1;
            endprop = false;
        }

        std::vector<std::vector<int>> buffered = memory_system_.getBuffer(index);
        systolic_array.setR(buffered);

        // fix new Weight of B
        std::vector<std::vector<int>> v;
        for(int i=0;i<row_;i++){
            if(i+index.first*row_ < systolic_array.arrayB.size()){
                std::vector<int> temp;
                for(int j=0;j<row_;j++){
                    if(j+index.second*row_ < bcol){
                        temp.push_back(systolic_array.arrayB[i+index.first*row_][j+index.second*row_]);
                    }
                }
                v.push_back(temp);
            }
        }
        fixWeight(v);
    }


    if(endprop && !inserted_c_ && lastIdx()){
        // write
        // offset, buffer num = 0, ..initialization
        while(systolic_array.getNums(3) != 0){
            if(memory_system_.WillAcceptTransaction(addr_c_ + offset_c_, true)){
                memory_system_.AddTransaction(addr_c_ + offset_c_, true);
                systolic_array.subNums(3);
                offset_c_ += stride_;
            }
            else{
                break;
            }
        }
        if(systolic_array.getNums(3) == 0){
            inserted_c_ = true;
            systolic_array.bufferReset(3);
        }
    }

    if(writeCallBacks.size() == array_length * bcol){writeCallBacks.clear();}

    // moving on to next element
    if (inserted_a_ && inserted_c_ && readCallBacks.empty() && writeCallBacks.empty() && writeStart_) {
        //offset_ += stride_;
        writeStart_ = false;
        inserted_a_ = false;
        inserted_c_ = false;

        endCal = false;
        endprop = false;
        systolic_array.bufferReset(1);
        std::vector<std::vector<int>> buffering = systolic_array.getR();
        memory_system_.newBuffer(buffering, index);
        std::cout<<std::endl;
        memory_system_.printBuff();
        if(currentLayer + 1 < DNNLayers.size()){
            std::vector<std::vector<int>> res = memory_system_.getallBuffer();
            std::vector<std::vector<std::vector<std::vector<int>>>> nextarrayA;
            // put result array into next Layer
            if(DNNLayers[currentLayer].getType().compare("Conv") == 0){
                // 1row -> minibatch * (aRow-bRow+1) * (aCol-bCol+1)
                // # of rows : filterNum
                // minibatch * FilterNum * (aRow-bRow+1) * (aCol-bCol+1)
                for(int i=0;i<DNNLayers[currentLayer].getMinibatch();i++){
                    std::vector<std::vector<std::vector<int>>> thirdtemp;
                    for(int j=0;j<DNNLayers[currentLayer].getFilterNum();j++){
                        std::vector<std::vector<int>> secondtemp;
                        for(int l=0;l<DNNLayers[currentLayer].getaRow()-DNNLayers[currentLayer].getbRow()+1;l++){
                            std::vector<int> temp;
                            for(int m=0;m<DNNLayers[currentLayer].getaCol()-DNNLayers[currentLayer].getbCol()+1;m++){
                                int unit = DNNLayers[currentLayer].getaRow()-DNNLayers[currentLayer].getbRow() + DNNLayers[currentLayer].getaCol()-DNNLayers[currentLayer].getbCol()+2;
                                temp.push_back(res[j][i*unit+l+m]);
                            }
                            secondtemp.push_back(temp);
                        }
                        thirdtemp.push_back(secondtemp);
                    }
                    nextarrayA.push_back(thirdtemp);
                }
            }

            for(auto i=nextarrayA.begin();i!=nextarrayA.end();i++){
                auto j = *i;
                for(auto k=j.begin();k!=j.end();k++){
                    auto l=*k;
                    for(auto m=l.begin();m!=l.end();m++){
                        auto n=*m;
                        for(auto o=n.begin();o!=n.end();o++){
                            std::cout<<*o<<" ";
                        }
                        std::cout<<std::endl;
                    }
                    std::cout<<std::endl;
                }
                std::cout<<std::endl;
            }
            std::cout<<std::endl;


            /*else if(DNNLayers[currentLayer].getType().compare("FC") == 0){

            }*/
            DNNLayers[currentLayer+1].setarrayA(nextarrayA);

            // init
            newLayer = true;
            currentLayer++;
            index = std::make_pair(0, 0);

            clk_++;
            return false;
        }
        return true;
    }
    else{
        stall_counter_++;
    }
    clk_++;


    return false;
}

TraceBasedCPU::TraceBasedCPU(const std::string& config_file,
                             const std::string& output_dir,
                             const std::string& trace_file, const int32_t units_, const std::int32_t row_, const std::int32_t column_,
                             const std::int32_t array_, const std::int32_t array_height, const std::int32_t brow_, const std::int32_t bcol_)
    : CPU(config_file, output_dir, units_, row_, column_, array_, array_height, brow_, bcol_) {
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
                    //std::cout<<clk_<<" "<<idx<<" "<<trans_.added_cycle<<std::endl;
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
        if(end){
            return true;
        }
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
