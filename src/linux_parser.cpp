#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <unistd.h>

#include "linux_parser.h"


using std::stof;
using std::string;
using std::to_string;
using std::vector;

// anonyomous namespace for some utility functions that only are needed in this file
namespace {
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

// DONE: An example of how to read data from the filesystem
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

// DONE: An example of how to read data from the filesystem
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

// BONUS: Update this to use std::filesystem
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

// DONE: Read and return the system memory utilization
// utilization is defined as: 
// used memory / total memory = (total memory - free memory) / total memory
float LinuxParser::MemoryUtilization() { 
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  int allKeysFound{0};
  int memtotal, memfree;
  if (filestream.is_open()) {
    while (std::getline(filestream, line) && allKeysFound<2) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "MemTotal:") {
        memtotal = std::stoi(value);
        allKeysFound +=1;
      }
      if (key == "MemFree:") {
        memfree = std::stoi(value);
        allKeysFound +=1;
      }
    }
  }
  return allKeysFound ? (float) (memtotal-memfree) / (float) memtotal : 0.0;
}

float LinuxParser::UpTimeF() { 
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

// DONE: Read and return the system uptime
long LinuxParser::UpTime() { 
  return std::lround(LinuxParser::UpTimeF());
}

// DONE: Read and return CPU utilization
// ? element 0 is aggregated cpu utilization
// ? remaining elements are utilizations for the individual processors
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


// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  return std::stoi(ReadValueFromFile(kProcDirectory + kStatFilename, "processes"));
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  return std::stoi(ReadValueFromFile(kProcDirectory + kStatFilename, "procs_running"));
}

// DONE: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) { 
  std::ifstream filestream(PidDir(pid) + kCmdlineFilename);
  string line;
  if (filestream.is_open()) {
    std::getline(filestream, line);
  }
  return line;
}

// DONE: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) { 
  long ram_kb = std::stol(ReadValueFromFile(PidDir(pid) + kStatusFilename, "VmSize:"));
  long ram_mb = ram_kb / 1024;
  return to_string(ram_mb); 
}

// DONE: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) { 
  return ReadValueFromFile(PidDir(pid) + kStatusFilename, "Uid:");
}

// DONE: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) { 
  string uid = LinuxParser::Uid(pid);
  string line;
  string user, x, userid;
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream { line };
      std::getline(linestream, user, ':');
      std::getline(linestream, x, ':');
      std::getline(linestream, userid, ':');
      //linestream >> user >> x >> userid;
      if (userid == uid) {return user;}
    }
  }
  return "user_not_found";
}

// DONE: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) { 
  long seconds{0};
  std::ifstream stream(PidDir(pid) + kStatFilename);
  if (stream.is_open()) {
    string line;
    std::getline(stream, line);
    std::istringstream linestream(line);
    string v;
    for (int i = 0; i < 22; i++) {
      linestream >> v;
    }
    // uptime of this process is the uptime of system minus the starttime of this process
    seconds = LinuxParser::UpTime() - std::stol(v) / sysconf(_SC_CLK_TCK) ;
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
#16 cutime - Waited-for children's CPU time spent in user code (in clock ticks)
#17 cstime - Waited-for children's CPU time spent in kernel code (in clock ticks)
#22 starttime - Time when the process started, measured in clock ticks
*/
    for (int i = 0; i < 13; i++) {
      linestream >> v;
    }
    linestream >> utime >> stime >> cutime >> cstime;
    total_time = std::stol(utime) + std::stol(stime) + std::stol(cutime) + std::stol(cstime);
  }
  return (float) total_time / (float) sysconf(_SC_CLK_TCK);
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
 /proc/[PID]/stat
#22 starttime - Time when the process started, measured in clock ticks
*/

    for (int i = 0 ; i < 21 ; i++) {
      linestream >> v;
    }
    linestream >> starttime;
    starttimef= std::stol(starttime) / sysconf(_SC_CLK_TCK) ;  
  }
  return starttimef;
}

float LinuxParser::CpuUtilization(int pid) {  
  std::ifstream stream(PidDir(pid) + kStatFilename);
  float util{0.0};
  if (stream.is_open()) {
    long seconds{0};
    string line;
    std::getline(stream, line);
    std::istringstream linestream(line);
    string v, utime, stime, cutime, cstime, starttime;
 /*
 /proc/[PID]/stat
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
    for (int i = 17 ; i < 21 ; i++) {
      linestream >> v;
    }
    linestream >> starttime;
    long total_time = std::stol(utime) + std::stol(stime) + std::stol(cutime) + std::stol(cstime);
    // uptime of this process is the uptime of system minus the starttime of this process
    seconds = LinuxParser::UpTime() - std::stol(starttime) / sysconf(_SC_CLK_TCK) ;
    util = (float) (total_time / sysconf(_SC_CLK_TCK)) / (float) seconds;

  }
  return util;
}
