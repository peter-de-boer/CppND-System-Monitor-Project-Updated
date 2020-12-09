#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>


#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) {
  pid_ = pid;
  prev_total_time_ = LinuxParser::TotalTime(pid_);
  prev_uptime_ = LinuxParser::UpTimeF();
  float starttime = LinuxParser::StartTime(pid_);
  // if pid really just started set util to 0.0
  util_ = prev_uptime_ - starttime < 0.5 ? 0.0 : prev_total_time_ / (prev_uptime_-starttime);
}

// DONE: Return this process's ID
int Process::Pid() const { return pid_; }

void Process::UpdateCpuUtilization() {
  float total_time = LinuxParser::TotalTime(pid_);
  float uptime = LinuxParser::UpTimeF();
  // only update util if sufficiently time has passed;
  if (uptime - prev_uptime_ > 0.5) {
    util_ = (total_time - prev_total_time_) / (uptime - prev_uptime_);
    prev_total_time_ = total_time;
    prev_uptime_ = uptime;
  }
}

// DONE: Return this process's CPU utilization
float Process::CpuUtilization() const { 
  /*
  float total_time = LinuxParser::TotalTime(pid_);
  float uptime = LinuxParser::UpTimeF();
  float util = (total_time - prev_total_time_) / (uptime - prev_uptime_);
  prev_total_time_ = total_time;
  prev_uptime_ = uptime;
  */
  return util_;
  //return  LinuxParser::CpuUtilization(pid_) ; 
}

// DONE: Return the command that generated this process
string Process::Command() { return LinuxParser::Command(pid_); }

// DONE: Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid_); }

// DONE: Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(pid_);}

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a) const { 
  return (this->CpuUtilization() < a.CpuUtilization()); 
}

bool Process::operator>(Process const& a) const { 
  return (this->CpuUtilization() > a.CpuUtilization()); 
}