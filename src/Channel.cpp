#include "Channel.hpp"
#include "Server.hpp"

Channel::Channel()
{
    
}

Channel::Channel(const std::string& name): _name(name), _topic(""), _flags(0), _password(""), _quantityUsers(0), _limitUsers(100)
{
    
}

void Channel::everyOneMessage(Client *c, const std::string& message, Server& server, int action)
{
    if (action == 0)
    {
        server.messageClientJoinChannel(_users, c, message, _name);
    }
    else if (action == 1)
    {
        server.messageClientChannel(_users, c, message, "");
    }
        
}

void Channel::clientJoinMessage(Client *c, Server& server)
{
    server.messageServerJoinChannel(_users, c, this->getName());
}

void Channel::setUser(Client *c)
{
    if (_users.empty())
    {
        _operators.push_back(c);
    }
    _users.push_back(c);
    _quantityUsers ++;
}

void Channel::setOperator(Client *c)
{
    _operators.push_back(c);
}

void Channel::removeUser(Client *c)
{
    removeOperator(c);
    for (iteratorClientChannel it = _users.begin(); it != _users.end(); ++ it)
    {
        if ((*it) == c)
        {
            _users.erase(it);
            _quantityUsers --;
            break ;
        }
    }
    if (_operators.empty() && !_users.empty())
    {
        this->setOperator(*_users.begin());
    }
}

void Channel::removeOperator(Client *c)
{
    for (iteratorClientChannel it = _operators.begin(); it != _operators.end(); ++ it)
    {
        if ((*it) == c)
        {
            _operators.erase(it);
            return ;
        }
    }
    return ;
}

bool Channel::findUser(Client *c)
{
    for (iteratorClientChannel it = _users.begin(); it != _users.end(); ++ it)
    {
        if (*it == c)
        {
            return (true);
        }
    }
    return (false);
}

bool Channel::findOperator(Client *c)
{
    for (iteratorClientChannel it = _operators.begin(); it != _operators.end(); ++ it)
    {
        if (*it == c)
        {
            return (true);
        }
    }
    return (false);
}

void Channel::setTopic(const std::string& topic)
{
    _topic = topic;
}

void Channel::setFlag(char flag)
{
    _flags = flag;
}

void Channel::setPassword(const std::string& password)
{
    _password = password;
}

void Channel::setQuantityUsers(unsigned int quantityUsers)
{
    _quantityUsers = quantityUsers;
}

void Channel::setLimitUsers(unsigned int limitUsers)
{
    _limitUsers = limitUsers;
}

const std::string& Channel::getName()
{
    return (_name);
}

const std::string& Channel::getTopic()
{
    return (_topic);
}

char  Channel::getFlag()
{
    return (_flags);
}

const std::string& Channel::getPassword()
{
    return (_password);
}

unsigned int Channel::getQuantityUsers()
{
    return (_quantityUsers);
}

unsigned int Channel::getLimitUsers()
{
    return (_limitUsers);
}

Channel::~Channel()
{
   
}

