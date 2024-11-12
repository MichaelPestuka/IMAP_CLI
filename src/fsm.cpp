#include "fsm.hpp"

FSM::FSM(Argparser *args, auth_data authdata, Connection *connect)
{
    this->args = args;
    this->authdata = authdata;
    current_state = fsm_state::START;
    current_response_data = "";
    sent_message_id = "*"; // No starting message ID
    this->connect = connect;
    this->uidvalidity = "0";
    this->downloaded_email_count = 0;
}

int FSM::WaitForFullAnswer()
{
    if(sent_message_id == "")
    {
        current_state = fsm_state::ERR;
        return 1;
    }

    std::string received_part = connect->Receive();
    current_response_data = received_part;
    while (received_part.find(sent_message_id) == std::string::npos)
    {
        // if reading returns 0, there is an error while reading, most likely connection closed
        if(received_part == "\0")
        {
            std::cout << "Error reading from server" << std::endl;
            return 1;
        }
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
    while(current_state != fsm_state::SHUTDOWN)
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
                    std::cerr << "Couldn't connect to server" << std::endl;
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
                    std::cerr << "Couldn't authenticate user" << std::endl;
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
                    ExtractUIDValidity();
                }
                else
                {
                    std::cerr << "Couldn't select mailbox " << args->mailbox << std::endl;
                    next_state = fsm_state::ERR;
                }
                break;
            }

            case fsm_state::SEARCH:
            {
                if(args->only_new)
                {
                    sent_message_id = connect->Send("UID SEARCH NEW");
                }
                else
                {
                    sent_message_id = connect->Send("UID SEARCH ALL");
                }
                int success = WaitForFullAnswer();
                if(success == 0)
                {
                    SaveSearchUIDs();
                    next_state = fsm_state::FETCH;
                }
                else
                {
                    std::cerr << "SEARCH command failed" << std::endl;
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
                    // Headers and full emails are saved into separate files

                    // check if file named LOGIN_INBOX_UID_HEADER already exists (message is already saved)
                    if(args->only_headers)
                    {
                        if(!CheckIfFileExists(args->outdir + authdata.login + "_" + args->mailbox + "_" + uidvalidity + "_" + mail_ids.front() + "_HEADER"))
                        {
                            sent_message_id = connect->Send("UID FETCH " + mail_ids.front() + " BODY.PEEK[HEADER]"); // checking headers wont change SEEN state
                        }
                        else
                        {
                            mail_ids.pop();
                            break;
                        }
                    }
                    else
                    {
                        // Check if file named LOGIN_INBOX_UID exists
                        if(!CheckIfFileExists(args->outdir + authdata.login + "_" + args->mailbox + "_" + uidvalidity + "_" + mail_ids.front()))
                        {
                            sent_message_id = connect->Send("UID FETCH " + mail_ids.front() + " BODY[]");
                        }
                        else
                        {
                            mail_ids.pop();
                            break;
                        }
                    }
                    int success = WaitForFullAnswer();
                    if(success == 0)
                    {
                        downloaded_email_count++;
                        std::string email_body = ExtractEmailBody();
                        WriteToFile(args->outdir + authdata.login + "_" + args->mailbox + "_" + uidvalidity + "_" + mail_ids.front(), email_body); // save email contents to INBOX_UID file
                        mail_ids.pop();
                    }
                    else
                    {
                        std::cerr << "Couldn't fetch email with UID " << mail_ids.front() << std::endl;
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
                std::cout << "Downloaded " << downloaded_email_count << " email(s) from mailbox " << args->mailbox << std::endl;
                next_state = fsm_state::SHUTDOWN;
                break;
            }

            case fsm_state::ERR:
            {
                std::cerr << "Exiting program because an error occured" << std::endl;
                next_state = fsm_state::SHUTDOWN;
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

std::string FSM::ExtractEmailBody()
{
    int body_start = current_response_data.find("BODY");
    std::string body = current_response_data.substr(body_start);
    int size_start = body.find("{") + 1;
    int size_end = body.find("}");
    int body_size = std::stoi(body.substr(size_start, size_end - size_start));

    return body.substr(size_end + 3, body_size - 3); // +3 to skip CRFL after }, -3 to compensate for this skip
}


void FSM::ExtractUIDValidity()
{
    uidvalidity = "";
    size_t uid_validity_pos = current_response_data.find("* OK [UIDVALIDITY ");
    if(uid_validity_pos != std::string::npos)
    {
        uid_validity_pos += 18;
        while(current_response_data[uid_validity_pos] >= 48 && current_response_data[uid_validity_pos] <= 57)
        {
            uidvalidity += current_response_data[uid_validity_pos];

            uid_validity_pos += 1;
        }
    }
    else
    {
        std::cerr << "Error: UIDVALIIDTY not found" << std::endl;
    }
}