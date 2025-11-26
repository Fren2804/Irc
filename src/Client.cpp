#include "Client.hpp"

Client::Client(int fd, int id): _logged(false), _complete(false), _fd(fd), _id(id), _nickname(""), _username(""), _realName(""), _prefix(""), _buffer("")
{
   
}

void Client::buildPrefix()
{
    _prefix = _nickname + "!" + _username + "@host";
}

void Client::setLogged(bool logged)
{
    _logged = logged;
}

void Client::setComplete(bool complete)
{
    _complete = complete;
}

void Client::setFd(int fd)
{
    _fd = fd;
}

void Client::setId(int id)
{
    _id = id;
}

void Client::setNickname(const std::string& nickname)
{
    _nickname = nickname;
}

void Client::setUsername(const std::string& username)
{
    _username = username;
}

void Client::setRealName(const std::string& realName)
{
    _realName = realName;
}

void Client::setPrefix(const std::string& prefix)
{
    _prefix = prefix;
}

void Client::setBuffer(const std::string& buffer)
{
    _buffer = buffer;
}

bool Client::getLogged()
{
    return (_logged);
}

bool Client::getComplete()
{
    return (_complete);
}

int Client::getFd()
{
    return (_fd);
}

int Client::getId()
{
    return (_id);
}

const std::string& Client::getNickname()
{
    return (_nickname);
}

const std::string& Client::getUsername()
{
    return (_username);
}

const std::string& Client::getRealName()
{
    return (_realName);
}

const std::string& Client::getPrefix()
{
    return (_prefix);
}

const std::string& Client::getBuffer()
{
    return (_buffer);
}

Client::~Client()
{
    
}

