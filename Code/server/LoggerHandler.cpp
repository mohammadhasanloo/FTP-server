#include "LoggerHandler.h"
string get_date_time()
{
    auto now = chrono::system_clock::now();
    auto in_time_t = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

void write_log(string message)
{
    ofstream ofs;
    ofs.open (LOG_DIR, ofstream::out | ofstream::app);
    ofs << get_date_time() + " :: " + message + "\n";
    ofs.close();
}