#include <string>
#include <iostream>
#include <cstring>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <stdlib.h>

int checkPort(const char *port)
{
    int i;

    i = 0;
    while (port[i] != '\0')
    {
        if (!isdigit(port[i]))
        {
            return (1);
        }
        i ++;
    }
    return (0);
}

int checkArgs(int argc, const char *argv[], int *port, std::string& password)
{
    if (argc != 3)
    {
        std::cerr << "Incorrect number of arguments." << std::endl;
        std::cout << "./irc port password" << std::endl;
        return (1);
    }
    else if (checkPort(argv[1]))
    {
        std::cerr << "The port can only have numbers." << std::endl;
        return (1);
    }
    *port = std::atoi(argv[1]);
    if (*port > 65535 || *port < 1024)
    {
        std::cerr << "The port must be between 1024 and 65535." << std::endl;
        return (1);
    }
    else if (strlen(argv[2]) <= 0)
    {
        std::cerr << "The password must be at least one character long." << std::endl;
        return (1);
    }
    password = argv[2];
    return (0);
}

bool allDigit(const std::string& str)
{
    for (std::size_t i = 0; i < str.size(); i ++)
    {
        if (!isdigit(str[i]))
        {
            return (false);
        }
    }
    return (true);
}