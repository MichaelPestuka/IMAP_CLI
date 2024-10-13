#include "fsm.hpp"

FSM::FSM(Argparser *args, auth_data authdata, Connection *connect)
{
    this->args = args;
    this->authdata = authdata;
    current_state = fsm_state::START;
    current_response_data = "";
    this->connect = connect;
}

int FSM::WaitForFullAnswer()
{
    current_response_data = "";
    std::string received_part = connect->Receive();
    while (received_part.find(sent_message_id) == std::string::npos)
    {
        current_response_data += received_part;
        received_part = connect->Receive();
    }
    if(received_part.find("OK") != std::string::npos)
    {
        return 0;
    }     
    return 1;
}

void FSM::FSMLoop()
{
    while(current_state != fsm_state::END)
    {
        fsm_state next_state = current_state;

        switch(current_state)
        {
            case fsm_state::START:
            {
                int success = WaitForFullAnswer();
                if(success == 0) // * OK READY
                {
                    next_state = fsm_state::AUTH;
                }
                else
                {
                    next_state = fsm_state::ERR; // Server connection not successful
                }
                break;
            }
            
            case fsm_state::AUTH:
            {
                sent_message_id = connect->Send("LOGIN " + authdata.login + " " + authdata.password);
                int success = WaitForFullAnswer();
                if(success == 0)
                {
                    next_state = fsm_state::INBOX;
                }
                else
                {
                    next_state = fsm_state::ERR;
                }
                break;
            }

            case fsm_state::INBOX:
            {
                sent_message_id = connect->Send("SELECT " + args->mailbox);
                int success = WaitForFullAnswer();
                if(success == 0)
                {
                    next_state = fsm_state::SEARCH;
                }
                else
                {
                    next_state = fsm_state::ERR;
                }
                break;
            }

            case fsm_state::SEARCH:
            {
                sent_message_id = connect->Send("UID SEARCH ALL"); //TODO arg parsing
                int success = WaitForFullAnswer();
                if(success == 0)
                {
                    SaveSearchUIDs();
                    next_state = fsm_state::FETCH;
                }
                else
                {
                    next_state = fsm_state::ERR;
                }
                break;
            }

            case fsm_state::FETCH:
            {
                if(mail_ids.size() <= 0)
                {
                    next_state = fsm_state::LOGOUT;
                }
                else
                {
                    // check if message already saved
                    sent_message_id = connect->Send("UID FETCH " + mail_ids.front() + " BODY[]");
                    mail_ids.pop();
                    int success = WaitForFullAnswer();
                    if(success == 0)
                    {
                        // save message
                    }
                    else
                    {
                        next_state = fsm_state::ERR;
                    }
                }
                break;
            }

            case fsm_state::LOGOUT:
            {
                sent_message_id = connect->Send("LOGOUT");
                // int success = WaitForFullAnswer();
                next_state = fsm_state::END;
                break;
            }

            case fsm_state::END:
            {
                break;
            }

            case fsm_state::ERR:
            {
                std::cout << "Erorr happened" << std::endl;
                next_state = fsm_state::END;
                break;
            }

            default:
            {
                break;
            }
        }

        current_state = next_state;
        current_response_data = "";
        std::cout << current_state << " state with mid " << sent_message_id << std::endl;
        // State is unlocked
    }
}


/**
 * @param response should be formatted like "1 2 3 \n" - taken from a *SEARCH response
 */
void FSM::SaveSearchUIDs()
{

    int pos = current_response_data.find("* SEARCH ");
    std::string current_num = "";
    for (int i = pos + 9; current_response_data[i] != '\n'; i++) // until reaching newline 
    {
        if(current_response_data[i] < 48 || current_response_data[i] > 57) // if not number, push completed number to queue
        {
            mail_ids.push(current_num);
            std::cout << "parsed: " << current_num << std::endl;
            current_num = "";
        }
        else // if number, add char to current number
        {
            current_num += current_response_data[i];
        }
    }
    return;
}