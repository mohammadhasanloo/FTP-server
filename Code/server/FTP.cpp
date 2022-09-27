#include "FTP.h"

FTP::FTP(vector<User*> users_list, vector<string> admin_files_list)
{
    users_vec = users_list;
    admin_files = admin_files_list;
    char cwd[1024];
    getcwd(cwd, 1024);
    string tmp(cwd);
    default_dir = tmp;
}

// This function is taken from a stackoverflow example
int FTP::delete_directory(string path)
{
    DIR *dir;
	struct dirent *ent;

	dir = opendir (path.c_str());

	if (dir != NULL)
    {
        while ((ent = readdir (dir)) != NULL)
        {
            if (strcmp(ent->d_name, "..") == 0 || strcmp(ent->d_name, ".") == 0)
                continue;

            int result;

            string name(ent->d_name);
            string addr = path + "/" + name;

            if (ent->d_type != DT_DIR)
                result = remove(addr.c_str());
        	else  
                result = delete_directory(addr);

            if (result != 0)
                return result;
        }
        closedir (dir);
	}

    remove(path.c_str());

    return 0;
}

bool FTP::file_exists(string file_name, string directory)
{
    DIR *dir;
	struct dirent *ent;

	dir = opendir (directory.c_str());

	if (dir != NULL)
    {
        while ((ent = readdir (dir)) != NULL)
        {
            if (ent->d_type != DT_REG)
                continue; 
        	if (strcmp(ent->d_name, file_name.c_str()) == 0)  
            {   
                closedir(dir);
                return true;
            }
        }
        closedir (dir);
	}

    return false;
}

bool FTP::has_admin_dir_file(string path)
{
    DIR *dir;
	struct dirent *ent;

	dir = opendir (path.c_str());

	if (dir != NULL)
    {
        while ((ent = readdir (dir)) != NULL)
        {
            if (strcmp(ent -> d_name, ".") == 0 || strcmp(ent -> d_name, "..") == 0)
                continue;

            if (ent->d_type != DT_DIR && does_file_belong_to_admin(ent->d_name))
            {
                closedir (dir);
                return true;
            }
        	else if(has_admin_dir_file(path + "/" + ent->d_name))
            {
                closedir (dir);
                return true;
            }   
        }
        closedir (dir);
	}

    return false;
}

bool FTP::does_file_belong_to_admin(string file_name)
{
    for (int i = 0 ; i < admin_files.size() ; i++)
        if (file_name == admin_files[i])
            return true;
    return false;
}

vector<string> FTP::split(string str, char divider = ' ')
{
    stringstream ss(str);
    string word;
    vector< string> res;
    while(getline(ss, word, divider))
    {
        if(word != "")
            res.push_back(word);
    }
    return res;
}

string FTP::combine(vector<string> vector_of_string, char c)
{
    string res = "";
    for(int i = 0; i < vector_of_string.size(); i++)
    {
        res += vector_of_string[i];
        if (i < vector_of_string.size()-1)
            res += c;
    }
    return res;
}

User* FTP::find_user(string username)
{
    for (int i = 0 ; i < users_vec.size() ; i++)
    {
        if (users_vec[i]->get_username() == username)
            return users_vec[i];
    }
    return NULL;
}

long FTP::get_file_size(string file_name, string directory)
{
    ifstream in_file(directory+"/"+file_name, ios::binary);
    in_file.seekg(0, ios::end);
    long file_size = in_file.tellg();
    in_file.close();
    return file_size;
}


bool FTP::does_user_have_data(int client_sd)
{
    map<int,client_ftp*>::iterator it;
    it = connected_users.find(client_sd);
    if (it != connected_users.end())
        return it->second->has_data;

    return false;
}

string FTP::get_user_data(int client_sd)
{
    map<int,client_ftp*>::iterator it;

    it = connected_users.find(client_sd);

    if (it != connected_users.end())
        return it->second->data;

    return "";
}

client_ftp* FTP::create_user(User* user)
{
    client_ftp* new_user = new client_ftp();
    new_user->has_data = false;
    new_user->dir_curr = default_dir;
    new_user->does_user_have_permission = false;
    new_user->data = "";
    new_user->user_info = user;
    return new_user;
}

