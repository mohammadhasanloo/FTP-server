/*
* Computer Network Spring 2022
* Computer Assignment 1
*** FTP
*** Danial Saeedi, Mohammad Ghare Hasanloo
*/
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <string.h>
#include <filesystem> 
#include "server.h"

using namespace std;

int main(int argc, char* argv[])
{
    Server server = Server(argv[1]);
    server.run();
    return 0; 
}