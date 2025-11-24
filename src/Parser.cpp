#include "Parser.hpp"
#include "Utils.h"

#include <stdlib.h>

Parser::Parser(Server &server): _server(server)
{

}

void Parser::parser(Server::iteratorClient& itClient, const char *buffer)
{
    std::string aux = buffer;
    std::vector<std::string> tokens;
    std::size_t i = 0;
    int position = -1;
    while (aux[i] != '\0')
    {
        if (aux[i] < 32 || aux[i] == 127)
        {
            tokens.clear();
             _server.messageServerClient(itClient, "000", (*itClient)->getNickname(), "Characters not valid in input");
            return ;
        }
        if (aux[i] != ' ')
        {
            if (position == -1)
            {
                position = i;
            }
        }
        else
        {
            if (position != -1)
            {
                tokens.push_back(aux.substr(position, i - position));
                position = -1;
            }
        }
        i ++;
    }
    if (position != -1)
    {
        tokens.push_back(aux.substr(position, i - position));
        position = -1;
    }

    /*for (int j = 0; j < tokens.size(); ++ j)
    {
        std::cout << tokens[j] << std::endl;
    }*/

    const char* validCommands[] = {"PASS", "NICK", "USER", "JOIN", "PART","PRIVMSG", "QUIT", "KICK", "TOPIC", "MODE", "BOT", "FILE", NULL};
    void (Parser::*actions[])(Server::iteratorClient&, const std::vector<std::string>&) = {
        &Parser::pass, &Parser::nick,
        &Parser::user, &Parser::join,
        &Parser::part, &Parser::privmsg,
        &Parser::quit, &Parser::kick,
        &Parser::topic, &Parser::mode,
        &Parser::bot, &Parser::file};
    
    i = 0;
    if (tokens.size() < 1)
    {
        //Comprobar nick o username
        /*std::string reply = std::string(":server * :empty command") + "\r\n";
        send(itClient->getFd(), reply.c_str(), reply.size(), 0);*/
        _server.messageServerClient(itClient, "000", (*itClient)->getNickname(), "Empty command");
        return ;
    }
    while (validCommands[i])
    {
        if (validCommands[i] == tokens[0])
        {
            (this->*actions[i])(itClient, tokens);
            return ;
        }
        i ++;
    }
    if (!validCommands[i])
    {
        _server.messageServerClient(itClient, "000", (*itClient)->getNickname(), "Command not found");
    }
}

void Parser::pass(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
    std::string message;
    std::string code;
    
    if ((*itClient)->getLogged() == true)
    {
        message = std::string("You are already logged");
        code = "000";
    }
    else if (tokens.size() < 2)
    {
        message = std::string("Need a password");
        code = "000";
    }
    else if (tokens.size() > 2)
    {
        message = std::string("Cannot have more than one argument");
        code = "000";
    }
    else
    {
        if (tokens[1] != _server.getPassword())
        {
            message = std::string("Incorrect password");
            code = "000";
        }
        else
        {
            (*itClient)->setLogged(true);
            message = std::string("Password correct");
            code = "000";
        }
    }
    _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
    if (!(*itClient)->getLogged())
    {
        _server.closeConexion((*itClient)->getFd());
    }
}

