#include <iostream>
#include "./../ext/headers/args.hxx"
#include "cpu.h"

using namespace dramsim3;

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
            {'a', "array"}, 3
            );
    args::ValueFlag<std::int32_t> array_height(
            parser, "array_height_", "calculating A array height",
            {'h', "height"}, 7
            );
    args::ValueFlag<std::int32_t> b_row(
            parser, "brow_", "calculating B array length",
            {'b', "brow"}, 7
            );
    args::ValueFlag<std::int32_t> b_column(
            parser, "bcol_", "calculating B array height",
            {'l', "bcol"}, 3
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

    CPU *cpu;
    int elements = row_;
    if (!trace_file.empty()) {
        cpu = new TraceBasedCPU(config_file, output_dir, trace_file, units_, row_, column_, array_, array_height_, brow_, bcol_);
    } else {
        if (stream_type == "stream" || stream_type == "s") {
            cpu = new StreamCPU(config_file, output_dir, units_, row_, column_, array_, array_height_, brow_, bcol_);
            std::vector<std::vector<int>> arrayA;
            for(int i=0;i <array_;i++){
                int x;
                std::vector<int> tempo;
                for(int j=0;j<array_height_;j++){
                    std::cin>>x;
                    tempo.push_back(x);
                }
                arrayA.push_back(tempo);
            }
            std::vector<std::vector<int>> arrayB;
            for(int i=0;i <brow_;i++){
                int x;
                std::vector<int> tempo;
                for(int j=0;j<bcol_;j++){
                    std::cin>>x;
                    tempo.push_back(x);
                }
                arrayB.push_back(tempo);
            }
            cpu->fixB(arrayB);
            cpu->fixA(arrayA);
            //std::cout<<array_<<" "<<array_height_<<" "<<brow_<<" "<<bcol_<<std::endl;
            /*for(int i=0;i<array_;i++){
                for(int j=0;j<array_height_;j++){
                    std::cout<<arrayA[i][j]<<" ";
                }
                std::cout<<std::endl;
            }
            for(int i=0;i<brow_;i++){
                for(int j=0;j<bcol_;j++){
                    std::cout<<arrayB[i][j]<<" ";
                }
                std::cout<<std::endl;
            }*/
            //std::cout<<std::endl;
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
