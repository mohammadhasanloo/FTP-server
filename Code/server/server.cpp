#include "server.h"

Server::Server(string config_file_path)
{
    JsonParser json_parser;
    string config = json_parser.read_json_file(config_file_path);
    system = FTP(read_users_config(config), read_files_config(config));
    init_port(config);
}

void Server::init_port(string config)
{
    JsonParser json_parser;
    port_cmd_channel = stoi(json_parser.find_val_json(config, CMD_CHANNEL_PORT_JSON));
    port_dchannell = stoi(json_parser.find_val_json(config, DChannel_PORT_KEY_JSON));
}

vector<string> Server::read_files_config(string config)
{
    vector<string> str;

    JsonParser json_parser;
    vector<string> file_names = json_parser.split_array(json_parser.find_val_json(config, FILES_JSON_KEY));

    for(int i = 0 ; i < file_names.size() ; i++)
        str.push_back(file_names[i]);

    return str;
}

vector<string> Server::split_pckt(string data, int pckt_size)
{
    vector<string> pckt;
    for(int i=0; i < data.size(); i += pckt_size){
        pckt.push_back(data.substr(i, pckt_size));
    }
    return pckt;
}

void Server::print_info()
{
    vector<User*> users = system.get_users();
    vector<string> files = system.get_admin_files();
    cout << COMMAND_PORT_MSG << port_cmd_channel << endl;
    cout << DATA_PORT_MSG << port_dchannell << endl;
    cout << USERS_MSG << endl;
    cout << ROW_MSG << endl;
    cout << NEW_LINE_MSG;
    for(int i = 0 ; i < users.size() ; i++)
        cout << users[i] -> get_username() << "     " << users[i] -> get_password() << "        " 
             << users[i] -> get_size() << "     " << users[i] -> get_is_admin() << endl; 
    cout << ADMIN_FILE_MSG << endl;
    cout << NEW_LINE_MSG;
    for(int i = 0 ; i < files.size() ; i++)
        cout << files[i] << endl; 
}

void Server::bind_sockets()
{
    struct sockaddr_in command_address, data_address;

    command_address.sin_family = AF_INET;
    command_address.sin_addr.s_addr = INADDR_ANY;
    command_address.sin_port = htons(port_cmd_channel);
    if (bind(cmd_sock, (struct sockaddr*)&command_address, sizeof(command_address)) < 0)
    {
        cout << BINDING_SOCKET_FAILED << endl;
        exit(0);
    }

    data_address.sin_family = AF_INET;
    data_address.sin_addr.s_addr = INADDR_ANY;
    data_address.sin_port = htons(port_dchannell);
    if (bind(data_sock, (struct sockaddr*)&data_address, sizeof(data_address)) < 0)
    {
        cout << BINDING_SOCKET_FAILED << endl;
        exit(0);
    }
}

void Server::sockets_listen()
{
    if (listen(cmd_sock, 3) < 0)  
    {  
        cout << LISTENING_COMMAND_SOCKET_FAILED << endl;  
        exit(0); 
    }
    if (listen(data_sock, 3) < 0)  
    {  
        cout << LISTENING_DATA_SOCKET_FAILED << endl;
        exit(0); 
    }  
}