void Parser::nick(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
    std::string message;
    std::string code;

    if (!(*itClient)->getLogged())
    {
        message = std::string("First introduce password");
        code = "000";
    }
    else if ((*itClient)->getNickname().size() > 0)
    {
        message = std::string("Your nick is already set");
        code = "000";
    }
    else if (tokens.size() < 2)
    {
        message = std::string("Need a nick");
        code = "000";
    }
    else if (tokens.size() > 2)
    {
        message = std::string("Cannot have more than one argument");
        code = "000";
    }
    else
    {
        Client *c = _server.getClientByNickname(tokens[1]);
        if (c)
        {
            message = std::string("Nickname already in use ") + c->getNickname();
            code = "000";
        }
        else
        {
            (*itClient)->setNickname(tokens[1]);
            if ((*itClient)->getUsername().size() > 0)
            {
                (*itClient)->buildPrefix();
                (*itClient)->setComplete(true);
            }
            message = std::string("Nickname set sucessfully");
            code = "000";
        }
    }
    _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
    if ((*itClient)->getComplete())
    {
        _server.messageServerClient(itClient, "001", (*itClient)->getNickname(), "Welcome to the IRC server");
    }
}
void Parser::user(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
    std::string message;
    std::string code;

    if (!(*itClient)->getLogged())
    {
        message = std::string("First introduce password");
        code = "000";
    }
    else if ((*itClient)->getUsername().size() > 0)
    {
        message = std::string("Your username is already set");
    }
    else if (tokens.size() < 5)
    {
        message = std::string("Need all parameters");
        code = "000";
    }
    else if (tokens.size() > 7)
    {
        message = std::string("Cannot have more than 7 arguments");
        code = "000";
    }
    else
    {
        if (tokens[4][0] != ':')
        {
            message = std::string("Missing : before username");
            code = "000";
        }
        else if (tokens[4].size() < 2)
        {
            message = std::string("Username cannot be empty");
            code = "000";
        }
        else
        {
            std::string realName = "";
            for (size_t i = 1; i < tokens[4].size(); i ++)
            {
                realName = realName + tokens[4][i];
            }
            for (size_t i = 5; i < tokens.size(); ++ i)
            {
                realName = realName + " ";
                realName = realName + tokens[i];
            }
            (*itClient)->setUsername(tokens[1]);
            (*itClient)->setRealName(realName);
            if ((*itClient)->getNickname().size() > 0)
            {
                (*itClient)->buildPrefix();
                (*itClient)->setComplete(true);
            }
            message = std::string("Username set sucessfully");
            code = "000";
        }
    }
    _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
    if ((*itClient)->getComplete())
    {
        _server.messageServerClient(itClient, "001", (*itClient)->getNickname(), "Welcome to the IRC server");
    }
}
void Parser::join(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
    std::string message;
    std::string code;

    if (!(*itClient)->getComplete())
    {
        message = std::string("You have not registered");
        code = "451";
    }
    else if (tokens.size() < 2)
    {
        message = std::string("Need all parameters");
        code = "000";
    }
	else if (tokens[1][0] != '#')
    {
        message = std::string("Channel must start with #");
        code = "000";
    }
    else
    {
        Channel *c = _server.getChannelByName(tokens[1]);
        if (!c)
        {
            _server.createChannel(itClient, tokens[1]);
            message = std::string("Channel ") + tokens[1] + " created";
            code = "000";
        }
		else if (c->getFlag() & FLAG_I)
		{
			message = std::string("Flag i set, you need an invitation for ") + c->getName();
			code = "000";
		}
		else if (tokens.size() > 2 && !(c->getFlag() & FLAG_K))
		{
			message = std::string("Cannot have more than 1 argument");
			code = "000";
		}
		else if (tokens.size() != 3 && (c->getFlag() & FLAG_K))
		{
			message = std::string("Flag k set, need password");
			code = "000";
		}
		else if ((c->getFlag() & FLAG_K) && (c->getPassword() != tokens[2]))
		{
            std::cout << c->getPassword() << "|" << tokens[2] << std::endl;
			message = std::string("Incorrect password");
			code = "000";
		}
        else
        {
            if (c->findUser((*itClient)))
            {
                message = "You are already joined in channel " + c->getName();
                code = "000";
            }
			else if ((c->getFlag() & FLAG_L) && (c->getQuantityUsers() >= c->getLimitUsers()))
			{
				message = std::string("Not enough capacity in the channel, limit of users reached in ") + c->getName();
				code = "000";
			}
            else
            {
                std::string messageEvery = (" JOIN :");
                c->everyOneMessage((*itClient), messageEvery, _server, 0);
                _server.joinClientChannel(itClient, tokens[1]);
                message = std::string("Joined in ") + tokens[1] + " sucessfull";
                code = "000";
                c->clientJoinMessage((*itClient), _server);
            }
        }
    }
    _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
}
void Parser::part(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
    std::string message;
    std::string code;

    if (!(*itClient)->getComplete())
    {
        message = std::string("You have not registered");
        code = "451";
    }
    else if (tokens.size() < 2)
    {
        message = std::string("Need all parameters");
        code = "000";
    }
    else
    {
        Channel *c = _server.getChannelByName(tokens[1]);
        if (!c)
        {
            message = std::string("This channel doesnt exist ") + tokens[1];
            code = "000";
        }
        else
        {
            if (!c->findUser((*itClient)))
            {
                message = std::string("You are not in the channel ") + tokens[1];
                code = "000";
            }
            else if (tokens.size() > 2 && tokens[2][0] != ':')
            {
                message = std::string("Format message incorrect start with -> :");
                code = "000";
            }
            else
            {
                std::string messageEvery = (" PART ") + tokens[1];
                if (tokens.size() > 2)
                {
                    messageEvery = messageEvery + " ";
                    for (std::size_t i = 2; i < tokens.size(); i ++)
                    {
                        messageEvery = messageEvery + tokens[i];
                        if (i + 1 < tokens.size())
                        {
                            messageEvery = messageEvery + " ";
                        }
                    }
                }
                c->everyOneMessage((*itClient), messageEvery, _server, 1);
                c->removeUser((*itClient));
                message = std::string("Exit from ") + tokens[1] + " sucessfull";
                code = "000";
            }
        }
    }
    _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
}
void Parser::privmsg(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
    std::string message;
    std::string code;

    if (!(*itClient)->getComplete())
    {
        message = std::string("You have not registered");
        code = "451";
    }
    else if (tokens.size() < 3)
    {
        message = std::string("Need all parameters");
        code = "000";
    }
    else if (tokens[2][0] != ':')
    {
        message = std::string("Format message incorrect start with -> :");
        code = "000";
    }
    else
    {
        if (tokens[1][0] == '#')
        {
            Channel *ch = _server.getChannelByName(tokens[1]);
            if (!ch)
            {
                message = std::string("This channel doesnt exist ") + tokens[1];
                code = "000";
            }
			else if (!ch->findUser((*itClient)))
			{
				message = std::string("You are not in this channel ") + tokens[1] ;
				code = "000";
			}
            else
            {
				std::string messageEvery = (" PRIVMSG ") + tokens[1];
				messageEvery = messageEvery + " ";
				for (std::size_t i = 2; i < tokens.size(); i ++)
				{
					messageEvery = messageEvery + tokens[i];
					if (i + 1 < tokens.size())
					{
						messageEvery = messageEvery + " ";
					}
				}
				ch->everyOneMessage((*itClient), messageEvery, _server, 1);
            }
        }
        else
        {
            Client *cl = _server.getClientByNickname(tokens[1]);
            if (!cl)
            {
                message = std::string("This nickname doesnt exist ") + tokens[1];
                code = "000";
            }
            else
            {
                std::string messageUser = (" PRIVMSG ") + tokens[1];
                messageUser = messageUser + " ";
                for (std::size_t i = 2; i < tokens.size(); i ++)
                {
                    messageUser = messageUser + tokens[i];
                    if (i + 1 < tokens.size())
                    {
                        messageUser = messageUser + " ";
                    }
                }
                _server.messageClientClient(cl, (*itClient), messageUser, "");
            }
        }
    }
    _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
}
void Parser::quit(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
    std::string message;
    std::string code;

    if (!(*itClient)->getComplete())
    {
        message = std::string("Goodbye");
        code = "000";
    }
    else
    {
        if (tokens.size() > 1 && tokens[1][0] != ':')
        {
            message = std::string("Format message incorrect start with -> :");
            code = "000";
        }
        else
        {
            std::string messageUser = (" QUIT");
            if (tokens.size() > 1)
            {
				messageUser = messageUser + " ";
				for (std::size_t i = 1; i < tokens.size(); i ++)
				{
					messageUser = messageUser + tokens[i];
					if (i + 1 < tokens.size())
					{
						messageUser = messageUser + " ";
					}
				}
            }
            _server.messageClientQuitChannels((*itClient), messageUser);
			message = std::string("Goodbye");
        	code = "000";
        }
    }
    _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
    _server.closeConexionQuit(itClient);
}

