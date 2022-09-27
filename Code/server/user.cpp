#include "user.h"

User::User(string _username, string _password, long _size, bool _is_admin)
{
    username = _username;
    password = _password;
    size = _size;
    is_admin = _is_admin;
}

string User::get_username()
{
    return username;
}

string User::get_password()
{
    return password;
}

int User::get_size()
{
    return size;
}

bool User::get_is_admin()
{
    return is_admin;
}

void User::reduce_user_storage_size(int amount)
{
    size -= amount;
}