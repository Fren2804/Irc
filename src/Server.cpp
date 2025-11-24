#include "Server.hpp"

#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <poll.h>
#include <iostream>
#include <stdlib.h>

#include <ctime>
#include <iomanip>

#include <stdio.h>

#include <sstream>


#include "Parser.hpp"



Server::Server(): _port(0), _password(""), _parser(NULL)
{
    
}

Server::Server(int port, const std::string &password): _port(port), _password(password), _idClient(1)
{
    _log = new std::ofstream("log.txt", std::ios::app);
    if (!(*_log).is_open())
    {
        perror("log");
        _exit(EXIT_FAILURE);
    }
    Parser *parser = new Parser(*this);
    _parser = parser;
    _jokes.push_back("Why don't scientists trust atoms? Because they make up everything.");
    _jokes.push_back("Why did the scarecrow win an award? He was outstanding in his field.");
    _jokes.push_back("Why don't skeletons fight each other? They don't have the guts.");
    _jokes.push_back("Why did the bicycle fall over? Because it was two-tired.");
    _jokes.push_back("What do you call fake spaghetti? An impasta.");
    _jokes.push_back("Why couldn't the leopard play hide and seek? Because he was always spotted.");
    _jokes.push_back("What do you call cheese that isn't yours? Nacho cheese.");
    _jokes.push_back("Why did the math book look sad? It had too many problems.");
    _jokes.push_back("Why don't eggs tell jokes? They'd crack each other up.");
    _jokes.push_back("What do you call a bear with no teeth? A gummy bear.");
}

void Server::configSocket()
{
   _fdSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_fdSocket < 0)
    {
        perror("socket");
        (*_log).close();
        _exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(_fdSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        (*_log).close();
        close(_fdSocket);
        _exit(EXIT_FAILURE);
    }

    struct sockaddr_in sockaddr_server;
    sockaddr_server.sin_family = AF_INET;
    sockaddr_server.sin_port = htons(_port);
    sockaddr_server.sin_addr.s_addr = INADDR_ANY;

    if (bind(_fdSocket, (struct sockaddr*) &sockaddr_server, sizeof(sockaddr_server)) < 0)
    {
        perror("bind");
        (*_log).close();
        close(_fdSocket);
        _exit(EXIT_FAILURE);
    }

    if (listen(_fdSocket, 10) < 0)
    {
        perror("listen");
        (*_log).close();
        close(_fdSocket);
        _exit(EXIT_FAILURE);
    }

    if (nonBlock(_fdSocket))
    {
        perror("fcntl");
        (*_log).close();
        close(_fdSocket);
        _exit(EXIT_FAILURE);
    }

    struct pollfd pollfdSocket;
    pollfdSocket.fd =_fdSocket;
    pollfdSocket.events = POLLIN;
    pollfdSocket.revents = 0;
    _fds.push_back(pollfdSocket);
}

int Server::nonBlock(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK, 0) < 0)
    {
        return (-1);
    }
    return (0);
}