void Parser::kick(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
	std::string message;
    std::string code;

    if (!(*itClient)->getComplete())
    {
        message = std::string("You have not registered");
        code = "451";
    }
    else if (tokens.size() < 4)
    {
        message = std::string("Need all parameters");
        code = "000";
    }
    else
    {
        Channel *c = _server.getChannelByName(tokens[1]);
        if (!c)
        {
            message = std::string("This channel doesnt exist ")  + tokens[1];
            code = "000";
        }
		else if (!c->findUser((*itClient)))
		{
			message = std::string("You are not in the channel ") + tokens[1];
			code = "000";
		}
        else if (!c->findOperator((*itClient)))
        {
			message = std::string("You dont have permissions of operator");
            code = "000";
		}
		else
		{
			Client *cKick = _server.getClientByNickname(tokens[2]);
			if (cKick == (*itClient))
			{
				message = std::string("You cannot kick yourself, try -> QUIT");
                code = "000";
			}
			else if (!c->findUser(cKick))
			{
				message = std::string("User is not in the channel") + tokens[2];
                code = "000";
			}
            else if (tokens[3][0] != ':')
            {
                message = std::string("Format message incorrect start with -> :");
                code = "000";
            }
            else
            {
                std::string messageEvery = (" KICK ") + tokens[1] + " " + tokens[2] + " ";
				for (std::size_t i = 3; i < tokens.size(); i ++)
				{
					messageEvery = messageEvery + tokens[i];
					if (i + 1 < tokens.size())
					{
						messageEvery = messageEvery + " ";
					}
				}
                _server.messageClientClient(cKick, (*itClient), messageEvery, "");
                c->removeUser(&(*cKick));
            }
        }
    }
    _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
}

