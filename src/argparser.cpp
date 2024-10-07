#include "argparser.hpp"

Argparser::Argparser(int argc, char* argv[])
{
    for (int i = 0; i < argc; i++)
    {
        // port
        if(strcmp(argv[i], "-p") == 0)
        {
            i++;
            if(i >= argc)
            {
                valid_arguments = false;
                MissingValueError(argv[i]);
                break;
            }
            char *next;
            port = strtol(argv[i], &next, 10);
            if(next != '\0')
            {
                BadValueError(argv[i]); 
            }
        }

        // TLS
        if(strcmp(argv[i], "-T") == 0)
        {
            i++;
            imaps = true;
        }
        // certificate
        if(strcmp(argv[i], "-c") == 0)
        {
            i++;
            if(i >= argc)
            {
                valid_arguments = false;
                MissingValueError(argv[i]);
                break;
            }
            certfile = argv[i];
            use_certfile = true;
        }

        // certificate file
        if(strcmp(argv[i], "-C") == 0)
        {
            i++;
            if(i >= argc)
            {
                valid_arguments = false;
                MissingValueError(argv[i]);
                break;
            }
            certfolder = argv[i];
            use_certfile = true;
        }
        // only new
        if(strcmp(argv[i], "-n") == 0)
        {
            i++;
            only_headers = true;
        }
        // only headers
        if(strcmp(argv[i], "-h") == 0)
        {
            i++;
            only_headers = true;
        }
        
        // auth file
        if(strcmp(argv[i], "-a") == 0)
        {
            i++;
            if(i >= argc)
            {
                valid_arguments = false;
                MissingValueError(argv[i]);
                break;
            }
            authfile = argv[i];
            use_authfile = true;
        }
        // auth file
        if(strcmp(argv[i], "-b") == 0)
        {
            i++;
            if(i >= argc)
            {
                valid_arguments = false;
                MissingValueError(argv[i]);
                break;
            }
            mailbox = argv[i];
        }
        // auth file
        if(strcmp(argv[i], "-o") == 0)
        {
            i++;
            if(i >= argc)
            {
                valid_arguments = false;
                MissingValueError(argv[i]);
                break;
            }
            outdir = argv[i];
            use_outdir = true;
        }
        else
        {
            UnknownOptionError(argv[i]);
            valid_arguments = false;
        }
    }
    
}

void Argparser::BadValueError(char* bad_option)
{
    std::cout << "Argument " << bad_option << " has invalid value" << std::endl;
}

void Argparser::MissingValueError(char* bad_option)
{
    std::cout << "Argument " << bad_option << " is missing a value" << std::endl;
}

void Argparser::UnknownOptionError(char* unknown)
{
    std::cout << "Unknown option " << unknown << std::endl;
}