/**
 * @author Michael Pestuka (xpestu01)
 */


#include "fsm.hpp"

FSM::FSM(Argparser *args, auth_data authdata, Connection *connect)
{
    this->args = args;
    this->authdata = authdata;
    current_state = fsm_state::START;
    current_response_data = "";
    sent_message_id = "*"; // No starting message ID, since first expected message is "* OK ..."
    this->connect = connect;
    this->uidvalidity = "0";
    this->downloaded_email_count = 0;
}

int FSM::WaitForFullAnswer()
{
    // Check if message id is valid (probably unnecessary)
    if(sent_message_id == "")
    {
        current_state = fsm_state::ERR;
        return 1;
    }

    // Loop until the sent message id is found - response ends
    while (true)
    {
        std::string received = connect->Receive();

        // If server times out or connection closes
        if(received == "\0")
        {
            std::cerr << "Error reading from server" << std::endl;
            return 1;
        }
        current_response_data += received;

        // Look for message ID + OK|NOK|BAD in response
        if(current_response_data.find(sent_message_id) != std::string::npos)
        {
            if(current_response_data.find(sent_message_id + " OK") != std::string::npos)
            {
                return 0;
            }
            else if(current_response_data.find(sent_message_id + " NOK") != std::string::npos || current_response_data.find(sent_message_id + " BAD") != std::string::npos)
            {
                return 1;
            }
        }
    }

    return 1;
}

// Correct flow should be START -> AUTH -> INBOX -> SEARCH -> FETCH -> LOGOUT -> END -> SHUTDOWN
void FSM::FSMLoop()
{
    // Run until shutdown
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
                    std::cerr << "Error: Couldn't connect to server" << std::endl;
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
                    std::cerr << "Error: Couldn't authenticate user" << std::endl;
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
                    std::cerr << "Error: Couldn't select mailbox " << args->mailbox << std::endl;
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
                    std::cerr << "Error: SEARCH command failed" << std::endl;
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
                    // filename format is email_mailbox_UID_UIDVALIDITY
                    // if only header is saved _HEADER is added to filename

                    std::string email_filename = args->outdir + authdata.login + "_" + args->mailbox + "_" + mail_ids.front() + "_" + uidvalidity;
                    if(args -> only_headers)
                    {
                        email_filename += "_HEADER";
                    }
                    
                    // check if email file already exists
                    if(args->only_headers)
                    {
                        if(!CheckIfFileExists(email_filename))
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
                        // Check if email file already exists
                        if(!CheckIfFileExists(email_filename))
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
                        WriteToFile(email_filename, email_body); // Save email contents to file
                        mail_ids.pop();
                    }
                    else
                    {
                        std::cerr << "Error: Couldn't fetch email with UID " << mail_ids.front() << std::endl;
                        next_state = fsm_state::ERR;
                    }
                }
                break;
            }

            case fsm_state::LOGOUT:
            {
                sent_message_id = connect->Send("LOGOUT");
                int success = WaitForFullAnswer();
                if(success == 0)
                {
                    next_state = fsm_state::END;
                }
                else
                {
                    std::cerr << "Error: Logout failed" << std::endl;
                    next_state = fsm_state::ERR;
                }
                break;
            }

            case fsm_state::END:
            {
                if(args->only_headers)
                {
                    std::cout << "Downloaded " << downloaded_email_count << " header(s) from mailbox " << args->mailbox << std::endl;
                }
                else
                {
                    std::cout << "Downloaded " << downloaded_email_count << " email(s) from mailbox " << args->mailbox << std::endl;
                }
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

        current_state = next_state; // Update state
        current_response_data = ""; // Clear server resonse data
    }
}


void FSM::SaveSearchUIDs()
{

    int pos = current_response_data.find("* SEARCH ");
    std::string current_num = "";
    for (size_t i = pos + 9; i < current_response_data.length(); i++) // until reaching newline 
    {
        if(current_response_data[i] < 48 || current_response_data[i] > 57) // if not number, push completed number to queue
        {
            if(current_num == "")
            {
                return; 
            }
            mail_ids.push(current_num);
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