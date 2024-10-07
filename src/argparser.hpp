#include <string.h>
#include <iostream>

class Argparser
{
    public: 
        Argparser(int argc, char* argv[]);
        bool valid_arguments = true;

        int port = -1;

        bool imaps = false;

        bool use_certfile = false;
        std::string certfile;
        bool use_certfolder = false;
        std::string certfolder = "/etc/ssl/certs";

        bool only_new = false;
        bool only_headers = false;

        bool use_authfile = false;
        std::string authfile = NULL;

        std::string mailbox = "INBOX";

        bool use_outdir;
        std::string outdir;

    private:
        void MissingValueError(char* bad_option);
        void Argparser::BadValueError(char* bad_option);
        void UnknownOptionError(char* unknown);
        bool provided_authfile;
        bool provided_outdir;
};