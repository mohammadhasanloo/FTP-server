#ifndef FTP_HEADER
#define FTP_HEADER

#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <sstream>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <bits/stdc++.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <filesystem>
#include "user.h"
#include "LoggerHandler.h"

using namespace std;

// Command String Constants
const string FILE_FLAG  = "-f";
const string DIRECTORY_FLAG =  "-d";
const string USR_CMD =  "user";
const string PASS_CMD =  "pass";
const string QUIT_CMD =  "quit";
const string LS_CMD =  "ls";
const string PWD_CMD =  "pwd";
const string MKD_CMD =  "mkd";
const string DELETE_COMMAND =  "dele";
const string CWD_CMD =  "cwd";
const string RENAME_CMD =  "rename";
const string RETRIEVE_CMD =  "retr";
const string HELP_CMD =  "help";
const string HELP_FILE_DIR = "help.txt";

// Error Messages
const string ACC_IS_NECES  = "332: Need account for login.\n";
const string SERVER_ERROR  = "500: Error\n";
const string LIST_TRANSFER_DONE  = "226: List transfer done.\n";
const string PASSWORD_IS_CORRECT  = "230: User logged in, proceed. Logged out if appropriate.\n";
const string QUIT_CODE =  "221: Successful Quit.\n";
const string CMD_ERROR  = "501: Syntax error in parameters or arguments.\n";
const string FILE_UNAVAILABLE  = "550: File unavailable.\n";
const string BAD_SEQUENCE_OF_COMMANDS =  "503: Bad sequence of commands.\n";
const string SUCCESSFUL_CHANGE  = "250: Successful change.\n";
const string CANT_OPEN_DATA_CONNECTION =  "425: Can\'t open data connection.\n";
const string SUCCESSFUL_DOWNLOAD  = "226: Successful download.\n";
const string INVALID_USERNAME_OR_PASSWORD =  "430: Invalid username or password\n";
const string USERNAME_IS_CORRECT =  "331: User name okay, need password.\n";

struct client_ftp
{
    bool has_data;
    User* user_info;
    string dir_curr;
    string data;
    bool does_user_have_permission;
};

class FTP
{
public:
    FTP(vector<User*> users_list, vector<string> admin_files_list);
    FTP() = default;
    
    string get_user_data(int client_sd);
    vector<User*> get_users();
    vector<string> get_admin_files();
    void remove_user(int client_sd);
    string cmd_handler(char command[], int client_sd);
    bool does_user_have_data(int client_sd);
    bool is_true(string cmd_input,string cmd_matched,int cmd_size,int cmd_size_matched);
private:
    vector<User*> users_vec;
    vector<string> admin_files;
    string default_dir;
    map<int, client_ftp*> connected_users;

    // Command Handlers
    string password_command(vector<string> args, int client_sd);
    string mkd_command(string path, int client_sd);
    string help_command(int client_sd);
    string quit_command(int client_sd);
    string dele_command(string type, string path, int client_sd);
    string cwd_command(string path, int client_sd);
    string rename_command(string old_name, string new_name, int client_sd);
    string download_command(string file_name, int client_sd);
    string ls_command(int client_sd);
    string pwd_command(int client_sd);
    string user_command(vector<string> args, int client_sd);

    // Helper Functions
    User* find_user(string username);
    bool file_exists(string file_name, string directory);
    bool does_file_belong_to_admin(string file_name);
    client_ftp* create_user(User* user);
    int delete_directory(string path);
    bool has_admin_dir_file(string path);
    long get_file_size(string file_name, string directory);
    vector<string> split(string str, char divider);
    string combine(vector<string> vector_of_string, char c);
};

#endif