void Parser::topic(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
	std::string message;
    std::string code;

    if (!(*itClient)->getComplete())
    {
        message = std::string("You have not registered");
        code = "451";
    }
    else if (tokens.size() < 2)
    {
        message = std::string("Need all parameters");
        code = "000";
    }
    else
    {
        Channel *c = _server.getChannelByName(tokens[1]);
        if (!c)
        {
            message = std::string("This channel doesnt exist ") + tokens[1];
            code = "000";
        }
		else if (!c->findUser((*itClient)))
		{
			message = std::string("You are not in the channel ") + tokens[1];
			code = "000";
		}
		else if (c->getFlag() & FLAG_T && !c->findOperator(*itClient))
		{
			message = std::string("Flag t set, you cannot change topic because you are not a operator");
			code = "000";
		}
		else
		{
			if (tokens.size() == 2)
			{
				std::string messageTopic;
				std::string codeTopic;
				if (c->getTopic().empty())
				{
					messageTopic = tokens[1] + " :" + "No topic is set";
					codeTopic = "331";
				}
				else
				{
					messageTopic = tokens[1] + " " + c->getTopic();
					codeTopic = "332";
				}
				_server.messageServerClientTopic(itClient, codeTopic, (*itClient)->getNickname(), messageTopic);			
			}
			else
			{
				if (tokens[2][0] != ':')
				{
					message = std::string("Format message incorrect start with -> :");
					code = "000";
				}
				else
				{
					//Flags canal comprobar t
					
					std::string topic;
					std::string messageEvery = (" TOPIC ") + tokens[1] + " ";
					for (std::size_t i = 2; i < tokens.size(); i ++)
					{
						topic = topic + tokens[i];
						messageEvery = messageEvery + tokens[i];
						if (i + 1 < tokens.size())
						{
							messageEvery = messageEvery + " ";
							topic = topic + " ";
						}
					}
					c->everyOneMessage((*itClient), messageEvery, _server, 1);
					c->setTopic(topic);
				}
			}
        }
    }
    _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
}

