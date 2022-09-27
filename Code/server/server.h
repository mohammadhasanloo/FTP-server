#ifndef SERVER
#define SERVER

#include <arpa/inet.h> 
#include <unistd.h> 
#include <sys/time.h>
#include <string.h>
#include <string>
#include <vector>
#include <iostream>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <map>
#include <time.h>
#include "user.h"
#include "LoggerHandler.h"
#include "JsonParser.h"
#include "FTP.h"

// Constants
#define DATA_PORT_MSG "Data Port: "
#define USERS_MSG "Users:"
#define ROW_MSG "Username   Password    Size        IsAdmin"
#define USERS_MSG "Users:"
#define NEW_LINE_MSG "----------------------------------------------------------------------\n"
#define ADMIN_FILE_MSG "Admin Files:"
#define BINDING_SOCKET_FAILED "Binding command socket failed!"
#define LISTENING_COMMAND_SOCKET_FAILED "Listening command socket failed!"
#define LISTENING_COMMAND_SOCKET_FAILED "Listening command socket failed!"
#define LISTENING_DATA_SOCKET_FAILED "Listening data socket failed!"
#define CMD_CREATION_FAILED "Command socket creation failed!"
#define DATA_CREATION_FAILED "Data socket creation failed!"
#define SET_SOCKET_OPT_FAILED "Setsockopt command socket error!"
#define ACCEPT_ERROR "Accept error for data socket!"
#define NEW_CONNECTION_CMD_SOCKET "New connection for command socket, socket fd is "
#define IP_IS ", ip is : "
#define PORT_IS ", port : "
#define NEW_CON_DATA_SOCKET "New connection for data socket, socket fd is "
#define WELCOME_MSG "Welcome! \n"
#define SEND_FAILED "send failed"
#define GUEST_DISCONNECTED "Guest disconnected ip "
#define SELECT_ERROR "Select error!"

const int PACKET_SIZE = 2048;
const int SLEEP_AMNT = 500;
#define ADMIN_JSON_KEY "admin"
#define DATA_IS_BEING_SENT "Data is being sent."
#define CMD_CHANNEL_PORT_JSON  "commandChannelPort"
#define DChannel_PORT_KEY_JSON  "dataChannelPort"
#define PWD_JSON_KEY  "password"
#define THERES_NO_DATA_AVAILABLE  "There's no data available"
#define FILES_JSON_KEY  "files"
#define USRS_JSON_KEY  "users"
#define USR_JSON_KEY  "user"
#define SZE_JSON_KEY  "size"

#define COMMAND_PORT_MSG "Command Port: "
#define DATA_PORT_MSG "Data Port: "

using namespace std;

class Server
{ 
public:
    // Server's Constructor
    Server(string config_file_path);
    vector<string> split_pckt(string data, int packet_size);
    void print_info();
    void run();
    
private:
    FTP system;
    vector<User*> read_users_config(string config);
    vector<string> read_files_config(string config);
    map<int, int> cmd_data_from_clients;
    vector<int> clients;
    int cmd_sock;
    int data_sock;
    void init_port(string config);
    void bind_sockets();
    void sockets_listen();
    void create_new_sockets();
    void set_sockets(int& max_socket_descriptor, fd_set& readfds);
    void accept_connections();
    void clients_req_handler(fd_set& readfds);
    void recieve_send_handler(int valread, char buffer[], int sd);
    int port_cmd_channel;
    int port_dchannell;
};

#endif