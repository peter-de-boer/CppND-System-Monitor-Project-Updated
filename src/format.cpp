#include <string>

#include "format.h"

using std::string;
using std::to_string;

// DONE: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) { 
  int hours = seconds / 3600;
  int minutes = (seconds % 3600) / 60;
  int sec = seconds % 60;
  
  string prev_hh{""};
  if (hours < 10) {prev_hh = "0";}
  string prev_mm{""};
  if (minutes < 10) {prev_mm = "0";}
  string prev_ss{""};
  if (sec < 10) {prev_ss = "0";}
  
  return prev_hh + to_string(hours) + ":" + prev_mm + std::to_string(minutes) + ":" + prev_ss + std::to_string(sec);
}