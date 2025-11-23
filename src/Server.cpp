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
}

void Server::configSocket()
{
   _fd_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd_socket < 0)
    {
        perror("socket");
        (*_log).close();
        _exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(_fd_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        (*_log).close();
        close(_fd_socket);
        _exit(EXIT_FAILURE);
    }

    struct sockaddr_in sockaddr_server;
    sockaddr_server.sin_family = AF_INET;
    sockaddr_server.sin_port = htons(_port);
    sockaddr_server.sin_addr.s_addr = INADDR_ANY;

    if (bind(_fd_socket, (struct sockaddr*) &sockaddr_server, sizeof(sockaddr_server)) < 0)
    {
        perror("bind");
        (*_log).close();
        close(_fd_socket);
        _exit(EXIT_FAILURE);
    }

    if (listen(_fd_socket, 10) < 0)
    {
        perror("listen");
        (*_log).close();
        close(_fd_socket);
        _exit(EXIT_FAILURE);
    }

    if (nonBlock(_fd_socket))
    {
        perror("fcntl");
        (*_log).close();
        close(_fd_socket);
        _exit(EXIT_FAILURE);
    }

    struct pollfd pollfd_socket;
    pollfd_socket.fd =_fd_socket;
    pollfd_socket.events = POLLIN;
    pollfd_socket.revents = 0;
    _fds.push_back(pollfd_socket);
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
            if ((_fds[i].fd == _fd_socket) && (_fds[i].revents & POLLIN))
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
    int fd_client;
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    if ((fd_client = accept(_fd_socket, (struct sockaddr*)&client_addr, &len)) < 0)
    {
        perror("accept client");
        return ;
    }
    std::string ipClient = inet_ntoa(client_addr.sin_addr);
    messageLog(_idClient, "New client connected: ", (char *) ipClient.c_str());
    if (nonBlock(_fd_socket))
    {
        perror("fcntl client");
        close(fd_client);
        return ;
    }
    struct pollfd pollfd_new_client;
    pollfd_new_client.fd = fd_client;
    pollfd_new_client.events = POLLIN | POLLHUP;
    pollfd_new_client.revents = 0;
    _fds.push_back(pollfd_new_client);
    Client *newClient = new Client(fd_client, _idClient);
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
    ssize_t len_recv;
    int fd_client;
    fd_client = _fds[i].fd;
    iteratorClient it = _searchClientFd(fd_client);
    len_recv = recv(fd_client, buffer, sizeof(buffer) - 1, 0);
    if (len_recv <= 0)
    {
        messageOutput(it, "Client disconected", (char *)"");
        messageLog(it, "Client disconected", (char *)"");
        closeConexion(fd_client);
        return ;
    }
    //Temporal suyo simplemente buffer[len_recv] = '\0';
    //std::cout << len_recv << std::endl;
    buffer[len_recv] = '\0';
    if (len_recv < 2)
    {
        if (buffer[len_recv - 1] == '\n')
        {
            buffer[len_recv - 1] = '\0';
        }
    }
    else
    {
        if (buffer[len_recv - 2] == '\r')
        {
            (*it)->setConnection(true);
            buffer[len_recv - 1] = '\0';
        }
        else if (buffer[len_recv - 1] == '\n')
        {
            buffer[len_recv - 1] = '\0';
        }
    }

    //Temporal
    /*for (ssize_t i = 0; buffer[i] != '\0'; i ++)
    {
        printf("%d\n", buffer[i]);
    }*/

    messageOutput(it, "Message client: ", buffer);
    messageLog(it, "Message client: ", buffer);

    _parser->parser(it, buffer);

    //Temporal
    //Temporal std::string reply = std::string("Servidor recibió: ") + buffer + "\r\n"; quitar \n
    //std::string reply = std::string("Servidor recibió: ") + buffer + "\r\n";
    //send(_fds[i].fd, reply.c_str(), reply.size(), 0);
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

void Server::closeConexion(int fd_client)
{
    iteratorClient itClient = _searchClientFd(fd_client);
    close(fd_client);
    for (iteratorPDF it = _fds.begin(); it != _fds.end(); ++it)
    {
        if (it->fd == fd_client)
        {
            _fds.erase(it);
            break ;
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
    if ((*itClient)->getConnection())
    {
        reply = reply + message + "\r\n";
    }
    else
    {
        reply = reply + message + "\n";
    }
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
    if ((*itClient)->getConnection())
    {
        reply = reply + message + "\r\n";
    }
    else
    {
        reply = reply + message + "\n";
    }
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
    if (newClient->getConnection())
    {
        reply = reply + "\r\n";
    }
    else
    {
        reply = reply + "\n";
    }
    send(newClient->getFd(), reply.c_str(), reply.size(), 0);

    reply = (":server ");
    reply = reply + "366 " + newClient->getNickname() + " " + channelName + " :End of /NAMES list.";
    if (newClient->getConnection())
    {
        reply = reply + "\r\n";
    }
    else
    {
        reply = reply + "\n";
    }
    send(newClient->getFd(), reply.c_str(), reply.size(), 0);
}

void Server::messageClientClient(Client *receiver, Client *transmitter, const std::string& messagePartOne, const std::string& messagePartTwo)
{
    std::string reply = (":") + transmitter->getPrefix() + messagePartOne + messagePartTwo;
    if (receiver->getConnection())
    {
        reply = reply + "\r\n";
    }
    else
    {
        reply = reply + "\n";
    }
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


void Server::messageLog(int id, std::string message, char* buffer)
{
    std::time_t now = std::time(NULL);
    std::tm* tm_now = std::localtime(&now);
    char timeBuf[32];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_now);

    (*_log) << "[" << timeBuf << "] " << id << " - " << message << buffer << std::endl;
}


void Server::messageOutput(int id, std::string message, char* buffer)
{
    std::time_t now = std::time(NULL);
    std::tm* tm_now = std::localtime(&now);
    char timeBuf[32];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_now);

    std::cout << "[" << timeBuf << "] " << id << " - " << message << buffer << std::endl;
}

void Server::messageLog(iteratorClient &it, std::string message, char* buffer)
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


void Server::messageOutput(iteratorClient &it, std::string message, char* buffer)
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