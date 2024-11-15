/**
 * @author Michael Pestuka (xpestu01)
 */

#ifndef H_FSM
#define H_FSM

#include <mutex>
#include <condition_variable>
#include <queue>
#include <regex>

#include "argparser.hpp"
#include "connection.hpp"
#include "fileops.hpp"

enum fsm_state {START, AUTH, SEARCH, INBOX, FETCH, LOGOUT, END, ERR, SHUTDOWN};

/**
 * Class handling program logic, modelled as a finite state machine
 */
class FSM
{
    public:
        /**
         * Constructor
         * @param args Parsed CLI arguments
         * @param authdata User login data
         * @param connect Initialized connection to IMAP server
         */
        FSM(Argparser *args, auth_data authdata, Connection *connect);

        /**
         * Function to run the FSM
         */
        void FSMLoop();
        fsm_state current_state;
        std::string sent_message_id;       
    private:
        /**
         * Finds returned email UIDs in SEARCH command response
         */
        void SaveSearchUIDs();

        /**
         * Reads response from server
         */
        int WaitForFullAnswer();

        /**
         * Extracts email contents from server response
         */
        std::string ExtractEmailBody();

        /**
         * Extracts UIDVALIDITY number from SELECT command response
         */
        void ExtractUIDValidity();

        std::string uidvalidity;
        int downloaded_email_count;
        std::queue<std::string> mail_ids;
        auth_data authdata;
        Argparser *args;
        Connection *connect;        
        std::string current_response_data;
};

#endif