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

std::string Connection::Send(std::string message)
{
    return "";
}

TLSConnection::~TLSConnection()
{
    std::cout << "Cleanup Time" << std::endl;
    SSL_free(ssl);
    SSL_CTX_free(ssl_ctx);
    DestroySSL();
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
    ssl_ctx = SSL_CTX_new(meth);
    ssl = SSL_new(ssl_ctx);
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
    CONF_modules_unload(1);
    ERR_free_strings();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
}

std::string TLSConnection::Send(std::string message)
{
    message_id += 1;
    std::string formatted_message = "msg" + std::to_string(message_id) + " " + message + "\n";
    int len = SSL_write(ssl, formatted_message.c_str(), strlen(formatted_message.c_str()));
    std::cout << "written " << len << " bytes: " << formatted_message << std::endl;
    if(len < 0)
    {
        std::cout << "error sending" << std::endl;
    }
    return "msg" + std::to_string(message_id); // return sent message ID

}

std::string TLSConnection::Receive()
{
    std::string full_response;
    int len = 0;
    char buf[1000000]; //TODO delka bufferu
    len = SSL_read(ssl, buf, 100);
    buf[len] = 0;
    std::cout << "received: " << buf << std::endl;
    full_response = buf;
    return full_response;
}