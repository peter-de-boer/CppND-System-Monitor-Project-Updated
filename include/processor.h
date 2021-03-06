#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <vector>

class Processor {
 public:
  Processor();
  void GetCpuValues(std::vector<std::vector<long>>&);
  std::vector<float> Utilization();

 private:
  std::vector<std::vector<long>> prev_values;
};

#endif
