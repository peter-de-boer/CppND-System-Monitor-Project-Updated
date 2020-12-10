#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) {
  pid_ = pid;
  // initialize the utilization and the total_time and uptime for
  // future utilizazion updates
  prev_total_time_ = LinuxParser::TotalTime(pid_);
  prev_uptime_ = LinuxParser::UpTimeF();
  float starttime = LinuxParser::StartTime(pid_);
  // if pid really just started set util to 0.0
  util_ = prev_uptime_ - starttime < 0.5 ? 0.0 : prev_total_time_ /
                                                     (prev_uptime_ - starttime);
}

// Return this process's ID
int Process::Pid() const { return pid_; }

// Calculate the utilization in the most recent time window
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

// Return this process's CPU utilization
float Process::CpuUtilization() const {
  return util_;
}

// Return the command that generated this process
string Process::Command() { return LinuxParser::Command(pid_); }

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid_); }

// Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(pid_); }

// Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return (this->CpuUtilization() < a.CpuUtilization());
}

// Overload the "greater than" comparison operator for Process objects
bool Process::operator>(Process const& a) const {
  return (this->CpuUtilization() > a.CpuUtilization());
}
