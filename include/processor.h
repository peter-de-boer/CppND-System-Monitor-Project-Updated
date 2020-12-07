#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <vector> 

class Processor {
 public:
  Processor();
  void GetCpuValues(std::vector<long int>&);
  float Utilization();  // TODO: See src/processor.cpp

  // TODO: Declare any necessary private members
 private:
  std::vector<long> prev_values;
};

#endif