#include "fsm.hpp"

FSM::FSM(Argparser *args, auth_data authdata, Connection *connect)
{
    this->args = args;
    this->authdata = authdata;
    current_state = fsm_state::START;
    current_response_data = "";
    this->connect = connect;
}

void FSM::WaitUntilReply()
{
    while(current_state != fsm_state::END || current_state != fsm_state::ERR)
    {
        std::unique_lock<std::mutex> lock{state_lock};
        state_cv.wait(lock, []() {return true;}); // will unlock for others until notified
        SendNextCommand();
    }
}

void FSM::ListenForReply()
{
    std::cout << "Starting ListenForReply()" << std::endl;
    while(current_state != fsm_state::END)
    {
        std::string message = connect->Receive();
        ProcessReceivedMessage(message);
    }
    std::cout << "finishing ListenForReply()" << std::endl;
}

void FSM::ProcessReceivedMessage(std::string message)
{
    fsm_state next_state = current_state;

    // Lock state
    std::lock_guard<std::mutex> l{state_lock};
    switch(current_state)
    {
        case fsm_state::START:
            if(message.find("OK") != std::string::npos)
            {
                next_state = fsm_state::AUTH;
                sent_message_id = connect->Send("LOGIN " + authdata.login + " " + authdata.password);
            }
        break;
        
        case fsm_state::AUTH:
            if(message.find(sent_message_id) != std::string::npos && message.find("OK") != std::string::npos)
            {
                next_state = fsm_state::INBOX;
                sent_message_id = connect->Send("SELECT " + args->mailbox);
            }
            else
            {
                std::cout << "wut" << std::endl;
            }
        break;

        case fsm_state::INBOX:
            if(message.find(sent_message_id) != std::string::npos)
            {
                if(message.find("OK") != std::string::npos)
                {
                    //TODO load data
                    sent_message_id = connect->Send("UID SEARCH ALL");
                    next_state = fsm_state::FETCH;
                }
                else if(message.find("NO") != std::string::npos || message.find("BAD") != std::string::npos)
                {
                    next_state = fsm_state::ERR;
                }
            }
            else 
            {
                current_response_data += message;
                return;
            }
        break;

        case fsm_state::FETCH:
            if(message.find(sent_message_id) != std::string::npos)
            {
                if(current_response_data.find("* SEARCH ") != std::string::npos)
                {
                    int pos = current_response_data.find("* SEARCH ");
                    SaveSearchUIDs(current_response_data.substr(pos + 9), &mail_ids); // Get UID from after * SEARCH until \n
                }

                if(message.find("OK") != std::string::npos)
                {
                    if(mail_ids.size() == 0)
                    {
                        next_state = fsm_state::LOGOUT;
                        sent_message_id = connect->Send("LOGOUT");
                    }
                    else
                    {
                        std::cout << "nextuid: " << mail_ids.front() << " end" << std::endl;
                        sent_message_id = connect->Send("UID FETCH " + mail_ids.front() + " BODY[]");
                        mail_ids.pop();
                    }
                    //TODO check jestli vsechny uz jsou stazene
                    // sent_message_id = connect->Send("FETCH 1"); //TODO fetch
                    // sent_message_id = connect->Send("FETCH 1 BODY[]"); //TODO fetch
                }
                else if(message.find("NO") != std::string::npos || message.find("BAD") != std::string::npos)
                {
                    next_state = fsm_state::ERR;
                }
            }
            else 
            {
                current_response_data += message;
                return;
            }
        break;

        case fsm_state::LOGOUT:
            if(message.find(sent_message_id) != std::string::npos && message.find("OK") != std::string::npos)
            {
                //end connection
                next_state = fsm_state::END;
            }
            else if(message.find(sent_message_id) != std::string::npos && message.find("BAD") != std::string::npos)
            {

                next_state = fsm_state::ERR;
            }
        break;

        case fsm_state::END:
        break;

        case fsm_state::ERR:
            std::cout << "Erorr happened" << std::endl;
            next_state = fsm_state::END;
        break;

        default:
        break;
    }

    current_state = next_state;
    current_response_data = "";
    state_cv.notify_one();
    std::cout << current_state << " state with mid " << sent_message_id << std::endl;
    // State is unlocked
}

void FSM::SendNextCommand()
{
    switch(current_state)
    {
        case fsm_state::START:
            connect->Send("LOGIN " + authdata.login + " " + authdata.password);
        break;

        case fsm_state::AUTH:
        break;

        case fsm_state::INBOX:
            connect->Send("SELECT " + args->mailbox);
        break;

        case fsm_state::FETCH:
            connect->Send("FETCH 1"); //TODO fetch
        break;

        case fsm_state::LOGOUT:
            connect->Send("LOGOUT");
        break;

        case fsm_state::END:
        break;

        case fsm_state::ERR:
        break;
    }
}


/**
 * @param response should be formatted like "1 2 3 \n" - taken from a *SEARCH response
 */
void FSM::SaveSearchUIDs(std::string response, std::queue<std::string> *uid_queue)
{
    std::string current_num = "";
    for (int i = 0; response[i] != '\n'; i++)
    {
        if(response[i] < 48 || response[i] > 57)
        {
            uid_queue->push(current_num);
            std::cout << "parsed: " << current_num << std::endl;
            current_num = "";
        }
        else
        {
            current_num += response[i];
        }
    }
    return;
}