void Server::running(int *g_running)
{
    while((*g_running))
    {
        poll(&_fds[0], _fds.size(), -1);
    
        for (size_t i = 0; i < _fds.size(); i ++)
        {
            //Esto indica nuevo cliente
            if ((_fds[i].fd == _fdSocket) && (_fds[i].revents & POLLIN))
            {
                newClientConnected();
            }
            //Cliente se va, error etc.
            else if (_fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
            {
                errorClientDisconnected(i);
            }
            //Entrada de mensaje de cliente
            else if (_fds[i].revents & POLLIN)
            {
                waitClient(i);
            }
        }
    }
}


void Server::newClientConnected()
{
    int fdCliente;
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    if ((fdCliente = accept(_fdSocket, (struct sockaddr*)&client_addr, &len)) < 0)
    {
        perror("accept client");
        return ;
    }
    std::string ipClient = inet_ntoa(client_addr.sin_addr);
    messageLog(_idClient, "New client connected: ", (char *) ipClient.c_str());
    if (nonBlock(_fdSocket))
    {
        perror("fcntl client");
        close(fdCliente);
        return ;
    }
    struct pollfd pollfd_new_client;
    pollfd_new_client.fd = fdCliente;
    pollfd_new_client.events = POLLIN | POLLHUP;
    pollfd_new_client.revents = 0;
    _fds.push_back(pollfd_new_client);
    Client *newClient = new Client(fdCliente, _idClient);
    _clients.push_back(newClient);
    messageOutput(newClient->getId(), "Cliente conected", (char *)"");
    _idClient ++;
    return ;
}

void Server::errorClientDisconnected(int i)
{
    iteratorClient it = _searchClientFd(_fds[i].fd);
    messageOutput(it, "Client disconected error/hangup", (char *)"");
    messageLog(it, "Client disconected error/hangup", (char *)"");
    closeConexion(_fds[i].fd);
}

void Server::waitClient(int i)
{
    char buffer[1024];
    ssize_t lenRecv;
    int fdCliente = _fds[i].fd;
    iteratorClient it = _searchClientFd(fdCliente);

    lenRecv = recv(fdCliente, buffer, sizeof(buffer) - 1, 0);
    if (lenRecv <= 0)
    {
        messageOutput(it, "Client disconected", "");
        messageLog(it, "Client disconected", "");
        closeConexion(fdCliente);
        return;
    }

    if ((*it)->getFileMode())
    {
        size_t pending = (*it)->getBytesPending();
        Client *receiver = (*it)->getReceiver();
        size_t bytesToProcess = std::min((size_t)lenRecv, pending);
        send(receiver->getFd(), buffer, bytesToProcess, 0);
        pending -= bytesToProcess;
        (*it)->setBytesPending(pending);
        if (pending == 0)
        {
            (*it)->setFileMode(false);
            messageServerClient(it, "200", (*it)->getNickname(), "File transfer complete");

            std::string messageReceiver;
            messageReceiver = "\n:server " + receiver->getNickname() + " :Send file " + (*it)->getFilename() + "\r\n";
            send(receiver->getFd(), messageReceiver.c_str(), messageReceiver.size(), 0);
        }
        return ;
    }

    std::string bufferClient = (*it)->getBuffer();
    bufferClient.append(buffer, lenRecv);
    size_t pos;
    while ((pos = bufferClient.find('\n')) != std::string::npos)
    {
        std::string line = bufferClient.substr(0, pos);

        if (!line.empty() && line[line.size() - 1] == '\r')
        {
            line.erase(line.size() - 1);
        }
        bufferClient.erase(0, pos + 1);

        messageOutput(it, "Message client:", line.c_str());
        messageLog(it, "Message client:", line.c_str());

        _parser->parser(it, line.c_str());
        it = _searchClientFd(fdCliente);
        if (it == _clients.end())
        {
            return ;
        }
    }
    (*it)->setBuffer(bufferClient);
}


Server::iteratorClient Server::_searchClientFd(int fd)
{
    for (iteratorClient it = _clients.begin(); it != _clients.end(); ++ it)
    {
        if ((*it)->getFd() == fd)
            return (it);
    }
    return (_clients.end());
}

void Server::closeConexion(int fdCliente)
{
    iteratorClient itClient = _searchClientFd(fdCliente);
    close(fdCliente);
    for (iteratorPDF it = _fds.begin(); it != _fds.end(); ++it)
    {
        if (it->fd == fdCliente)
        {
            _fds.erase(it);
            break ;
        }
    }
    for (iteratorChannel it = _channels.begin(); it != _channels.end(); ++ it)
    {
        if (it->second.findUser(*itClient))
        {
            it->second.removeUser(*itClient);
        }
    }
    delete *itClient;
    _clients.erase(itClient);
}

void Server::closeConexionQuit(iteratorClient& itClient)
{
    messageOutput(itClient, "Client disconected", (char *)"");
    messageLog(itClient, "Client disconected", (char *)"");
    closeConexion((*itClient)->getFd());
}

void Server::messageServerClient(iteratorClient& itClient, const std::string& cod, const std::string& nick, const std::string& message)
{
    std::string reply = (":server ");
    if (message.size() < 1)
    {
        return ;
    }
    if (std::atoi(cod.c_str()) > 0)
    {
        reply = reply + cod + " ";
    }
    if (nick.size() > 0)
    {
        reply = reply + nick + " :";
    }
    else
    {
        reply = reply + "* :";
    }
    reply = reply + message + "\r\n";
    send((*itClient)->getFd(), reply.c_str(), reply.size(), 0);
}

void Server::messageServerClientTopic(iteratorClient& itClient, const std::string& cod, const std::string& nick, const std::string& message)
{
    (void) nick;
    std::string reply = (":server ");
    if (message.size() < 1)
    {
        return ;
    }
    if (std::atoi(cod.c_str()) > 0)
    {
        reply = reply + cod + " ";
    }
    reply = reply + message + "\r\n";
    send((*itClient)->getFd(), reply.c_str(), reply.size(), 0);
}

void Server::messageServerJoinChannel(std::vector<Client*> users, Client *newClient, const std::string& channelName)
{
    std::string reply = (":server ");

    reply = reply + "353 " + newClient->getNickname() + " = " + channelName + " :";

    for (int i = users.size() - 1; i >= 0; --i)
    {
        reply = reply + users[i]->getNickname();
        if (i - 1 >= 0)
        {
            reply = reply + " ";
        }
    }
    reply = reply + "\r\n";
    send(newClient->getFd(), reply.c_str(), reply.size(), 0);

    reply = (":server ");
    reply = reply + "366 " + newClient->getNickname() + " " + channelName + " :End of /NAMES list.";
    reply = reply + "\r\n";
    send(newClient->getFd(), reply.c_str(), reply.size(), 0);
}

void Server::messageClientClient(Client *receiver, Client *transmitter, const std::string& messagePartOne, const std::string& messagePartTwo)
{
    std::string reply = (":") + transmitter->getPrefix() + messagePartOne + messagePartTwo;
    reply = reply + "\r\n";
    send(receiver->getFd(), reply.c_str(), reply.size(), 0);
}

void Server::messageClientJoinChannel(std::vector<Client*> users, Client *transmitter, const std::string& message, const std::string& name)
{
    for (Channel::iteratorClientChannel it = users.begin(); it != users.end(); ++ it)
    {
        messageClientClient((*it), transmitter, message, name);
    }
}

void Server::messageClientChannel(std::vector<Client*> users, Client *transmitter, const std::string& messagePartOne, const std::string& messagePartTwo)
{
    for (Channel::iteratorClientChannel it = users.begin(); it != users.end(); ++ it)
    {
        if (transmitter != (*it))
        {
            messageClientClient((*it), transmitter, messagePartOne, messagePartTwo);
        }
    }
}

void Server::messageClientQuitChannels(Client *transmitter, const std::string& message)
{
    for(iteratorChannel it = _channels.begin(); it != _channels.end(); ++ it)
    {
        if(it->second.findUser(transmitter))
        {
            it->second.everyOneMessage(transmitter, message, *this, 1);
            it->second.removeUser(transmitter);
        }
    }
}


void Server::messageLog(int id, const std::string& message, const char* buffer)
{
    std::time_t now = std::time(NULL);
    std::tm* tm_now = std::localtime(&now);
    char timeBuf[32];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_now);

    (*_log) << "[" << timeBuf << "] " << id << " - " << message << buffer << std::endl;
}


