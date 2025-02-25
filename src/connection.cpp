/**
 * @author Michael Pestuka (xpestu01)
 */

#include "connection.hpp"

Connection::Connection(const char* hostname, const char* default_port, Argparser* args)
{
    // Message id counts how many messages were sent, is prepended to every sent message
    message_id = 0;

    this->args = args;
    
    struct addrinfo hints, *p;
    memset(&hints, 0, sizeof(hints));

    // Connection parameters setup (TCP over IPV4)
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve hostname
    int resolve_success;
    if(args->provided_port)
    {
        resolve_success = getaddrinfo(hostname, args->port.c_str(), &hints, &resolved_data);
    }
    else
    {
        resolve_success = getaddrinfo(hostname, default_port, &hints, &resolved_data);
    }

    // Check resolution success
    if(resolve_success != 0)
    {
        std::cerr << "Error resolving hostname: " << hostname << " error code " << gai_strerror(resolve_success) << std::endl;
        return;
    }

    // Go through all IPs from resolved name
    for (p = resolved_data; p != NULL; p = p->ai_next)
    {
        char ipstr[INET_ADDRSTRLEN];
        void* addr;
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;

        addr = &(ipv4->sin_addr);
        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
    }

    // Create socket        
    client_socket = socket(resolved_data->ai_family, resolved_data->ai_socktype, resolved_data->ai_protocol);

    // Setting socket timeout
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof(tv));
    setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *) &tv, sizeof(tv));
}

// Virtual function prototypes

Connection::~Connection()
{}

int Connection::Connect()
{
    return 1;
}

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
    if(ssl != nullptr)
    {
        int err = SSL_shutdown(ssl);
        if(err == 0)
        {
            // Second call performs bidirectional shutdown
            SSL_shutdown(ssl);
        }
        if(err == -1)
        {
            std::cerr << "Error during SSL shutdown" << std::endl;
        }
        SSL_free(ssl);
    }

    // Free SSL allocated memory
    if(ssl_ctx != nullptr)
    {
        SSL_CTX_free(ssl_ctx);
    }
    DestroySSL();

    close(client_socket);
    freeaddrinfo(resolved_data);
}

int TLSConnection::Connect()
{
    int err = connect(this->client_socket, resolved_data->ai_addr, resolved_data->ai_addrlen);
    
    if(err < 0)
    {
        std::cerr << "Error connecting" << std::endl;
        return 1;
    }

    InitializeSSL();
    const SSL_METHOD *meth = TLS_client_method();
    ssl_ctx = SSL_CTX_new(meth);

    // Set where certificates for verification are located
    if(args->use_certfile)
    {
        if(SSL_CTX_load_verify_file(ssl_ctx, args->certfile.c_str()) == 0) // Error loading
        {
            std::cerr << "Error loading certificate file, default directory will be used instead" << std::endl;
        }
    }
    else if(args->use_certfolder)
    {
        if(SSL_CTX_load_verify_dir(ssl_ctx, args->certfolder.c_str()) == 0)
        {
            std::cerr << "Error loading certificate directory, default directory will be used instead" << std::endl;
        }
    }

    // Create SSL from data
    ssl = SSL_new(ssl_ctx);

    //check error
    if(!ssl)
    {
        std::cerr << "Error creating SSL" <<  std::endl;
        return 1;
    }

    // Assign socket to SSL
    sock = SSL_get_fd(ssl);
    SSL_set_fd(ssl, client_socket);
    err = SSL_connect(ssl);

    if(err <= 0)
    {
        int ret = ERR_get_error();
        char *str = ERR_error_string(ret, NULL);
        std::cerr << "Error connecting ssl - " << str << std::endl;
        return 1;
    }
    return 0;
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
        std::cerr << "Error sending message to server" << std::endl;
        return "";
    }
    return "msg" + std::to_string(message_id); // return sent message ID
}

std::string TLSConnection::Receive()
{
    char buf[1000];
    int len = SSL_read(ssl, buf, 1000);
    buf[len] = 0;
    return std::string(buf);
}

// UNSECURED CONNECTION CODE --------------------------------------------------------------------------

int UnsecuredConnection::Connect()
{
    // TODO check all resolved addresses
    int err = connect(this->client_socket, resolved_data->ai_addr, resolved_data->ai_addrlen);
    
    if(err < 0)
    {
        std::cerr << "Error connecting to server" << std::endl;
        return 1;
    }
    return 0;
}

std::string UnsecuredConnection::Send(std::string message)
{
    message_id += 1;
    std::string formatted_message = "msg" + std::to_string(message_id) + " " + message + "\r\n";
    int len = write(client_socket, formatted_message.c_str(), formatted_message.length());
    if(len < 0)
    {
        std::cerr << "Error sending message to server" << std::endl;
    }
    return "msg" + std::to_string(message_id); // return sent message ID
}

std::string UnsecuredConnection::Receive()
{
    char buf[1000];
    int len = recv(client_socket, buf, 1000, 0);
    buf[len] = 0;
    return std::string(buf);
}

UnsecuredConnection::~UnsecuredConnection()
{
    close(client_socket);
    freeaddrinfo(resolved_data);
}