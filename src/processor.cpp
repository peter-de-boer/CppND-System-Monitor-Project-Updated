#include "processor.h"
#include "linux_parser.h"
#include <string>
#include <vector>




Processor::Processor() {
  Processor::GetCpuValues(prev_values);
}

void Processor::GetCpuValues(std::vector<long> &values) {
  std::string name;
  std::string line = LinuxParser::CpuUtilization()[0];
  std::istringstream linestream(line);
  // first element is a name, the remainig elements are long values
  linestream >> name; 
  long ticks;
  while (linestream >> ticks) {values.push_back(ticks);}
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
float Processor::Utilization() { 
  
  std::vector<long> values;
  Processor::GetCpuValues(values);
    
  long Idle = values[3] + values[4];
  long PrevIdle = prev_values[3] + prev_values[4];
  
  long NonIdle = values[0] + values[1] + values[2] + values[5] + values[6] + values[7];
  long PrevNonIdle = prev_values[0] + prev_values[1] + prev_values[2] + prev_values[5] + prev_values[6] + prev_values[7];
  
  long Total = Idle + NonIdle;
  long PrevTotal = PrevIdle + PrevNonIdle;
  
  long totald = Total - PrevTotal;
  long idled = Idle - PrevIdle;

  float utilization = totald == 0 ? 0.0 : (float) (totald - idled)/ (float) totald;
  
  // save the current values for the following time step;
  prev_values = values;

  return utilization;
}