void Server::messageOutput(int id, const std::string& message, const char* buffer)
{
    std::time_t now = std::time(NULL);
    std::tm* tm_now = std::localtime(&now);
    char timeBuf[32];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_now);

    std::cout << "[" << timeBuf << "] " << id << " - " << message << buffer << std::endl;
}

void Server::messageLog(iteratorClient &it, const std::string& message, const char* buffer)
{
    std::time_t now = std::time(NULL);
    std::tm* tm_now = std::localtime(&now);
    char timeBuf[32];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_now);
    std::string prefix;

    prefix = (*it)->getPrefix();
    if (prefix.empty())
    {
        std::stringstream ss;
        ss << (*it)->getId();
        prefix = ss.str();
    }

    (*_log) << "[" << timeBuf << "] " << prefix << " - " << message << buffer << std::endl;
}


void Server::messageOutput(iteratorClient &it, const std::string& message, const char* buffer)
{
    std::time_t now = std::time(NULL);
    std::tm* tm_now = std::localtime(&now);
    char timeBuf[32];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_now);
    std::string prefix;

    
    prefix = (*it)->getPrefix();
    if (prefix.empty())
    {
        std::stringstream ss;
        ss << (*it)->getId();
        prefix = ss.str();
    }


    std::cout << "[" << timeBuf << "] " << prefix << " - " << message << buffer << std::endl;
}

