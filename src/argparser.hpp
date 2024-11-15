/**
 * @author Michael Pestuka (xpestu01)
 */

#ifndef H_ARGPARSER
#define H_ARGPARSER

#include <string.h>
#include <iostream>

/**
 * Class for parsing and storing CLI arguments
 */
class Argparser
{
    public: 
        /**
        *   Constructor, takes the CLI arguments and parses them into usable data
        *   @param argc Number of CLI arguments
        *   @param argv CLI arguments
        */
        Argparser(int argc, char* argv[]);
        /**
         * Checks if the parsed argument configuration is valid
         * @return bool true if valid 
         */
        bool AreArgsValid();
        bool valid_arguments = true;

        // Parameters parsed from CLI and their default values

        std::string server;
        bool provided_server;

        std::string port = "";
        bool provided_port = false;

        bool imaps = false;

        bool use_certfile = false;
        std::string certfile;
        bool use_certfolder = false;
        std::string certfolder = "/etc/ssl/certs";

        bool only_new = false;
        bool only_headers = false;

        bool use_authfile = false;
        std::string authfile;

        std::string mailbox = "INBOX";

        bool use_outdir;
        std::string outdir;

    private:

        /**
         * Error printing function
         * @param bad_option Option with invalid value
         */
        void MissingValueError(char* bad_option);

        /**
         * Error printing function
         * @param bad_option Option with invalid value
         */
        void BadValueError(char* bad_option);

        /**
         * Error printing function
         * @param bad_option Option with invalid value
         */
        void UnknownOptionError(char* unknown);

        bool provided_authfile;
        bool provided_outdir;
};

#endif