string FTP::password_command(vector<string> args, int client_sd)
{
    map<int,client_ftp*>::iterator it;

    it = connected_users.find(client_sd);

    if (it == connected_users.end() || it->second->does_user_have_permission == true)
        return BAD_SEQUENCE_OF_COMMANDS;

    User* selected_user = it->second->user_info;
    if (selected_user->get_password() != args[1])
        return INVALID_USERNAME_OR_PASSWORD;

    write_log("This user has logged in: " + it->second->user_info->get_username());
    it->second->does_user_have_permission = true;
    return PASSWORD_IS_CORRECT;
}

string FTP::quit_command(int client_sd)
{
    map<int,client_ftp*>::iterator it;

    it = connected_users.find(client_sd);

    if (it == connected_users.end())
        return ACC_IS_NECES;

    remove_user(client_sd);
    return QUIT_CODE;
}

string FTP::user_command(vector<string> args, int client_sd)
{
    map<int,client_ftp*>::iterator it;

    it = connected_users.find(client_sd);

    if (it != connected_users.end() && it->second->does_user_have_permission)
        return BAD_SEQUENCE_OF_COMMANDS;

    // Check if user exists or not
    User* selected_user = find_user(args[1]);
    if (selected_user == NULL)
        return INVALID_USERNAME_OR_PASSWORD;

    if (it != connected_users.end())
        remove_user(client_sd);

    write_log("This user logged in: " + args[1]);
    connected_users.insert(pair<int, client_ftp*>(client_sd, create_user(selected_user)));
    return USERNAME_IS_CORRECT;
}

string FTP::pwd_command(int client_sd)
{
    map<int,client_ftp*>::iterator it;

    it = connected_users.find(client_sd);

    if (it == connected_users.end() || !it->second->does_user_have_permission)
        return ACC_IS_NECES;
    
    return "257: " + it->second->dir_curr + "\n";
}

string FTP::ls_command(int client_sd)
{
    DIR *dir;
	struct dirent *ent;
    map<int,client_ftp*>::iterator it;

    it = connected_users.find(client_sd);

    if (it == connected_users.end() || !it->second->does_user_have_permission)
        return ACC_IS_NECES;
    
    it->second->data = "";

    dir = opendir (it->second->dir_curr.c_str());

	if (dir != NULL)
    {
        while ((ent = readdir (dir)) != NULL)
        {
            if (strcmp(ent -> d_name, ".") != 0 && strcmp(ent -> d_name, "..") != 0)
                it->second->data += string(ent->d_name) + "\n";
        }
        closedir (dir);
        it->second->has_data = true;
        return LIST_TRANSFER_DONE;
	}
    else
        return SERVER_ERROR;
}

string FTP::dele_command(string type, string path, int client_sd)
{
    string chosen_dir;
    map<int,client_ftp*>::iterator it;
    it = connected_users.find(client_sd);

    if (!it->second->does_user_have_permission || it == connected_users.end())
        return ACC_IS_NECES;

    if (type == DIRECTORY_FLAG)
    {
        vector<string> file_folder_path = split(path, '/');
        vector<string> tmp;

        if(path[0] == '/')
            chosen_dir = "";
        else
            chosen_dir = it->second->dir_curr;

        for(int i = 0; i < file_folder_path.size(); i++)
        {
            if (file_folder_path[i] == ".") continue;
            else if (file_folder_path[i] == "..")
            {
                tmp = split(chosen_dir, '/');
                if(tmp.size() > 0)
                    tmp.pop_back();
                chosen_dir = "/" + combine(tmp, '/');
            }
            else
            {
                if(chosen_dir.back() == '/')
                    chosen_dir = "";
                else
                    chosen_dir = "/";
                chosen_dir += file_folder_path[i];
                DIR *dir = opendir(chosen_dir.c_str());
                if (dir == NULL)
                    return SERVER_ERROR;
                closedir(dir);
            }
        }

        if (has_admin_dir_file(chosen_dir) && !it->second->user_info->get_is_admin())
            return FILE_UNAVAILABLE;

        if ((delete_directory(chosen_dir)) == 0)
        {
            write_log("This user: " + it->second->user_info->get_username() + "has deleted this file: " + chosen_dir);
            return "250: " + path + " deleted.\n";
        }
    }
    else if (type == FILE_FLAG)
    {
        if (does_file_belong_to_admin(path) && !it->second->user_info->get_is_admin())
            return FILE_UNAVAILABLE;

        if (!file_exists(path, it->second->dir_curr))
            return SERVER_ERROR;
        
        string file_path = it->second->dir_curr + "/" + path ;
        if ((remove(file_path.c_str()) == 0))
        {
            write_log("This user: " + it->second->user_info->get_username() + " has deleted this file: " + file_path);
            return "250: " + path + " deleted.\n";
        }
    }

    return SERVER_ERROR;
}

