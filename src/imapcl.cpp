#include <iostream>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <thread>


#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "connection.hpp"
#include "argparser.hpp"
#include "fileops.hpp"
#include "fsm.hpp"

int main(int argc, char* argv[])
{
    Argparser argparser(argc, argv);
    if(!argparser.AreArgsValid())
    {
        std::cout << "Invalid arguments" << std::endl;
        return 1;
    }

    struct auth_data authdata;
    if(ReadAuthfile(argparser.authfile, &authdata) == 1)
    {
        return 1;
    }
    std::cout << "logging in as " << authdata.login << " pass: " << authdata.password << std::endl;

    Connection* connect;
    if(argparser.imaps)
    {
        connect = new TLSConnection(argparser.server.c_str(), "993");
    }
    else
    {
        connect = new UnsecuredConnection(argparser.server.c_str(), "143");
    }
    connect->Connect();
    FSM fsm = FSM(&argparser, authdata, connect);
    fsm.FSMLoop();
    // std::thread receiver(&FSM::FSMLoop, std::ref(fsm));
    // std::thread sender(&FSM::WaitUntilReply, std::ref(fsm));
    // receiver.join();
    // sender.join();
    std::cout << "Endies :>" << std::endl;
    return 0;
}