void Server::messageBot(iteratorClient& itClient, const std::string &param)
{
    std::string reply;

    if (param == "help")
    {
        reply = "!HELP\nPASS <password>\n\tAuthenticate with the server. Must be used before NICK/USER.\n"
                "\nNICK <nickname>\n\tSet or change your nickname. Must be unique on the server.\n"
                "\nUSER <username> 0 * :<realname>\n\tRegister your username and real name. Required to complete login.\n"
                "\nJOIN <#channel>\n\tJoin or create a channel. You will start receiving messages from it.\n"
                "\nPART <#channel> :<optional!message>\n\tLeave a channel.\n"
                "\nPRIVMSG <target> :<message>\n\tSend a private message to a user or channel.\n"
                "\nQUIT :<optional!message>\n\tDisconnect from the server with an optional message.\n"
                "\nTOPIC <#channel> :<toSet!topic>\n\tSet or view the topic of a channel.\n"
                "\nKICK <#channel> <user> :<reason>\n\tRemove a user from a channel (requires operator privileges).\n"
                "\nMODE <target> <mode>\n\tChange channel or user modes (depends on your implementation).\n"
                "\nBOT <help|users|channels|joke>\n\tInteract with the built-in server bot. Examples:\n\t\tBOT help      → shows bot commands"
                "\n\t\tBOT users     → lists online users\n\t\tBOT channels  → lists available channels\n\t\tBOT joke      → a random joke\n"
                "\nFILE <nick> <filename> <size>\n\tStart a file transfer to another user.\n\tAfter this command, send the raw file bytes.";
    }
    else if (param == "users")
    {
        for (iteratorClient it = _clients.begin(); it != _clients.end(); ++it)
        {
            reply += (*it)->getNickname() + " ";
        }
    }
    else if (param == "channels")
    {
        for (iteratorChannel it = _channels.begin(); it != _channels.end(); ++it)
        {
            reply += it->first + " ";
        }
    }
    else
    {
        srand(time(NULL));
        int joke = rand() % 10;
        reply = _jokes[joke];
    }
    messageServerClient(itClient, "000", (*itClient)->getNickname(), reply);
}


void Server::createChannel(iteratorClient& itClient, const std::string& name)
{
    Channel c = Channel(name);

    c.setUser((*itClient));
    c.setOperator((*itClient));
    _channels.insert(std::make_pair(name, c));
}

void Server::joinClientChannel(iteratorClient& itClient, const std::string& name)
{
    Channel *c = &_channels[name];
    c->setUser((*itClient));
}

void Server::operatorClientChannel(iteratorClient& itClient, const std::string& name)
{
    Channel *c = &_channels[name];
    c->setOperator((*itClient));
}

Client* Server::getClientByNickname(const std::string& nick)
{
    for (iteratorClient it = _clients.begin(); it != _clients.end(); ++ it)
    {
        if ((*it)->getNickname() == nick)
        {
            return (*it);
        }
    }
    return (NULL);
}

Client* Server::getClientByUsername(const std::string& username)
{
    for (iteratorClient it = _clients.begin(); it != _clients.end(); ++ it)
    {
        if ((*it)->getUsername() == username)
        {
            return (*it);
        }
    }
    return (NULL);
}

Channel* Server::getChannelByName(const std::string& name)
{
    iteratorChannel it = _channels.find(name);
    if (it != _channels.end())
    {
        return (&it->second);
    }
    return (NULL);
}

void Server::setFileTransfer(Client* transmitter, Client* receiver, const std::string& filename, std::size_t size)
{
    transmitter->setFileMode(true);
    transmitter->setReceiver(receiver);
    transmitter->setFilename(filename);
    transmitter->setBytesPending(size);
}

const std::string& Server::getPassword()
{
    return (_password);
}

Server::~Server()
{
    for (std::size_t i = 0; i < _fds.size(); ++ i)
    {
        close(_fds[i].fd);
    }
    for (std::size_t i = 0; i < _clients.size(); ++i)
    {
        delete _clients[i];
    }
    _clients.clear();
    delete (_log);
    delete (_parser);
}