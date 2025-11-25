#ifndef PARSER_HPP
#define PARSER_HPP

#include "Server.hpp"

class Parser
{
    private:
        Server& _server;
    public:
        Parser(Server &server);

        void parser(Server::iteratorClient& itClient, const char *buffer);
        void pass(Server::iteratorClient& itClient, const std::vector<std::string>& tokens);
        void nick(Server::iteratorClient& itClient, const std::vector<std::string>& tokens);
        void user(Server::iteratorClient& itClient, const std::vector<std::string>& tokens);
        void join(Server::iteratorClient& itClient, const std::vector<std::string>& tokens);
        void part(Server::iteratorClient& itClient, const std::vector<std::string>& tokens);
        void privmsg(Server::iteratorClient& itClient, const std::vector<std::string>& tokens);
        void quit(Server::iteratorClient& itClient, const std::vector<std::string>& tokens);
        void kick(Server::iteratorClient& itClient, const std::vector<std::string>& tokens);
        void topic(Server::iteratorClient& itClient, const std::vector<std::string>& tokens);
        void mode(Server::iteratorClient& itClient, const std::vector<std::string>& tokens);
        void bot(Server::iteratorClient& itClient, const std::vector<std::string>& tokens);




        ~Parser();

};



#endif