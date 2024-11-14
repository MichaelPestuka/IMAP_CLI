#ifndef H_ARGPARSER
#define H_ARGPARSER

#include <string.h>
#include <iostream>

class Argparser
{
    public: 
        Argparser(int argc, char* argv[]);
        bool AreArgsValid();
        bool valid_arguments = true;

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
        void MissingValueError(char* bad_option);
        void BadValueError(char* bad_option);
        void UnknownOptionError(char* unknown);
        bool provided_authfile;
        bool provided_outdir;
};

#endif