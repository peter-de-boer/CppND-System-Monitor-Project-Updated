#include "processor.h"
#include "linux_parser.h"
#include <string>
#include <vector>

// TODO: Return the aggregate CPU utilization
/*
From https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux:

     user    nice   system  idle      iowait irq   softirq  steal  guest  guest_nice
cpu  74608   2520   24433   1117073   6176   4054  0        0      0      0

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
  
  std::string name;
  long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
  
  std::string line = LinuxParser::CpuUtilization()[0];
  std::istringstream linestream(line);
  linestream >> name >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

  long Idle = idle + iowait;
  long NonIdle = user + nice + system + irq + softirq + steal;
  long Total = Idle + NonIdle;
  float utilization = (float) NonIdle/ (float) Total;

  return utilization;
}