/**
 * @author Michael Pestuka (xpestu01)
 */

#ifndef H_FILEOPS
#define H_FILEOPS

#include <iostream>
#include <fstream>


/**
 * @struct Structure holding user login data
 */
struct auth_data
{
    std::string login;
    std::string password;
};

/**
 * Writes string to file
 * @param path Path to written file
 * @param contents Written data
 * @return 0 if operation successful
 */
int WriteToFile(std::string path, std::string contents);

/**
 * Reads login credentials from file
 * @param path Path to authorization file
 * @param data pointer to structure holding the data
 * @return 0 if operation successful
 */
int ReadAuthfile(std::string path, struct auth_data *data);

/**
 * Checks if a file exists
 * @param path Path to checked file
 * @return true if file exists
 */
bool CheckIfFileExists(std::string path);

#endif