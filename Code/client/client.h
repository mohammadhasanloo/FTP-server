#ifndef CLIENT
#define CLIENT

#include <string>
#include <vector>
#include <iostream>
#include <unistd.h> 
#include <sys/time.h>
#include <string.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 

#include "JsonParser.h"
#define SOCET_CREATION_ERROR "\n Can't create a socket \n"
#define SET_SOCKET_OPT_ERRORS "Setsockopt error" 
#define CON_FAILED "Connection Failed"
#define FAILED_TO_CONNECT "Cannot connect to the server!"
#define FAILED_CREATE_DOWNLOAD_DIR "Cannot create download directory!"
#define USR_CMD "user"
#define QUIT_CMD "quit"
#define DATA_IS_BEING_SENT "Data is being sent."
#define PASS_CMD "pass"
#define CMD_CHANNEL_PORT_JSON "commandChannelPort"
#define DIR_DOWNLOAD "./downloads"
#define NO_DATA "No data"
#define DChannel_PORT_KEY_JSON "dataChannelPort"
#define RETRIEVE_CMD "retr"
#define CORRECT_USR_CODE 331
#define CORRECT_PASS_CODE 230
#define QUIT_CODE 221


using namespace std;

class Client{
public:
    Client(string config_file_path);
    void run();
private:
    int port_cmd_channel;
    int port_dchannell;
    string username;
    bool is_user_online;
    int cmd_sock;
    int data_sock;
    void init_port(string config);
    int create_download_dir();
    void recieve_send_handler(string request, char buffer[], vector<string> request_params);
    void username_update(short req_type, char response[], vector<string> request_params);
    int connect_port(int port);
    vector<string> split(string str,char divider = ' ');
};

#endif