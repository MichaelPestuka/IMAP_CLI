/**
 * @author Michael Pestuka (xpestu01)
 */

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

/**
 * Abstract class handling the connection to server, only used as a parent class
 */
class Connection
{
    public:
        /**
         * Common constructor
         * @param hostname Hostname of IMAP server
         * @param default_port Port to be used unless otherwise specified in args
         * @param args Parsed CLI arguments
         */
        Connection(const char *hostname, const char* default_port, Argparser* args);
        virtual ~Connection();

        /**
         * Initializes connection to server
         * @return 0 if successful, otherwise 1
         */
        virtual int Connect();

        /**
         * Send string message to server
         * @return sent message ID in string format (eg. "msg1")
         */
        virtual std::string Send(std::string message);

        /**
         * Receive data from server
         * Will only receive only up to buffer capacity, has to be called multiple times for longer responses
         * @return Received data
         */
        virtual std::string Receive();

    protected:
        Argparser* args;
        int message_id;
        int client_socket;
        struct addrinfo *resolved_data;
        sockaddr_in server_address;
};

/**
 * Class handling a TLS secured connection, implements Connection class functions
 */
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

        /**
         * Initializes openssl for encryption
        */
        void InitializeSSL();

        /**
         * Cleans up after openssl not needed
         */
        void DestroySSL();
        
};

/**
 * Class handling an unsecured connection, implements Connection class functions
 */
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