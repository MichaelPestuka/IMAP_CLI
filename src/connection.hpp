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
        Connection(const char *ip, int port);
        virtual void Connect();
        virtual int Send(std::string message);
        virtual std::string Receive();
    protected:
        int message_id;
        int client_socket;
        sockaddr_in server_address;
};

class TLSConnection : Connection
{
    using Connection::Connection;
    public:
        void Connect();
        int Send(std::string message);
        std::string Receive();
    private:
        SSL* ssl;
        int sock;
        void InitializeSSL();
        void DestroySSL();
        
};