string FTP::mkd_command(string path, int client_sd)
{
    map<int,client_ftp*>::iterator it;

    it = connected_users.find(client_sd);

    if (it == connected_users.end() || !it->second->does_user_have_permission)
        return ACC_IS_NECES;

    vector<string> file_folder_path = split(path, '/');
    vector<string> tmp;

    string new_dir;
    if(path[0] == '/')
        new_dir = "";
    else
        new_dir = it->second->dir_curr;

    for(int i = 0; i < file_folder_path.size(); i++)
    {
        if (file_folder_path[i] == ".") continue;
        else if (file_folder_path[i] == "..")
        {
            tmp = split(new_dir, '/');
            if(tmp.size() > 0)
                tmp.pop_back();
            new_dir = "/" + combine(tmp, '/');
        }
        else
        {
            if(new_dir.back() == '/')
                new_dir += "";
            else
                new_dir = "/";
            new_dir += file_folder_path[i];
        }
    }

    DIR *dir = opendir(new_dir.c_str());
    if (dir != NULL)
        return SERVER_ERROR;

    if ((mkdir(path.c_str(),0777) == 0))
    {
        write_log("This user: " + it->second->user_info->get_username() + "has created " + new_dir);
        return "257: " + path + " created.\n";
    }

    return SERVER_ERROR;
}

string FTP::cwd_command(string path, int client_sd)
{
    map<int,client_ftp*>::iterator it;
    it = connected_users.find(client_sd);

    if (it == connected_users.end() || !it->second->does_user_have_permission)
        return ACC_IS_NECES;
    if (path == "")
    {
        it->second->dir_curr = default_dir;
        return SUCCESSFUL_CHANGE;
    }

    vector<string> file_folder_path = split(path, '/');
    vector<string> tmp;
    string new_dir;
    if (file_folder_path.size() == 0)
    {
        it->second->dir_curr = "/";
        return SUCCESSFUL_CHANGE;
    }

    if(path[0] == '/')
        new_dir = "";
    else
        new_dir = it->second->dir_curr;
    
    for(int i = 0; i < file_folder_path.size(); i++)
    {
        if (file_folder_path[i] == ".") continue;
        else if (file_folder_path[i] == ".."){
            tmp = split(new_dir, '/');
            if(tmp.size() > 0)
            {
                tmp.pop_back();
            }
            new_dir = "/" + combine(tmp, '/');
        }
        else{
            if(new_dir.back() == '/')
                new_dir = "";
            else
                new_dir = "/";
            new_dir += file_folder_path[i];
            DIR *dir = opendir(new_dir.c_str());
            if (dir == NULL)
                return SERVER_ERROR;
            closedir(dir);
        }
    }

    write_log("This user: " + it->second->user_info->get_username() + "has change folder name from " +
             it->second->dir_curr + " to " + new_dir );

    it->second->dir_curr = new_dir;
    return SUCCESSFUL_CHANGE;
}

string FTP::download_command(string file_name, int client_sd)
{
    map<int,client_ftp*>::iterator it;
    it = connected_users.find(client_sd);

    if (it == connected_users.end() || !it->second->does_user_have_permission)
        return ACC_IS_NECES;

    if (does_file_belong_to_admin(file_name) && !it->second->user_info->get_is_admin())
        return FILE_UNAVAILABLE;

    if (file_exists(file_name, it->second->dir_curr))
    {
        int size = get_file_size(file_name, it->second->dir_curr);
        if (size > it->second->user_info->get_size())
            return CANT_OPEN_DATA_CONNECTION;

        it->second->user_info->reduce_user_storage_size(size);
        ifstream ifs(it->second->dir_curr + "/" + file_name);
        string content((istreambuf_iterator<char>(ifs)),
                        (istreambuf_iterator<char>()));
        ifs.close();

        it->second->data = content;
        it->second->has_data = true;

        write_log("This user: " + it->second->user_info->get_username() + " has downloaded this file: " + file_name);
        return SUCCESSFUL_DOWNLOAD;
    }

    return SERVER_ERROR;
}

