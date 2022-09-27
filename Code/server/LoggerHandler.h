#ifndef LOGGER_HEADER
#define LOGGER_HEADER
#include <iomanip>
#include <string>
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <fstream>
#define LOG_DIR "log.txt"
using namespace std;
void write_log(string message);
string get_date_time();
#endif