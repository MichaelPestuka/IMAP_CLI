#include "fileops.hpp"

int WriteToFile(std::string path, std::string contents)
{
    std::ofstream outfile(path);
    if(outfile.is_open())
    {
        outfile << contents << std::endl; //TODO endl ma byt?
        outfile.close();
    }
    else
    {
        std::cout << "Couldnt open " << path << std::endl;
        return 1;
    }
    return 0;
}

int ReadAuthfile(std::string path, struct auth_data *data)
{
    std::ifstream authfile(path); 
    if(authfile.is_open())
    {
        std::string login;
        getline(authfile, login);
        std::string password;
        getline(authfile, password);

        if(login.substr(0, 11) == "username = ")
        {
            data->login = login.substr(11);
        }
        else
        {
            std::cout << "bad authfile format" << std::endl;
            return 1;
        }
        if(password.substr(0, 11) == "password = ")
        {
            data->password = password.substr(11);
        }
        else
        {
            std::cout << "bad authfile format" << std::endl;
            return 1;
        }
        authfile.close();
    }
    else
    {
        std::cout << "Unable to open " << path << std::endl;
        return 1;
    }
    return 0;
}