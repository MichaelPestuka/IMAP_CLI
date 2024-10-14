#include "argparser.hpp"

Argparser::Argparser(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++)
    {
        std::cout << "parsing arg " << i << " = " << argv[i] << std::endl;
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
            if(*next != '\0')
            {
                BadValueError(argv[i]); 
            }
        }

        // TLS
        else if(strcmp(argv[i], "-T") == 0)
        {
            i++;
            imaps = true;
        }
        // certificate
        else if(strcmp(argv[i], "-c") == 0)
        {
            i++;
            if(i >= argc)
            {
                valid_arguments = false;
                MissingValueError(argv[i]);
                break;
            }
            certfile.assign(argv[i]);
            use_certfile = true;
        }

        // certificate file
        else if(strcmp(argv[i], "-C") == 0)
        {
            i++;
            if(i >= argc)
            {
                valid_arguments = false;
                MissingValueError(argv[i]);
                break;
            }
            certfolder.assign(argv[i]);
            use_certfile = true;
        }
        // only new
        else if(strcmp(argv[i], "-n") == 0)
        {
            i++;
            only_new = true;
        }
        // only headers
        else if(strcmp(argv[i], "-h") == 0)
        {
            i++;
            only_headers = true;
        }
        
        // auth file
        else if(strcmp(argv[i], "-a") == 0)
        {
            i++;
            if(i >= argc)
            {
                valid_arguments = false;
                MissingValueError(argv[i]);
                break;
            }
            authfile.assign(argv[i]);
            use_authfile = true;
        }
        // mailbox
        else if(strcmp(argv[i], "-b") == 0)
        {
            i++;
            if(i >= argc)
            {
                valid_arguments = false;
                MissingValueError(argv[i]);
                break;
            }
            mailbox.assign(argv[i]);
        }
        // output folder
        else if(strcmp(argv[i], "-o") == 0)
        {
            i++;
            if(i >= argc)
            {
                valid_arguments = false;
                MissingValueError(argv[i]);
                break;
            }
            outdir.assign(argv[i]);
            if(outdir[outdir.length()] != '/') // add a slash if not present
            {
                outdir += '/';
            }
            use_outdir = true;
        }
        else
        {
            // server
            if(i == 1)
            {
                server.assign(argv[i]);
                provided_server = true;
            }
            else
            {
                UnknownOptionError(argv[i]);
                valid_arguments = false;
            }
        }
    } 
}

bool Argparser::AreArgsValid()
{
    if(valid_arguments == false)
    {
        return false;
    }
    if(!provided_server)
    {
        std::cout << "Missing server address" << std::endl;
        return false;
    }
    if(use_certfile && use_certfolder)
    {
        std::cout << "Cannot use both -c and -C at once" << std::endl;
        return false;
    }
    if(!use_authfile)
    {
        std::cout << "Missing authfile argument (-a)" << std::endl;
        return false;
    }
    if(!use_outdir)
    {
        std::cout << "Missing output directory argument (-o)" << std::endl;
        return false;
    }
    return true;
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