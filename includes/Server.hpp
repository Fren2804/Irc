#ifndef SERVER_HPP
#define SERVER_HPP

#include <fstream>
#include <vector>
#include <map>
#include <poll.h>
#include <sys/socket.h>

#include "Client.hpp"
#include "Channel.hpp"

#define EXIT_FAILURE 1

class Parser;

class Server
{
    public:
        typedef std::vector<pollfd>::iterator   iteratorPDF;
        typedef std::vector<Client*>::iterator   iteratorClient;
        typedef std::map<std::string, Channel>::iterator   iteratorChannel;

    private:
        std::vector<pollfd> _fds;
        std::vector<Client*> _clients;
        std::map<std::string, Channel> _channels;

        std::vector<std::string> _jokes;


        Client * _bot;

        std::ofstream *_log;

        const int _port;
        const std::string _password;
        int _fdSocket;
        int _idClient;
        Parser *_parser;

        Server();
        Server::iteratorClient _searchClientFd(int fd);        
    public:
        Server(int port, const std::string &password);

        void configSocket();
        void running(int *g_running);
        void newClientConnected();
        void errorClientDisconnected(int i);
        void waitClient(int i);

        

        int nonBlock(int fd);
        void closeConexion(int fd);
        void closeConexionQuit(iteratorClient& itClient);
        


        void messageServerClient(iteratorClient& itClient, const std::string& cod, const std::string& nick, const std::string& message);
        void messageServerClientTopic(iteratorClient& itClient, const std::string& cod, const std::string& nick, const std::string& message);
        void messageClientClient(Client *receiver, Client *transmitter, const std::string& messagePartOne, const std::string& messagePartTwo);
        void messageClientJoinChannel(std::vector<Client*> users, Client *transmitter, const std::string& message, const std::string& name);
        void messageClientChannel(std::vector<Client*> users, Client *transmitter, const std::string& messagePartOne, const std::string& messagePartTwo);
        void messageClientQuitChannels(Client *transmitter, const std::string& message);
        void messageServerJoinChannel(std::vector<Client*> users, Client *newClient, const std::string& channelName);
        void messageLog(int id, const std::string& message, const char *buffer);
        void messageOutput(int id, const std::string& message, const char* buffer);
        void messageLog(iteratorClient& itClient, const std::string& message, const char *buffer);
        void messageOutput(iteratorClient& itClient, const std::string& message, const char* buffer);
        void messageBot(iteratorClient& itClient, const std::string& param);

        void createChannel(iteratorClient& itClient, const std::string& name);
        void joinClientChannel(iteratorClient& itClient, const std::string& name);
        void operatorClientChannel(iteratorClient& itClient, const std::string& name);


        Client* getClientByNickname(const std::string& nick);
        Client* getClientByUsername(const std::string& username);
        Channel* getChannelByName(const std::string& name);

        void setFileTransfer(Client* transmitter, Client* receiver, const std::string& filename, std::size_t size);

        const std::string& getPassword();

        ~Server();


};


#endif