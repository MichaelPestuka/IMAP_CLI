#include "connection.hpp"

Connection::Connection(const char* ip, int port)
{
    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Connect to socket
    server_address.sin_family = AF_INET;
    server_address.sin_port=htons(port);
    inet_pton(AF_INET, ip, &(server_address.sin_addr));

    message_id = 0;
}

void Connection::Connect()
{}

std::string Connection::Receive()
{
    return std::string("0");
}

int Connection::Send(std::string message)
{
    return 0;
}

void TLSConnection::Connect()
{
    int err = connect(this->client_socket, (struct sockaddr*)&(server_address), sizeof(server_address));
    
    if(err < 0)
    {
        std::cout << "Error connecting" << std::endl;
        return;
    }

    InitializeSSL();
    const SSL_METHOD *meth = TLS_client_method();
    SSL_CTX *sslctx = SSL_CTX_new(meth);
    ssl = SSL_new(sslctx);
    //check error
    if(!ssl)
    {
        std::cout << "Error creating SSL" <<  std::endl;
        return;
    }

    sock = SSL_get_fd(ssl);
    SSL_set_fd(ssl, client_socket);
    err = SSL_connect(ssl);

    if(err <= 0)
    {
        std::cout << "Error connecting ssl errno: " << err << std::endl;
        int ret = ERR_get_error();
        char *str = ERR_error_string(ret, NULL);
        std::cout << "errno: " << ret << " -> " <<  str << std::endl;
        return;
    }
    std::cout << "SSl setup using " << SSL_get_cipher(ssl) << std::endl;
}

void TLSConnection::InitializeSSL()
{

    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
}

void TLSConnection::DestroySSL()
{

}

int TLSConnection::Send(std::string message)
{
    std::string formatted_message = "msg" + std::to_string(message_id) + " " + message + "\n";
    int len = SSL_write(ssl, formatted_message.c_str(), strlen(formatted_message.c_str()));
    std::cout << "written " << len << " bytes: " << formatted_message << std::endl;
    if(len < 0)
    {
        std::cout << "error sending" << std::endl;
    }
    return 0;
}

std::string TLSConnection::Receive()
{
    std::string full_response;
    std::string current_line;
    std::string current_message_id = "msg" + std::to_string(message_id);
    std::cout << "receiving" << std::endl;
    while(current_line.find(current_message_id) == std::string::npos && current_line.find("server ready") == std::string::npos)
    {
        int len = 100;
        char buf[1000000];
        len = SSL_read(ssl, buf, 100);
        buf[len] = 0;
        std::cout << "received: " << buf << std::endl;
        full_response += buf;
    }
    return full_response;
}