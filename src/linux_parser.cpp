#include <dirent.h>
#include <unistd.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// anonyomous namespace for utility functions that
// are only needed in this file
namespace {
// read key value pairs from file
// return the value for the first found key that matches req_key
string ReadValueFromFile(string file, string req_key) {
  string line;
  string key;
  string value;
  std::ifstream filestream(file);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == req_key) {
        return value;
      }
    }
  }
  return value;
}

string PidDir(int pid) {
  return LinuxParser::kProcDirectory + "/" + std::to_string(pid);
}
}

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// return utilization, defined as:
// used memory / total memory = (total memory - free memory) / total memory
float LinuxParser::MemoryUtilization() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  int allKeysFound{0};
  int memtotal, memfree;
  if (filestream.is_open()) {
    while (std::getline(filestream, line) && allKeysFound < 2) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "MemTotal:") {
        memtotal = std::stoi(value);
        allKeysFound += 1;
      }
      if (key == "MemFree:") {
        memfree = std::stoi(value);
        allKeysFound += 1;
      }
    }
  }
  return allKeysFound ? (float)(memtotal - memfree) / (float)memtotal : 0.0;
}

float LinuxParser::UpTime() {
  string uptime{"0.0"};
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return std::stof(uptime);
}


// Read and return CPU utilization
// The first element of the vector is the aggregated cpu utilization
// The remaining elements are utilizations for the individual processors
vector<string> LinuxParser::CpuUtilization() {
  string line{""};
  vector<string> cpu_lines;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line) && line.rfind("cpu", 0) == 0) {
      cpu_lines.push_back(line);
    }
  }
  return cpu_lines;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  return std::stoi(
      ReadValueFromFile(kProcDirectory + kStatFilename, "processes"));
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  return std::stoi(
      ReadValueFromFile(kProcDirectory + kStatFilename, "procs_running"));
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  std::ifstream filestream(PidDir(pid) + kCmdlineFilename);
  string line;
  if (filestream.is_open()) {
    std::getline(filestream, line);
  }
  return line;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  long ram_kb =
      std::stol(ReadValueFromFile(PidDir(pid) + kStatusFilename, "VmSize:"));
  long ram_mb = ram_kb / 1024;
  return to_string(ram_mb);
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  return ReadValueFromFile(PidDir(pid) + kStatusFilename, "Uid:");
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  // first find the uid
  string uid = LinuxParser::Uid(pid);
  // now find the user name corresponding to the uid
  string line;
  string user, x, userid;
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream{line};
      std::getline(linestream, user, ':');
      std::getline(linestream, x, ':');
      std::getline(linestream, userid, ':');
      if (userid == uid) {
        return user;
      }
    }
  }
  return "user_not_found";
}

// Read and return the uptime of a process
float LinuxParser::UpTime(int pid) {
  float seconds{0.0};
  std::ifstream stream(PidDir(pid) + kStatFilename);
  if (stream.is_open()) {
    string line;
    std::getline(stream, line);
    std::istringstream linestream(line);
    string v;
    for (int i = 0; i < 22; i++) {
      linestream >> v;
    }
    // uptime of this process is the uptime of system minus the starttime of
    // this process
    seconds = LinuxParser::UpTime() - std::stof(v) / (float) sysconf(_SC_CLK_TCK);
  }
  return seconds;
}

float LinuxParser::TotalTime(int pid) {
  std::ifstream stream(PidDir(pid) + kStatFilename);
  long total_time{0};
  if (stream.is_open()) {
    string line;
    std::getline(stream, line);
    std::istringstream linestream(line);
    string v, utime, stime, cutime, cstime;
    /*
    /proc/[PID]/stat
   #14 utime - CPU time spent in user code, measured in clock ticks
   #15 stime - CPU time spent in kernel code, measured in clock ticks
   #16 cutime - Waited-for children's CPU time spent in user code (in clock
   ticks)
   #17 cstime - Waited-for children's CPU time spent in kernel code (in clock
   ticks)
   */
    for (int i = 0; i < 13; i++) {
      linestream >> v;
    }
    linestream >> utime >> stime >> cutime >> cstime;
    total_time = std::stol(utime) + std::stol(stime) + std::stol(cutime) +
                 std::stol(cstime);
  }
  return (float)total_time / (float)sysconf(_SC_CLK_TCK);
}

float LinuxParser::StartTime(int pid) {
  float starttimef;
  std::ifstream stream(PidDir(pid) + kStatFilename);
  if (stream.is_open()) {
    string line;
    std::getline(stream, line);
    std::istringstream linestream(line);
    string v, starttime;
    /*
    From 
      https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
       /proc/[PID]/stat
       #22 starttime - Time when the process started, measured in clock ticks
   */

    for (int i = 0; i < 21; i++) {
      linestream >> v;
    }
    linestream >> starttime;
    starttimef = std::stol(starttime) / sysconf(_SC_CLK_TCK);
  }
  return starttimef;
}

float LinuxParser::CpuUtilization(int pid) {
  std::ifstream stream(PidDir(pid) + kStatFilename);
  float util{0.0};
  if (stream.is_open()) {
    float seconds{0};
    string line;
    std::getline(stream, line);
    std::istringstream linestream(line);
    string v, utime, stime, cutime, cstime, starttime;
    /*
    Calculation from 
      https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
    The parameter are read from /proc/[PID]/stat:
     #14 utime - CPU time spent in user code, measured in clock ticks
     #15 stime - CPU time spent in kernel code, measured in clock ticks
     #16 cutime - Waited-for children's CPU time spent in user code (in clock ticks)
     #17 cstime - Waited-for children's CPU time spent in kernel code (in clock ticks)
     #22 starttime - Time when the process started, measured in clock ticks
   */
    for (int i = 0; i < 13; i++) {
      linestream >> v;
    }
    linestream >> utime >> stime >> cutime >> cstime;
    for (int i = 17; i < 21; i++) {
      linestream >> v;
    }
    linestream >> starttime;
    float total_time = std::stof(utime) + std::stof(stime) + std::stof(cutime) +
                      std::stof(cstime);
    // uptime of this process is the uptime of system minus the starttime of
    // this process
    seconds =
        LinuxParser::UpTime() - std::stof(starttime) / (float) sysconf(_SC_CLK_TCK);
    util = (total_time / (float) sysconf(_SC_CLK_TCK)) / seconds;
  }
  return util;
}
