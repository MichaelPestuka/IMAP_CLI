#include <iostream>
#include <fstream>

struct auth_data
{
    std::string login;
    std::string password;
};

int WriteToFile(std::string path, std::string contents);
int ReadAuthfile(std::string path, struct auth_data *data);