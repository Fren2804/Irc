#include "Client.hpp"

/*std::cout << "Default constructor of Character" << std::endl;
std::cout << "Constructor of Character with name" << std::endl;
std::cout << "Copy constructor of Character" << std::endl;
std::cout << "Character assignment operator." << std::endl;
std::cout << "Default destructor of Character" << std::endl;*/

Client::Client(int fd, int id): _logged(false), _complete(false), _fd(fd), _id(id), _nickname(""), _username(""), _realName(""), _prefix(""), _buffer(""), _fileMode(false), _bytesPending(0), _receiver(NULL), _filename("")
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

void Client::setFileMode(bool fileMode)
{
    _fileMode = fileMode;
}

void Client::setBytesPending(size_t bytesPending)
{
    _bytesPending = bytesPending;
}

void Client::setReceiver(Client* receiver)
{
    _receiver = receiver;
}

void Client::setFilename(const std::string& filename)
{
    _filename = filename;
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

bool Client::getFileMode()
{
    return (_fileMode);
}

size_t Client::getBytesPending()
{
    return (_bytesPending);
}

Client* Client::getReceiver()
{
    return(_receiver);
}

const std::string& Client::getFilename()
{
    return(_filename);
}

Client::~Client()
{
    
}

