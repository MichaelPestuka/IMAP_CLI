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
        void WaitUntilReply();
        void ProcessReceivedMessage(std::string message);
        void ListenForReply();
        fsm_state current_state;
        std::string sent_message_id;       
    private:
        void SaveSearchUIDs(std::string response, std::queue<std::string> *uid_queue);
        int FSM::WaitForFullAnswer(std::string* answer, std::string message_id);
        std::queue<std::string> mail_ids;
        std::mutex state_lock;
        std::condition_variable state_cv; 
        auth_data authdata;
        Argparser *args;
        Connection *connect;        
        std::string current_response_data;
};

#endif