#ifndef H_CONNECTION
#define H_CONNECTION

#include <iostream>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "argparser.hpp"

#include <fstream>

class Connection
{
    public:
        Connection(const char *hostname, const char* default_port, Argparser* args);
        virtual ~Connection();
        virtual int Connect();
        virtual std::string Send(std::string message);
        virtual std::string Receive();
    protected:
        Argparser* args;
        int message_id;
        int client_socket;
        struct addrinfo *resolved_data;
        sockaddr_in server_address;
};

class TLSConnection : public Connection
{
    using Connection::Connection;
    public:
        int Connect();
        ~TLSConnection();
        std::string Send(std::string message);
        std::string Receive();
    private:
        SSL* ssl = nullptr;
        SSL_CTX* ssl_ctx = nullptr;
        int sock;
        void InitializeSSL();
        void DestroySSL();
        
};

class UnsecuredConnection : public Connection
{
    using Connection::Connection;
    public:
        int Connect();
        ~UnsecuredConnection();
        std::string Send(std::string message);
        std::string Receive();

};

#endif