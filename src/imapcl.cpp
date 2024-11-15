/**
 * @author Michael Pestuka (xpestu01)
 */

#include "connection.hpp"
#include "argparser.hpp"
#include "fileops.hpp"
#include "fsm.hpp"

int main(int argc, char* argv[])
{
    // Parse and check arguments
    Argparser argparser(argc, argv);
    if(!argparser.AreArgsValid())
    {
        std::cerr << "Invalid arguments provided" << std::endl;
        return 1;
    }

    struct auth_data authdata;
    if(ReadAuthfile(argparser.authfile, &authdata) == 1)
    {
        return 1;
    }

    // Initialize connection to server
    Connection* connect;
    if(argparser.imaps)
    {
        connect = new TLSConnection(argparser.server.c_str(), "993", &argparser);
    }
    else
    {
        connect = new UnsecuredConnection(argparser.server.c_str(), "143", &argparser);
    }
    if(connect->Connect() != 0)
    {
        delete(connect);
        return 1;
    }

    // Start program loop
    FSM fsm = FSM(&argparser, authdata, connect);
    fsm.FSMLoop();
    delete(connect);
    return 0;
}