void Server::create_new_sockets()
{
    int opt = 1;
    cmd_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (cmd_sock == 0)
    {
        cout << CMD_CREATION_FAILED << endl;
        exit(0);
    }

    data_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (data_sock == 0)
    {
        cout << DATA_CREATION_FAILED << endl;
        exit(0);
    }

    if (setsockopt(cmd_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )  
    {  
        cout << SET_SOCKET_OPT_FAILED << endl;  
        exit(0); 
    } 

    if (setsockopt(data_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )  
    {  
        cout << SET_SOCKET_OPT_FAILED << endl;  
        exit(0); 
    } 

    bind_sockets();

    sockets_listen();
    
}

void Server::set_sockets(int& max_sd, fd_set& read_file_descriptor)
{
    FD_ZERO(&read_file_descriptor);  
      
    FD_SET(cmd_sock, &read_file_descriptor);
    FD_SET(data_sock, &read_file_descriptor);

    max_sd = cmd_sock > data_sock ? cmd_sock : data_sock;  
            
    for ( int i = 0 ; i < clients.size() ; i++)  
    {  
        int sd = clients[i];  

        if (sd > 0)  
            FD_SET( sd , &read_file_descriptor);  
                
        if (sd > max_sd)  
            max_sd = sd;  
    }  
}

void Server::accept_connections()
{
    struct sockaddr_in address_command, address_data;
    int addrlen_data = sizeof(address_data);
    int addrlen_command = sizeof(address_command);

    int new_socket_data = accept(data_sock, (struct sockaddr *)&address_data, (socklen_t*)&addrlen_data);
    if (new_socket_data < 0)  
    {  
        cout << ACCEPT_ERROR << endl;  
        exit(0); 
    }
    int new_sock_cmd = accept(cmd_sock, (struct sockaddr *)&address_command, (socklen_t*)&addrlen_command);
    if(new_sock_cmd < 0)  
    {  
        cout << ACCEPT_ERROR << endl;  
        exit(0); 
    }

    cout << NEW_CONNECTION_CMD_SOCKET << new_sock_cmd
            << IP_IS << inet_ntoa(address_command.sin_addr) <<  PORT_IS
            << ntohs(address_command.sin_port) << endl; 

    cout << NEW_CON_DATA_SOCKET << new_socket_data
            << IP_IS << inet_ntoa(address_data.sin_addr) <<  PORT_IS
            << ntohs(address_data.sin_port) << endl; 

    if (send(new_sock_cmd, WELCOME_MSG, strlen(WELCOME_MSG), 0) != strlen(WELCOME_MSG) )  
        cout << SEND_FAILED << endl; 

    bool is_client_set = false;
    for (int i = 0; i < clients.size(); i++)  
    {  
        if ( !clients[i] )  
        {
            is_client_set = true;
            clients[i] = new_sock_cmd;
            break;
        }  
    }  
    if (is_client_set == 0)
        clients.push_back(new_sock_cmd);

    cmd_data_from_clients.insert(pair<int, int>(new_sock_cmd, new_socket_data));
}

void Server::clients_req_handler(fd_set& read_file_descriptor)
{
    char buffer[2048+1];
    int buf;
    struct sockaddr_in address;
    int length_of_address = sizeof(address);

    for (int i = 0; i < clients.size(); i++)  
    {  
        int sd = clients[i];  
            
        if (FD_ISSET(sd , &read_file_descriptor))  
        {  
            if ((buf = read(sd , buffer, 2048)) == 0)  
            {  
                getpeername(sd , (struct sockaddr*)&address , \
                    (socklen_t*)&length_of_address);  

                cout << GUEST_DISCONNECTED << inet_ntoa(address.sin_addr) << PORT_IS << ntohs(address.sin_port) << endl;
                
                system.remove_user(sd);
                close(sd);
                close(cmd_data_from_clients[sd]);
                clients[i] = 0;  
            }  
            else 
                recieve_send_handler(buf, buffer, sd);
        }  
    }
}

void Server::recieve_send_handler(int buf, char buffer[], int sd)
{
    buffer[buf] = '\0';
    string response = system.cmd_handler(buffer, sd);
    if (fork() == 0)
    {
        if (system.does_user_have_data(sd))
        {
            vector<string> pckt = split_pckt(system.get_user_data(sd), 2048);
            send(sd ,DATA_IS_BEING_SENT ,strlen(DATA_IS_BEING_SENT) , 0);
            usleep(SLEEP_AMNT);
            string pckt_size = to_string(pckt.size());
            send(sd ,pckt_size.c_str() ,strlen(pckt_size.c_str()) , 0);
            usleep(SLEEP_AMNT);
            for(int i = 0; i < pckt.size(); i++)
            {
                send(cmd_data_from_clients[sd], pckt[i].c_str(), strlen(pckt[i].c_str()),0);
                usleep(SLEEP_AMNT);
            }
        }
        else
        {
            send(sd ,THERES_NO_DATA_AVAILABLE ,strlen(THERES_NO_DATA_AVAILABLE) , 0);
            usleep(500);
        }
        send(sd ,response.c_str() ,strlen(response.c_str()) , 0);
        usleep(500);
        exit(0);
    }
}

void Server::run()
{
    create_new_sockets();
    fd_set read_file_descriptor;
    int max_sd, activity;
    while(true)  
    {
        set_sockets(max_sd, read_file_descriptor);
        activity = select( max_sd + 1 , &read_file_descriptor , NULL , NULL , NULL);  
        if ((activity < 0) && (errno!=EINTR))  
            cout << SELECT_ERROR << endl;     
        if (FD_ISSET(cmd_sock, &read_file_descriptor) && FD_ISSET(data_sock, &read_file_descriptor))
            accept_connections();
        clients_req_handler(read_file_descriptor);
    }
}

vector<User*> Server::read_users_config(string config)
{
    vector<User*> result;
    JsonParser json_parser;
    vector<string> obj_usrs = json_parser.split_array(json_parser.find_val_json(config, USRS_JSON_KEY));
    for(int i = 0 ; i < obj_usrs.size() ; i++)
    {
        string username = json_parser.find_val_json(obj_usrs[i], USR_JSON_KEY);
        string password = json_parser.find_val_json(obj_usrs[i], PWD_JSON_KEY);
        string admin = json_parser.find_val_json(obj_usrs[i], ADMIN_JSON_KEY);
        string size = json_parser.find_val_json(obj_usrs[i], SZE_JSON_KEY);

        bool is_admin = admin == "true";
        User* new_user = new User(username, password, (long)(stol(size)*1024), is_admin);
        result.push_back(new_user);
    }
    return result;
}