void Parser::mode(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
	std::string message;
    std::string code;

    if (!(*itClient)->getComplete())
    {
        message = std::string("You have not registered");
        code = "451";
    }
    else if (tokens.size() < 3)
    {
        message = std::string("Need all parameters");
        code = "000";
    }
	else if (tokens.size() > 5)
	{
		message = std::string("Too much arguments");
        code = "000";
	}
	else if (tokens[2].size() != 2 || (tokens[2][0] != '+' && tokens[2][0] != '-'))
	{
		message = std::string("Incorrect flag format -> +/-(f) (+o -o)");
		code = "000";
	}
	else if (tokens[2][1] != 'i' && tokens[2][1] != 't' && tokens[2][1] != 'k' && tokens[2][1] != 'o' && tokens[2][1] != 'l')
	{
		message = std::string("Incorrect flag -> i t k o l");
		code = "000";
	}
    else
    {
        Channel *c = _server.getChannelByName(tokens[1]);
        if (!c)
        {
            message = std::string("This channel doesnt exist ") + tokens[1];
            code = "000";
        }
		else if (!c->findUser((*itClient)))
		{
			message = std::string("You are not in the channel ") + tokens[1];
			code = "000";
		}
        else if (!c->findOperator((*itClient)))
        {
			message = std::string("You dont have permissions of operator");
            code = "000";
		}
		else
		{
			if (tokens[2][1] == 'i')
			{
				if (tokens.size() != 3)
				{
					message = std::string("Flag i doesnt need extra argument");
            		code = "000";
				}
				else
				{
					char flags = c->getFlag();
					if (tokens[2][0] == '+')
					{
						flags |= FLAG_I;
                        message = std::string("Flag +i set succesfully");
            		    code = "000";
					}
					else
					{
						flags &= ~FLAG_I;	
                        message = std::string("Flag -i set succesfully");
            		    code = "000";					
					}
					c->setFlag(flags);
				}
			}
			else if (tokens[2][1] == 't')
			{
				if (tokens.size() != 3)
				{
					message = std::string("Flag t doesnt need extra argument");
            		code = "000";
				}
				else
				{
					char flags = c->getFlag();
					if (tokens[2][0] == '+')
					{
						flags |= FLAG_T;
                        message = std::string("Flag +t set succesfully");
            		    code = "000";
					}
					else
					{
						flags &= ~FLAG_T;
                        message = std::string("Flag -t set succesfully");
            		    code = "000";						
					}
					c->setFlag(flags);
				}
			}
			else if (tokens[2][1] == 'k')
			{
				char flags = c->getFlag();
				if (tokens[2][0] == '+')
				{
					if (tokens.size() != 4)
					{
						message = std::string("Flag +k need extra argument 'password'");
            			code = "000";
					}
					else
					{
						flags |= FLAG_K;
						c->setPassword(tokens[3]);
                        message = std::string("Flag +k and password set succesfully");
            		    code = "000";
					}
				}
				else
				{
					if (tokens.size() != 3)
					{
						message = std::string("Flag -k doesnt need extra argument");
            			code = "000";
					}
					else
					{
						flags &= ~FLAG_K;
						c->setPassword("");
                        message = std::string("Flag -k set succesfully");
            		    code = "000";
					}			
				}
				c->setFlag(flags);
			}
			else if(tokens[2][0] == 'o')
			{
				if (tokens.size() != 4)
				{
					message = std::string("Flag o need extra argument 'user'");
					code = "000";
				}
				else
				{
					Client *clientOperator = _server.getClientByNickname(tokens[3]);
					if (!c->findUser(clientOperator))
					{
						message = std::string("User is not in the channel ") + tokens[3];
						code = "000";
					}
					else if (tokens[2][1] == '+')
					{
						c->setOperator(clientOperator);
                        message = std::string("Operator set succesfully");
            		    code = "000";
					}
					else
					{
						c->removeOperator(clientOperator);
                        message = std::string("Operator remove succesfully");
            		    code = "000";
					}
				}
			}
			else
			{
				char flags = c->getFlag();
				if (tokens[2][0] == '+')
				{
					if (tokens.size() != 4)
					{
						message = std::string("Flag +l need extra argument 'limitUsers'");
            			code = "000";
					}
					else
					{
						unsigned int limitUsers;
						limitUsers = std::atoi(tokens[3].c_str());
						if (limitUsers < 2)
						{
							message = std::string("Limit of users minimum 2");
            				code = "000";
						}
						else
						{
							flags |= FLAG_L;
							c->setLimitUsers(limitUsers);
                            message = std::string("Flag +l and limit users set succesfully");
            		        code = "000";
						}
					}
				}
				else
				{
					if (tokens.size() != 3)
					{
						message = std::string("Flag -l doesnt need extra argument");
            			code = "000";
					}
					else
					{
						flags &= ~FLAG_L;
						c->setLimitUsers(100);
                        message = std::string("Flag -l set succesfully");
            		    code = "000";
					}			
				}
				c->setFlag(flags);
			}
		}
    }
    _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
}

void Parser::bot(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
    std::string message;
    std::string code;

    if (!(*itClient)->getComplete())
    {
        message = "You have not registered";
        code = "451";
    }
    else if (tokens.size() != 2)
    {
        message = "Must be one parameter help|users|channels|joke";
        code = "461";
    }
    else if (tokens[1] != "users" && tokens[1] != "channels" && tokens[1] != "help" && tokens[1] != "joke")
    {
        message = "Parameter must be help|users|channels|joke";
        code = "461";
    }
    else
    {
        _server.messageBot(itClient, tokens[1]);
    }

    _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
}

void Parser::file(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
    std::string message;
    std::string code;

    if (!(*itClient)->getComplete())
    {
        message = std::string("You have not registered");
        code = "451";
    }
    else if (tokens.size() != 4)
    {
        message = std::string("Not enough parameters");
        code = "000";
    }
    else if ((*itClient)->getFileMode())
    {
        message = std::string("You are already sending a file");
        code = "000";
    }
    else
    {
        Client *c = _server.getClientByNickname(tokens[1]);
        if (!c)
        {
            message = std::string("User is not in the channel ") + tokens[1];
            code = "000";
        }
        else if (!allDigit(tokens[3]))
        {
            message = std::string("Size must be a number") + tokens[1];
            code = "000";
        }
        else
        {
            message = std::string("Start sending file") + tokens[1];
            code = "000";
            _server.setFileTransfer((*itClient), c, tokens[2], static_cast<size_t>(std::atoi(tokens[3].c_str())));
        }
    }
    _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);

}

Parser::~Parser()
{
    
}