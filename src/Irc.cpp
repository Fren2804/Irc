#include <iostream>
#include <cstring>
#include <csignal>
#include <arpa/inet.h>     

#include "Server.hpp"
#include "Utils.h"

int g_running = true;

void handleSigint(int)
{
    g_running = false;
}

int main(int argc, char const *argv[])
{
    int port;
    std::string password;

    if (checkArgs(argc, argv, &port, password))
    {
        return (1);
    }

    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handleSigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    Server irc = Server(port, password);

    irc.configSocket();
    irc.running(&g_running);

    std::cout << std::endl;

    return 0;
}
