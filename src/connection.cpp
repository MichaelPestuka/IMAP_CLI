#include "connection.hpp"

Connection::Connection(const char* hostname, const char* port, Argparser* args)
{
    message_id = 0;
    this->args = args;
    
    // Resolve hostname
    struct addrinfo hints, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    

    int resolve_success = getaddrinfo(hostname, port, &hints, &resolved_data);
    
    if(resolve_success != 0)
    {
        std::cerr << "Error resolving hostname: " << hostname << " error code " << gai_strerror(resolve_success) << std::endl;
        return;
    }
    for (p = resolved_data; p != NULL; p = p->ai_next)
    {
        char ipstr[INET_ADDRSTRLEN];
        void* addr;
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;

        addr = &(ipv4->sin_addr);
        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        std::cout << "Resolved to: " << ipstr << std::endl;
    }
        
    client_socket = socket(resolved_data->ai_family, resolved_data->ai_socktype, resolved_data->ai_protocol);
}

Connection::~Connection()
{}

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

// TLS CONNECTION CODE --------------------------------------------------------

TLSConnection::~TLSConnection()
{
    SSL_free(ssl);
    SSL_CTX_free(ssl_ctx);
    DestroySSL();
    close(client_socket);
    freeaddrinfo(resolved_data);
}

void TLSConnection::Connect()
{
    int err = connect(this->client_socket, resolved_data->ai_addr, resolved_data->ai_addrlen);
    
    if(err < 0)
    {
        std::cout << "Error connecting" << std::endl;
        return;
    }

    InitializeSSL();
    const SSL_METHOD *meth = TLS_client_method();
    ssl_ctx = SSL_CTX_new(meth);
    if(args->use_certfile)
    {
        // SSL_CTX_use_certificate_file(ssl_ctx, args->certfile.c_str(), SSL_FILETYPE_PEM); // Only PEM certificates, idc anymore
        SSL_CTX_load_verify_file(ssl_ctx, args->certfile.c_str()); // May work :>
    }
    else if(args->use_certfolder)
    {
        SSL_CTX_load_verify_dir(ssl_ctx, args->certfolder.c_str());
    }
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
    std::string formatted_message = "msg" + std::to_string(message_id) + " " + message + "\r\n";
    int len = SSL_write(ssl, formatted_message.c_str(), formatted_message.length());
    if(len < 0)
    {
        std::cout << "error sending message" << std::endl;
    }
    std::cout << "wrt: " << formatted_message << std::endl; 
    return "msg" + std::to_string(message_id); // return sent message ID
}

std::string TLSConnection::Receive()
{
    char buf[1000];
    int len = SSL_read(ssl, buf, 1000);
    buf[len] = 0;
    std::cout << "rec: " << buf << std::endl; 
    return std::string(buf);
}

// UNSECURED CONNECTION CODE --------------------------------------------------------------------------

void UnsecuredConnection::Connect()
{
    // TODO check all resolved addresses
    int err = connect(this->client_socket, resolved_data->ai_addr, resolved_data->ai_addrlen);
    
    if(err < 0)
    {
        std::cout << "Error connecting" << std::endl;
        return;
    }
}

std::string UnsecuredConnection::Send(std::string message)
{
    message_id += 1;
    std::string formatted_message = "msg" + std::to_string(message_id) + " " + message + "\r\n";
    int len = write(client_socket, formatted_message.c_str(), formatted_message.length());
    if(len < 0)
    {
        std::cout << "error sending message" << std::endl;
    }
    std::cout << "wrt: " << formatted_message << std::endl;
    return "msg" + std::to_string(message_id); // return sent message ID
}

std::string UnsecuredConnection::Receive()
{
    char buf[1000];
    int len = recv(client_socket, buf, 1000, 0);
    buf[len] = 0;
    std::cout << "rec: " << buf << std::endl;
    return std::string(buf);
}

UnsecuredConnection::~UnsecuredConnection()
{
    close(client_socket);
    freeaddrinfo(resolved_data);
}