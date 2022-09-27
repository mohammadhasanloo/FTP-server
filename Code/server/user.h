#ifndef USER
#define USER

#include <string>
#include <filesystem>

using namespace std;

class User{
    public:
        User(string _username, string _password, long _size, bool _is_admin);
        int get_size();
        bool get_is_admin();
        string get_username();
        string get_password();
        void reduce_user_storage_size(int amount);
    private:
        string username;
        string password;
        long size;
        bool is_admin;
};

#endif