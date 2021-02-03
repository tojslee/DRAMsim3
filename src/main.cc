#include <iostream>
#include <fstream>
#include <string>
#include "./../ext/headers/args.hxx"
#include "cpu.h"

using namespace dramsim3;

void printArr(std::vector<std::vector<std::vector<std::vector<int>>>> oneArray){
    for(auto i=oneArray.begin();i!=oneArray.end();i++){
        auto j=*i;
        for(auto k=j.begin();k!=j.end();k++){
            auto l=*k;
            for(auto s=l.begin();s!=l.end();s++){
                auto d=*s;
                for(auto a=d.begin();a!=d.end();a++){
                    std::cout<<*a<<" ";
                }
                std::cout<<std::endl;
            }
            std::cout<<std::endl;
        }
        std::cout<<std::endl;
    }
    std::cout<<std::endl;
}

int main(int argc, const char **argv) {
    args::ArgumentParser parser(
        "DRAM Simulator.",
        "Examples: \n."
        "./build/dramsim3main configs/DDR4_8Gb_x8_3200.ini -c 100 -t "
        "sample_trace.txt\n"
        "./build/dramsim3main configs/DDR4_8Gb_x8_3200.ini -s random -c 100");
    args::HelpFlag help(parser, "help", "Display the help menu", {'h', "help"});
    args::ValueFlag<uint64_t> num_cycles_arg(parser, "num_cycles",
                                             "Number of cycles to simulate",
                                             {'c', "cycles"}, 100000);
    args::ValueFlag<std::string> output_dir_arg(
        parser, "output_dir", "Output directory for stats files",
        {'o', "output-dir"}, ".");
    args::ValueFlag<std::string> stream_arg(
        parser, "stream_type", "address stream generator - (random), stream",
        {'s', "stream"}, "");
    args::ValueFlag<std::string> trace_file_arg(
        parser, "trace",
        "Trace file, setting this option will ignore -s option",
        {'t', "trace"});
    args::ValueFlag<std::int32_t> unit_arg(
            parser, "unit_", "LD, ST unit",
            {'u', "unit"}, 100
            );
    args::ValueFlag<std::int32_t> row_arg(
            parser, "row_", "Systolic array row num",
            {'r', "row"}, 4
            );
    args::ValueFlag<std::int32_t> column_arg(
            parser, "column_", "Systolic array column num",
            {"col", "column"}, 4
            );
    args::ValueFlag<std::int32_t> array_length(
            parser, "array_", "calculating A array length",
            {'a', "array"}, 5
            );
    args::ValueFlag<std::int32_t> array_height(
            parser, "array_height_", "calculating A array height",
            {'h', "height"}, 10
            );
    args::ValueFlag<std::int32_t> b_row(
            parser, "brow_", "calculating B array length",
            {'b', "brow"}, 10
            );
    args::ValueFlag<std::int32_t> b_column(
            parser, "bcol_", "calculating B array height",
            {'l', "bcol"}, 3
            );
    args::ValueFlag<std::string> dnn_config(
            parser, "dnn_config_", "DNN config file name",
            {"con"}, "/Users/jisoo/CLionProjects/revDRAMsim3/input.txt"
            );
    args::Positional<std::string> config_arg(
        parser, "config", "The config file name (mandatory)");

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help) {
        std::cout << parser;
        return 0;
    } catch (args::ParseError e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    std::string config_file = args::get(config_arg);
    if (config_file.empty()) {
        std::cerr << parser;
        return 1;
    }

    uint64_t cycles = args::get(num_cycles_arg);
    std::string output_dir = args::get(output_dir_arg);
    std::string trace_file = args::get(trace_file_arg);
    std::string stream_type = args::get(stream_arg);
    std::int32_t units_ = args::get(unit_arg);
    std::int32_t row_ = args::get(row_arg);
    std::int32_t column_ = args::get(column_arg);
    std::int32_t array_ = args::get(array_length);
    std::int32_t array_height_ = args::get(array_height);
    std::int32_t brow_ = args::get(b_row);
    std::int32_t bcol_ = args::get(b_column);
    std::string dnn_config_file = args::get(dnn_config);

    CPU *cpu;
    std::vector<layerInfo> DNNLayers;
    std::ifstream dnnTrace;
    dnnTrace.open(dnn_config_file);
    if(dnnTrace.is_open()){
        std::string str;
        getline(dnnTrace, str);
        int layerNum = std::stoi(str);
        //printf("%d\n", layerNum);
        for(int i=0;i<layerNum;i++){ // get layer Info
            getline(dnnTrace, str);
            // type, minibatch, aRow, aCol, k, filterNum, bRow, bCol, optimal
            size_t prev = 0;
            std::vector<std::string> parse;
            for(int j=0;j<str.size();j++){
                if(isblank(str[j])){
                    parse.push_back(str.substr(prev, j-prev));
                    prev = j+1;
                }
            }
            parse.push_back(str.substr(prev, str.size()-prev));
            if(parse[0].compare("Conv") == 0){
                layerInfo oneLayer(parse[0], stoi(parse[1]), stoi(parse[2]), stoi(parse[3]), stoi(parse[4]), stoi(parse[5]), stoi(parse[6]), stoi(parse[7]), parse[8]);
                DNNLayers.push_back(oneLayer);
            }
            else if(parse[0].compare("FC") == 0){
                layerInfo oneLayer(parse[0], 1, 1, stoi(parse[1]), 1 , 1, stoi(parse[1]), stoi(parse[2]), parse[3]);
                DNNLayers.push_back(oneLayer);
            }
        }
        // get Input array & Filters
        std::vector<std::vector<std::vector<std::vector<int>>>> oneArray;
        for(int i=0;i<DNNLayers[0].getMinibatch();i++){
            // get input array
            // row : aRow & col : k * aCol
            // parse : integer vector containing jth row of each array
            // length -> k * aCol
            std::vector<std::int32_t> parse;
            std::vector<std::vector<std::vector<int>>> arrVector;
            for(int j=0;j<DNNLayers[0].getK();j++){
                std::vector<std::vector<int>> temp;
                arrVector.push_back(temp);
            }
            for(int j=0;j<DNNLayers[0].getaRow();j++){
                getline(dnnTrace,str);
                size_t prev = 0;
                for(int l=0;l<str.size();l++){
                    if(isblank(str[l])){
                        parse.push_back(stoi(str.substr(prev, l-prev)));
                        prev = l+1;
                    }
                }
                parse.push_back(stoi(str.substr(prev, str.size()-prev)));

                for(int l=0;l<DNNLayers[0].getK();l++){
                    std::vector<int> t;
                    for(int s=0;s<DNNLayers[0].getaCol();s++){
                        t.push_back(parse[l*DNNLayers[0].getaCol()+s]);
                    }
                    arrVector[l].push_back(t);
                }
                parse.clear();
            }
            oneArray.push_back(arrVector);
        }
        DNNLayers[0].setarrayA(oneArray);
        printArr(oneArray);
        oneArray.clear();

        // get filter array for each layer
        for(int i=0;i<layerNum;i++){
            for(int j=0;j<DNNLayers[i].getFilterNum();j++){
                // get filter array
                // row : bRow & col : k * bCol
                // parse : integer vector containing lth row of each filter
                // length -> k * bCol
                std::vector<std::int32_t> parse;
                std::vector<std::vector<std::vector<int>>> arrVector;
                for(int l=0;l<DNNLayers[i].getK();l++){
                    std::vector<std::vector<int>> temp;
                    arrVector.push_back(temp);
                }
                for(int l=0;l<DNNLayers[i].getbRow();l++){
                    getline(dnnTrace, str);
                    size_t prev = 0;
                    for(int s=0;s<str.size();s++){
                        if(isblank(str[s])){
                            parse.push_back(stoi(str.substr(prev, s-prev)));
                            prev = s+1;
                        }
                    }
                    parse.push_back(stoi(str.substr(prev, str.size()-prev)));

                    for(int s=0;s<DNNLayers[i].getK();s++){
                        std::vector<int> t;
                        for(int d=0;d<DNNLayers[i].getbCol();d++){
                            t.push_back(parse[s*DNNLayers[i].getbCol()+d]);
                        }
                        arrVector[s].push_back(t);
                    }
                    parse.clear();
                }
                oneArray.push_back(arrVector);
            }
            DNNLayers[i].setarrayB(oneArray);
            printArr(oneArray);
            oneArray.clear();
        }
    }
    dnnTrace.close();

    int currentLayer = 0;

    if (!trace_file.empty()) {
        cpu = new TraceBasedCPU(config_file, output_dir, trace_file, units_, row_, column_, array_, array_height_, brow_, bcol_);
    } else {
        if (stream_type == "stream" || stream_type == "s") {
            if(DNNLayers.size() != 0){
                cpu = new StreamCPU(config_file, output_dir, units_, row_, column_, DNNLayers[0].getaRow(), DNNLayers[0].getaCol(), DNNLayers[0].getbRow(), DNNLayers[0].getbCol(), currentLayer, DNNLayers);
            }
            else{
                cpu = new StreamCPU(config_file, output_dir, units_, row_, column_, array_, array_height_, brow_, bcol_, currentLayer, DNNLayers);
            }
        } else {
            cpu = new RandomCPU(config_file, output_dir, units_, row_, column_, array_, array_height_, brow_, bcol_);
        }
    }

    while(true){
        if(cpu->ClockTick()){
            break;
        }
    }
    cpu->PrintStats();
    //cpu->PrintStall();

    delete cpu;

    return 0;
}
