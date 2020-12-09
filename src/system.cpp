#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>
#include <thread>


#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;


System::System() : processor_count_(std::thread::hardware_concurrency()) {
  cpu_ = Processor();
  processes_ = {};
}

int System::ProcessorCount() {
  return processor_count_;
}

// DONE: Return the system's CPU
// Why does this work even without having a System constructor with a cpu_ definition? cpu_ is just an empty object?;
Processor& System::Cpu() { return cpu_; }


// DONE: Return a container composed of the system's processes
vector<Process>& System::Processes() { 
  // loop over ids, if it is a new Process, create it; else update to trigger util
  // this is needed this way since we want to sort based on the utilization
  // and we do not want to trigger new util calculation while sorting.
  
  std::vector<int> read_pids = LinuxParser::Pids();  // read pids from linux system
  std::vector<int> current_pids = {};               // pids already stored in processes_
  for (Process process : processes_) { current_pids.push_back(process.Pid()); }
  std::sort(read_pids.begin(), read_pids.end());
  std::sort(current_pids.begin(), current_pids.end());

  std::vector<int> new_pids = {};                   // pids which do no exist in the processes_ vector yet
  std::set_difference(read_pids.begin(),read_pids.end(),
                        current_pids.begin(),current_pids.end(),
                        back_inserter(new_pids));
  std::vector<int> old_pids = {};                 // pids which do not exist on linux anymore
  std::set_difference(current_pids.begin(),current_pids.end(),
                        read_pids.begin(),read_pids.end(),
                        back_inserter(old_pids));

  // old_pids: remove them from processes_
  processes_.erase(
    std::remove_if(processes_.begin(), processes_.end(), 
                   [old_pids](const Process & p) { return std::find(old_pids.begin(), old_pids.end(), p.Pid()) != old_pids.end(); }),
                   processes_.end());

  // existing processes: update
  for (Process &p : processes_) { p.UpdateCpuUtilization();}
  
    // new_pids: add them to processes_
  for (int pid : new_pids) {
    processes_.push_back(Process(pid));
  }
  
  std::vector<int> result_pids = {};               // pids already stored in processes_
  for (Process process : processes_) { result_pids.push_back(process.Pid()); }
  std::sort(result_pids.begin(), result_pids.end());
  
  std::sort(processes_.begin(), processes_.end(), std::greater<Process>());
  return processes_; 
}

// DONE: Return the system's kernel identifier (string)
std::string System::Kernel() { return LinuxParser::Kernel(); }

// DONE: Return the system's memory utilization
float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

// DONE: Return the operating system name
std::string System::OperatingSystem() { return LinuxParser::OperatingSystem(); }

// DONE: Return the number of processes actively running on the system
int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

// DONE: Return the total number of processes on the system
int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

// DONE Return the number of seconds since the system started running
long int System::UpTime() { return LinuxParser::UpTime(); }