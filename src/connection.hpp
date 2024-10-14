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


class Connection
{
    public:
        Connection(const char *hostname, const char* port);
        virtual void Connect();
        virtual std::string Send(std::string message);
        virtual std::string Receive();
    protected:
        int message_id;
        int client_socket;
        struct addrinfo *resolved_data;
        sockaddr_in server_address;
};

class TLSConnection : public Connection
{
    using Connection::Connection;
    public:
        void Connect();
        ~TLSConnection();
        std::string Send(std::string message);
        std::string Receive();
    private:
        SSL* ssl;
        SSL_CTX* ssl_ctx;
        int sock;
        void InitializeSSL();
        void DestroySSL();
        
};

class UnsecuredConnection : public Connection
{
    using Connection::Connection;
    public:
        void Connect();
        ~UnsecuredConnection();
        std::string Send(std::string message);
        std::string Receive();

};

#endif