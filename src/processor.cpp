#include "processor.h"
#include "linux_parser.h"
#include <string>
#include <vector>




Processor::Processor() {
  Processor::GetCpuValues(prev_values);
}

void Processor::GetCpuValues(std::vector<std::vector<long>> &values) {
  std::string name;
  std::vector<std::string> lines = LinuxParser::CpuUtilization();
  
  for (std::string line: lines) {
    std::istringstream linestream(line);
    std::vector<long> values_one_line;
    // first element is a name, the remaining elements are long values
    linestream >> name; 
    long ticks;
    while (linestream >> ticks) {values_one_line.push_back(ticks);}
    values.push_back(values_one_line);
  }
  
}  

// TODO: Return the aggregate CPU utilization
/*
From https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux:

     user    nice   system  idle      iowait irq   softirq  steal  guest  guest_nice
cpu  74608   2520   24433   1117073   6176   4054  0        0      0      0

       0    1      2       3         4       5     6       7      8      9

PrevIdle = previdle + previowait
Idle = idle + iowait

PrevNonIdle = prevuser + prevnice + prevsystem + previrq + prevsoftirq + prevsteal
NonIdle = user + nice + system + irq + softirq + steal

PrevTotal = PrevIdle + PrevNonIdle
Total = Idle + NonIdle

# differentiate: actual value minus the previous one
totald = Total - PrevTotal
idled = Idle - PrevIdle

CPU_Percentage = (totald - idled)/totald
*/ 
std::vector<float> Processor::Utilization() { 
  
  std::vector<float> utilization;
  
  std::vector<std::vector<long>> values;
  Processor::GetCpuValues(values);
  
  for(unsigned int i= 0; i < values.size(); i++) {
    
    long Idle = values[i][3] + values[i][4];
    long PrevIdle = prev_values[i][3] + prev_values[i][4];
  
    long NonIdle = values[i][0] + values[i][1] + values[i][2] + values[i][5] + values[i][6] + values[i][7];
    long PrevNonIdle = prev_values[i][0] + prev_values[i][1] + prev_values[i][2] + prev_values[i][5] + prev_values[i][6] + prev_values[i][7];
  
    long Total = Idle + NonIdle;
    long PrevTotal = PrevIdle + PrevNonIdle;
  
    long totald = Total - PrevTotal;
    long idled = Idle - PrevIdle;

    float util = totald == 0 ? 0.0 : (float) (totald - idled)/ (float) totald;
    utilization.push_back(util);
  }
  // save the current values for the following time step;
  prev_values = values;

  return utilization;
}