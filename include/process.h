#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(int pid);
  int Pid() const;
  std::string User();
  std::string Command();
  float CpuUtilization() const;
  std::string Ram();
  float UpTime();
  bool operator<(Process const& a) const;
  bool operator>(Process const& a) const;
  void UpdateCpuUtilization();

 private:
  int pid_;
  float prev_total_time_, prev_uptime_;
  float util_;
};

#endif
