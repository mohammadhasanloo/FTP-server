#include "client.h"

Client::Client(string config_file_path){
    username = "";
    JsonParser json_parser;
    string config = json_parser.read_json_file(config_file_path);
    is_user_online = false;
    init_port(config);
}

void Client::init_port(string config)
{
    JsonParser json_parser;
    port_dchannell = stoi(json_parser.find_val_json(config, DChannel_PORT_KEY_JSON));
    port_cmd_channel = stoi(json_parser.find_val_json(config, CMD_CHANNEL_PORT_JSON));
}

// This function splits the client's input
vector<string> Client::split(string str, char divider)
{
    vector<string> out;
    stringstream ss(str);
    string word;

    while(getline(ss, word, divider))
    {
        if(word != "")
            out.push_back(word);
    }
    return out;
}

int Client::connect_port(int port){
    int opt = 1;
    int fd = 1;
    struct sockaddr_in server_addr;
    
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << SOCET_CREATION_ERROR;
        exit(0);
    }

    // Config
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )  
    {
        cout << SET_SOCKET_OPT_ERRORS << endl;  
        exit(0);  
    }
 
    if (connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        cout << CON_FAILED << endl;
        exit(0);
    }
    return fd;
}

void Client::recieve_send_handler(string req, char res[], vector<string> request_params)
{
    string data;
    int buff;

    send(cmd_sock , req.c_str() , strlen(req.c_str()) , 0 );

    // Reading input from cmd
    memset(res, 0, strlen(res));
    buff = read(cmd_sock, res, 2048);

    if (strcmp(res, DATA_IS_BEING_SENT) == 0)
    {
        data = "";
        memset(res, 0, strlen(res));
        buff = read(cmd_sock, res, 2048);
        int pckt_size = atoi(res);
        for(int i = 0; i < pckt_size; i++)
        {
            memset(res, 0, strlen(res));
            buff = read(data_sock, res, 2048);
            string tmp = res;
            data += tmp;
            if (request_params.size() > 0 && request_params[0] == RETRIEVE_CMD)
            {
                cout.flush();
            }
        }
        if (request_params.size() > 0 && request_params[0] == RETRIEVE_CMD)
        {
            cout << endl;
            ofstream f(string(DIR_DOWNLOAD) + "/" + request_params[1]);
            f << data;
            f.close();
        }
        else
            cout << data << endl;
    }
    memset(res, 0, strlen(res));
    buff = read(cmd_sock , res, 2048);
    cout << res << endl;
}


void Client::username_update(short req_type, char res[], vector<string> request_params)
{
    vector<string> res_splitted = split(res, ':');

    switch(req_type){
        case 1:
            if(stoi(res_splitted[0]) == CORRECT_USR_CODE)
                username = request_params[1];
            else
                username = username;
            break;
        case 2:
            if(stoi(res_splitted[0]) == CORRECT_PASS_CODE){
                is_user_online = true;
            }
            else
                is_user_online = false;
            break;
        case 3:
            if (stoi(res_splitted[0]) == QUIT_CODE)
            {
                is_user_online = false;
                username = "";
            }

            break;
    }
}

void Client::run()
{
    if (create_download_dir() != 0)
    {
        cout << FAILED_CREATE_DOWNLOAD_DIR << endl;
        exit(0);
    }

    data_sock = connect_port(port_dchannell);
    cmd_sock = connect_port(port_cmd_channel);
    if (data_sock <= 0 || cmd_sock <= 0)
    {
        cout << FAILED_TO_CONNECT << endl;
        exit(0);
    }

    char res[2048] = {0};
    string req;
    int buff = read(cmd_sock , res, 2048);
    cout << res << endl; 
    while(1)
    {
        getline(cin, req);
        vector<string> request_params = split(req);
        recieve_send_handler(req, res, request_params);
        if (request_params.size() > 0 && request_params[0] == USR_CMD || request_params.size() > 0 && request_params[0] == PASS_CMD || request_params.size() > 0 && request_params[0] == QUIT_CMD)
        {
            short type;
            if(request_params.size() > 0 && request_params[0] == USR_CMD ? 1 : request_params.size() > 0 && request_params[0] == PASS_CMD){
                type = 2;
            }else{
                type = 3;
            }
            username_update(type, res, request_params);
        }
    }
}

int Client::create_download_dir()
{
    struct stat st = {0};

    if (stat(DIR_DOWNLOAD, &st) == -1)
        return mkdir(DIR_DOWNLOAD, 0700);

    return 0;
}

