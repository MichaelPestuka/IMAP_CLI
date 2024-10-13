#ifndef H_FSM
#define H_FSM

#include <mutex>
#include <condition_variable>
#include <queue>

#include "argparser.hpp"
#include "connection.hpp"
#include "fileops.hpp"

enum fsm_state {START, AUTH, SEARCH, INBOX, FETCH, LOGOUT, END, ERR};


class FSM
{
    public:
        FSM(Argparser *args, auth_data authdata, Connection *connect);
        void FSMLoop();
        fsm_state current_state;
        std::string sent_message_id;       
    private:
        void SaveSearchUIDs();
        int WaitForFullAnswer();
        std::queue<std::string> mail_ids;
        auth_data authdata;
        Argparser *args;
        Connection *connect;        
        std::string current_response_data;
};

#endif