string FTP::help_command(int client_sd)
{
    map<int,client_ftp*>::iterator it;
    it = connected_users.find(client_sd);

    if (it == connected_users.end() || !it->second->does_user_have_permission)
        return ACC_IS_NECES;
    
    ifstream inFile;
    inFile.open(HELP_FILE_DIR);
    stringstream strStream;
    strStream << inFile.rdbuf();
    string result = strStream.str();
    
    return result;
}

bool FTP::is_true(string cmd_input,string cmd_matched,int cmd_size,int cmd_size_matched){
    if (cmd_input == cmd_matched && cmd_size == cmd_size_matched)
        return true;
    return false;   
}

string FTP::cmd_handler(char command[], int client_sd)
{
    map<int,client_ftp*>::iterator it;
    it = connected_users.find(client_sd);
    if (it != connected_users.end())
    {
        it->second->data = "";
        it->second->has_data = false;
    }

    string cmd(command);

    vector<string> splitted_cmd = split(cmd);

    if (splitted_cmd.size() == 0)
        return CMD_ERROR;
    
    int splitted_sze = splitted_cmd.size();

    if (is_true(splitted_cmd[0],USR_CMD,splitted_sze,2))
        return user_command(splitted_cmd, client_sd);
    else if (is_true(splitted_cmd[0],LS_CMD,splitted_sze,1))
        return ls_command(client_sd);
    else if (is_true(splitted_cmd[0],MKD_CMD,splitted_sze,2))
        return mkd_command(splitted_cmd[1], client_sd);
    else if (is_true(splitted_cmd[0],PWD_CMD,splitted_sze,1))
        return pwd_command(client_sd);
    else if (is_true(splitted_cmd[0],QUIT_CMD,splitted_sze,1))
        return quit_command(client_sd);

    else if (is_true(splitted_cmd[0],PASS_CMD,splitted_sze,2))
        return password_command(splitted_cmd, client_sd);
    
    else if (splitted_cmd[0] == CWD_CMD)
    {
        switch(splitted_cmd.size())
        {
            case 1:
                return cwd_command("", client_sd);
                break;
            case 2:
                return cwd_command(splitted_cmd[1], client_sd);
                break;
        }    
    }
    
    else if (is_true(splitted_cmd[0],RENAME_CMD,splitted_sze,3))
        return rename_command(splitted_cmd[1], splitted_cmd[2], client_sd);
    
    else if (is_true(splitted_cmd[0],DELETE_COMMAND,splitted_sze,3) && (splitted_cmd[1] == FILE_FLAG || splitted_cmd[1] == DIRECTORY_FLAG))
        return dele_command(splitted_cmd[1], splitted_cmd[2], client_sd);
    
    else if (is_true(splitted_cmd[0],RETRIEVE_CMD,splitted_sze,2))
        return download_command(splitted_cmd[1], client_sd);
    
    else if (is_true(splitted_cmd[0],HELP_CMD,splitted_sze,1))
        return help_command(client_sd);

    return CMD_ERROR;
}

void FTP::remove_user(int client_sd)
{
    map<int,client_ftp*>::iterator it;

    it = connected_users.find(client_sd);

    if (it != connected_users.end())
    {
        write_log("This user logged out: " + it->second->user_info->get_username());
        connected_users.erase(client_sd);
    }
}

vector<User*> FTP::get_users()
{
    return users_vec;
}

vector<string> FTP::get_admin_files()
{
    return admin_files;
}

string FTP::rename_command(string old_name, string new_name, int client_sd)
{
    map<int,client_ftp*>::iterator it;
    it = connected_users.find(client_sd);

    if (it == connected_users.end() || !it->second->does_user_have_permission)
        return ACC_IS_NECES;

    string dir_name = it->second->dir_curr;
    if (!file_exists(old_name, dir_name))
        return SERVER_ERROR;

    string old_addr = dir_name + "/" + old_name;
    string new_addr = dir_name + "/" + new_name;
    if (rename(old_addr.c_str(), new_addr.c_str()) == 0)
    {
        write_log("This user:" + it->second->user_info->get_username() + " in this directory " + dir_name + " has renamed file from " + old_name + " to " + new_name);
        return SUCCESSFUL_CHANGE;
    }
    else
        return SERVER_ERROR;
}