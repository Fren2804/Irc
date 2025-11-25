#include "Parser.hpp"
#include "Utils.h"

#include <stdlib.h>
#include <sstream>

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
        if ((aux[i] < 32 && aux[i] != 1) || aux[i] == 127)
        {
            tokens.clear();
             _server.messageServerNoticeClient(itClient, "Characters not valid in input");
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

    const char* validCommands[] = {"PASS", "NICK", "USER", "JOIN", "PART","PRIVMSG", "QUIT", "KICK", "TOPIC", "MODE", "BOT", NULL};
    void (Parser::*actions[])(Server::iteratorClient&, const std::vector<std::string>&) = {
        &Parser::pass, &Parser::nick,
        &Parser::user, &Parser::join,
        &Parser::part, &Parser::privmsg,
        &Parser::quit, &Parser::kick,
        &Parser::topic, &Parser::mode,
        &Parser::bot};
    
    i = 0;
    if (tokens[0] == "CAP" || tokens[0] == "WHO")
    {
        return ;
    }
    if (tokens.size() < 1)
    {
        _server.messageServerClient(itClient, "421", (*itClient)->getNickname(), "Empty command");
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
        _server.messageServerClient(itClient, "421", (*itClient)->getNickname(), "Unknown command");
    }
}

void Parser::pass(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
    std::string message;
    std::string code;
    bool notice = false;
    
    if ((*itClient)->getLogged() == true)
    {
        message = std::string("You are already registered");
        code = "462";
    }
    else if (tokens.size() < 2)
    {
        message = std::string("Not enough parameters");
        code = "461";
    }
    else if (tokens.size() > 2)
    {
        message = std::string("Too many parameters");
        code = "461";
    }
    else
    {
        if (tokens[1] != _server.getPassword())
        {
            message = std::string("Password incorrect");
            code = "464";
        }
        else
        {
            (*itClient)->setLogged(true);
            message = std::string("Password correct");
            notice = true;
        }
    }
    if (notice)
    {
        _server.messageServerNoticeClient(itClient, message);
    }
    else
    {
        _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
    }
    if (!(*itClient)->getLogged())
    {
        _server.closeConexion((*itClient)->getFd());
    }
}

void Parser::nick(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
    std::string message;
    std::string code;
    bool notice = false;

    if (!(*itClient)->getLogged())
    {
        message = std::string("First you must enter the password");
        code = "451";
    }
    else if ((*itClient)->getNickname().size() > 0)
    {
        message = std::string("Your nick is already set");
        code = "433";
    }
    else if (tokens.size() < 2)
    {
        message = std::string("No nickname given");
        code = "431";
    }
    else if (tokens.size() > 2)
    {
        message = std::string("Too many parameters");
        code = "461";
    }
    else
    {
        Client *c = _server.getClientByNickname(tokens[1]);
        if (c)
        {
            message = std::string("Nickname is already in use ") + c->getNickname();
            code = "433";
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
            notice = true;
        }
    }
    if (notice)
    {
        _server.messageServerNoticeClient(itClient, message);
    }
    else
    {
        _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
    }
    if ((*itClient)->getComplete())
    {
        _server.messageServerClient(itClient, "001", (*itClient)->getNickname(), "Welcome to the IRC server");
    }
}

void Parser::user(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
    std::string message;
    std::string code;
    bool notice = false;

    if (!(*itClient)->getLogged())
    {
        message = std::string("First you must enter the password");
        code = "451";
    }
    else if ((*itClient)->getUsername().size() > 0)
    {
        message = std::string("Your username is already set");
        code = "462";
    }
    else if (tokens.size() < 5)
    {
        message = std::string("Not enough parameters");
        code = "461";
    }
    else if (tokens.size() > 7)
    {
        message = std::string("Too many parameters");
        code = "461";
    }
    else
    {
        if (tokens[4][0] != ':')
        {
            message = std::string("Missing : before username");
            code = "461";
        }
        else if (tokens[4].size() < 2)
        {
            message = std::string("Username cannot be empty");
            code = "461";
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
            notice = true;
        }
    }
    if (notice)
    {
        _server.messageServerNoticeClient(itClient, message);
    }
    else
    {
        _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
    }
    if ((*itClient)->getComplete())
    {
        _server.messageServerClient(itClient, "001", (*itClient)->getNickname(), "Welcome to the IRC server");
    }
}

void Parser::join(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
    std::string message;
    std::string code;
    bool notice = false;

    if (!(*itClient)->getComplete())
    {
        message = std::string("You have not registered");
        code = "451";
    }
    else if (tokens.size() < 2)
    {
        message = std::string("Not enough parameters");
        code = "461";
    }
	else if (tokens[1][0] != '#')
    {
        message = std::string("Channel must start with #");
        code = "479";
    }
    else
    {
        Channel *c = _server.getChannelByName(tokens[1]);
        if (!c && tokens.size() > 2)
        {
            message = std::string("Too many parameters");
			code = "461";
        }
        else if (!c)
        {
            _server.createChannel(itClient, tokens[1]);
            c = _server.getChannelByName(tokens[1]);
            code = "000";
            c->everyOneMessage((*itClient), " JOIN :", _server, 0);
            c->clientJoinMessage(c, (*itClient), _server);
            message = std::string("Channel created");
            notice = true;
        }
		else if (c->getFlag() & FLAG_I)
		{
			message = std::string("Flag i set, you need an invitation for ") + c->getName();
			notice = true;
		}
		else if (tokens.size() > 2 && !(c->getFlag() & FLAG_K))
		{
			message = std::string("Too many parameters");
			code = "461";
		}
		else if (tokens.size() != 3 && (c->getFlag() & FLAG_K))
		{
			message = std::string("Flag k set, need password");
			notice = true;
		}
		else if ((c->getFlag() & FLAG_K) && (c->getPassword() != tokens[2]))
		{
			message = std::string("Incorrect password");
			code = "475";
		}
        else
        {
            if (c->findUser((*itClient)))
            {
                message = "You are already on channel " + c->getName();
                code = "430";
            }
			else if ((c->getFlag() & FLAG_L) && (c->getQuantityUsers() >= c->getLimitUsers()))
			{
				message = std::string("Not enough capacity in the channel, limit of users reached in ") + c->getName();
				code = "471";
			}
            else
            {
                std::string messageEvery = (" JOIN :");
                _server.joinClientChannel(itClient, tokens[1]);
                c->everyOneMessage((*itClient), messageEvery, _server, 0);
                code = "000";
                c->clientJoinMessage(c, (*itClient), _server);
            }
        }
    }
    if (notice)
    {
        _server.messageServerNoticeClient(itClient, message);
    }
    else
    {
        _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
    }
}

void Parser::part(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
    std::string message;
    std::string code;
	bool notice = false;

    if (!(*itClient)->getComplete())
    {
        message = std::string("You have not registered");
        code = "451";
    }
    else if (tokens.size() < 2)
    {
        message = std::string("Not enough parameters");
        code = "461";
    }
    else
    {
        Channel *c = _server.getChannelByName(tokens[1]);
        if (!c)
        {
            message = std::string("This channel doesnt exist ") + tokens[1];
            code = "403";
        }
        else
        {
            if (!c->findUser((*itClient)))
            {
                message = std::string("You are not in the channel ") + tokens[1];
                code = "442";
            }
            else if (tokens.size() > 2 && tokens[2][0] != ':')
            {
                message = std::string("Format message incorrect start with -> :");
                code = "461";
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
                c->removeUser((*itClient), _server);
                message = std::string("Exit from ") + tokens[1] + " sucessfull";
                notice = true;
            }
        }
    }
    if (notice)
    {
        _server.messageServerNoticeClient(itClient, message);
    }
    else
    {
        _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
    }
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
        message = std::string("Not enough parameters");
        code = "461";
    }
    else if (tokens[2][0] != ':')
    {
        message = std::string("Format message incorrect start with -> :");
        code = "412";
    }
    else
    {
        if (tokens[1][0] == '#')
        {
            Channel *ch = _server.getChannelByName(tokens[1]);
            if (!ch)
            {
                message = std::string("This channel doesnt exist ") + tokens[1];
                code = "403";
            }
			else if (!ch->findUser((*itClient)))
			{
				message = std::string("You are not in the channel ") + tokens[1] ;
				code = "442";
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
				ch->everyOneMessage((*itClient), messageEvery, _server, 2);
            }
        }
        else
        {
            Client *cl = _server.getClientByNickname(tokens[1]);
            if (!cl)
            {
                message = std::string("This nickname doesnt exist ") + tokens[1];
                code = "401";
            }
			else if (tokens.size() >= 7 && (tokens[2] == ":DCC" || tokens[2] == ":\001DCC") && tokens[3] == "SEND")
			{
				std::string size = tokens.back();
				if (!size.empty() && size[size.size() - 1] == '\001')
				{
					size = size.substr(0, size.size() - 1);
				}
				std::string messageFile = ":" + (*itClient)->getPrefix() + " PRIVMSG " + tokens[1]
    			+ " :\001DCC SEND " + tokens[4] + " " + tokens[5] + " " + tokens[6] + " " + size + "\001\r\n";
				send(cl->getFd(), messageFile.c_str(), messageFile.size(), 0);
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
    std::string message = "Goodbye";
    std::string code = "000";

    if ((*itClient)->getComplete())
    {
        if (tokens.size() > 1 && tokens[1][0] == ':')
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
        }
    }
    _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
    _server.closeConexionQuit(itClient);
}

void Parser::kick(Server::iteratorClient& itClient, const std::vector<std::string>& tokens)
{
	std::string message;
    std::string code;
	bool notice = false;

    if (!(*itClient)->getComplete())
    {
        message = std::string("You have not registered");
        code = "451";
    }
    else if (tokens.size() < 4)
    {
        message = std::string("Not enough parameters");
        code = "461";
    }
    else
    {
        Channel *c = _server.getChannelByName(tokens[1]);
        if (!c)
        {
            message = std::string("This channel doesnt exist ")  + tokens[1];
            code = "403";
        }
		else if (!c->findUser((*itClient)))
		{
			message = std::string("You are not in the channel ") + tokens[1];
			code = "442";
		}
        else if (!c->findOperator((*itClient)))
        {
			message = std::string("You are not channel operator");
            code = "482";
		}
		else
		{
			Client *cKick = _server.getClientByNickname(tokens[2]);
            if (!cKick)
			{
				message = std::string("No such nick") + tokens[2];
                code = "401";
			}
			else if (cKick == (*itClient))
			{
				message = std::string("You cannot kick yourself, try -> QUIT");
                code = "401";
			}
			else if (!c->findUser(cKick))
			{
				message = std::string("User is not in the channel") + tokens[2];
                code = "441";
			}
            else if (tokens[3][0] != ':')
            {
                message = std::string("Format message incorrect start with -> :");
                code = "461";
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
                //_server.messageClientClient(cKick, (*itClient), messageEvery, "");
				c->everyOneMessage((*itClient), messageEvery, _server, 1);
                c->removeUser(&(*cKick), _server);
				message = "KICK " + tokens[2] + " success";
				notice = true;
            }
        }
    }
    if (notice)
    {
        _server.messageServerNoticeClient(itClient, message);
    }
    else
    {
        _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
    }
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
        message = std::string("Not enough parameters");
        code = "461";
    }
    else
    {
        Channel *c = _server.getChannelByName(tokens[1]);
        if (!c)
        {
            message = std::string("This channel doesnt exist ") + tokens[1];
            code = "403";
        }
		else if (!c->findUser((*itClient)))
		{
			message = std::string("You are not in the channel ") + tokens[1];
			code = "442";
		}
		else if (c->getFlag() & FLAG_T && !c->findOperator(*itClient))
		{
			message = std::string("Flag t set, you cannot change topic because you are not a operator");
			code = "482";
		}
		else
		{
			if (tokens.size() == 2)
			{
				std::string messageTopic;
				std::string codeTopic;
				if (c->getTopic().empty())
				{
					messageTopic = (*itClient)->getNickname() + " " + tokens[1] + " :" + "No topic is set";
					codeTopic = "331";
				}
				else
				{
					messageTopic = (*itClient)->getNickname() + " " + tokens[1] + " " + c->getTopic();
					codeTopic = "332";
				}
				_server.messageServerClientTopic(itClient, codeTopic, (*itClient)->getNickname(), messageTopic);			
			}
			else
			{
				if (tokens[2][0] != ':')
				{
					message = std::string("Format message incorrect start with -> :");
					code = "461";
				}
				else
				{
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
    bool notice = false;

    if (!(*itClient)->getComplete())
    {
        message = std::string("You have not registered");
        code = "451";
    }
    else if (tokens.size() < 2)
    {
        message = std::string("Not enough parameters");
        code = "461";
    }
	else if (tokens.size() > 5)
	{
		message = std::string("Too much arguments");
        code = "461";
	}
    else
    {
        Channel *c = _server.getChannelByName(tokens[1]);
        if (!c)
        {
            message = std::string("This channel doesnt exist ") + tokens[1];
            code = "403";
        }
		else if (!c->findUser((*itClient)))
		{
			message = std::string("You are not in the channel ") + tokens[1];
			code = "442";
		}
		else if (tokens.size() == 2)
		{
			char flags = c->getFlag();
			message = c->getName() + " +";
			if (flags & FLAG_I)
			{
				message += "i";
			}
			if(flags & FLAG_T)
			{
				message += "t";
			}
			if(flags & FLAG_L)
			{
				message += "l";
			}
			if (flags & FLAG_K)
			{
				message += "k " + c->getPassword();
			}
			if (flags & FLAG_L)
			{
                std::stringstream ss;
				ss << c->getLimitUsers();
				std::string limitUsersStr = ss.str();
				message += " " + limitUsersStr;
			}
			code = "324";
			_server.messageServerClientTopic(itClient, code, (*itClient)->getNickname(), message);
			message = "";
			code = "";
		}
        else if (!c->findOperator((*itClient)))
        {
			message = std::string("You are not channel operator");
            code = "482";
		}
		else
		{
			if (tokens[2].size() != 2 || (tokens[2][0] != '+' && tokens[2][0] != '-'))
			{
				message = std::string("Incorrect flag format -> +/-(f) (+o -o)");
				notice = true;
			}
			else if (tokens[2][1] != 'i' && tokens[2][1] != 't' && tokens[2][1] != 'k' && tokens[2][1] != 'o' && tokens[2][1] != 'l')
			{
				message = std::string("Incorrect flag -> i t k o l");
				notice = true;
			}
			else if (tokens[2][1] == 'i')
			{
				if (tokens.size() != 3)
				{
					message = std::string("Flag i doesnt need extra argument");
            		code = "461";
				}
				else
				{
					char flags = c->getFlag();
					if (tokens[2][0] == '+')
					{
						flags |= FLAG_I;
                        //message = std::string("Flag +i set succesfully");
            		    //notice = true;
						message = " MODE " + c->getName() + " +i";
						c->everyOneMessage((*itClient), message, _server, 1);
						message = "";
					}
					else
					{
						flags &= ~FLAG_I;	
                        //message = std::string("Flag -i set succesfully");
            		    //notice = true;
						message = " MODE " + c->getName() + " -i";
						c->everyOneMessage((*itClient), message, _server, 1);
						message = "";					
					}
					c->setFlag(flags);
				}
			}
			else if (tokens[2][1] == 't')
			{
				if (tokens.size() != 3)
				{
					message = std::string("Flag t doesnt need extra argument");
            		code = "461";
				}
				else
				{
					char flags = c->getFlag();
					if (tokens[2][0] == '+')
					{
						flags |= FLAG_T;
                        //message = std::string("Flag +t set succesfully");
            		    //notice = true;
						message = " MODE " + c->getName() + " +t";
						c->everyOneMessage((*itClient), message, _server, 1);
						message = "";
					}
					else
					{
						flags &= ~FLAG_T;
                        //message = std::string("Flag -t set succesfully");
            		    //notice = true;			
						message = " MODE " + c->getName() + " -t";
						c->everyOneMessage((*itClient), message, _server, 1);
						message = "";			
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
            			code = "461";
					}
					else
					{
						flags |= FLAG_K;
						c->setPassword(tokens[3]);
                        //message = std::string("Flag +k and password set succesfully");
            		    //notice = true;
						message = " MODE " + c->getName() + " +k " + tokens[3];
						c->everyOneMessage((*itClient), message, _server, 1);
						message = "";
					}
				}
				else
				{
					if (tokens.size() != 3)
					{
						message = std::string("Flag -k doesnt need extra argument");
            			code = "461";
					}
					else
					{
						flags &= ~FLAG_K;
						c->setPassword("");
                        //message = std::string("Flag -k set succesfully");
            		    //notice = true;
						message = " MODE " + c->getName() + " -k";
						c->everyOneMessage((*itClient), message, _server, 1);
						message = "";
					}			
				}
				c->setFlag(flags);
			}
			else if(tokens[2][1] == 'o')
			{
				if (tokens.size() != 4)
				{
					message = std::string("Flag o need extra argument 'user'");
					code = "461";
				}
				else
				{
					Client *clientOperator = _server.getClientByNickname(tokens[3]);
					if (!c->findUser(clientOperator))
					{
						message = std::string("User is not in the channel ") + tokens[3];
						code = "441";
					}
					else if (tokens[2][0] == '+')
					{
						c->setOperator(clientOperator);
                        //message = std::string("Operator set succesfully");
            		    //notice = true;
						message = " MODE " + c->getName() + " +o " + clientOperator->getNickname();
						c->everyOneMessage((*itClient), message, _server, 1);
						message = "";
					}
					else
					{
						c->removeOperator(clientOperator);
                        //message = std::string("Operator remove succesfully");
            		    //notice = true;
						message = " MODE " + c->getName() + " -o " + clientOperator->getNickname();
						c->everyOneMessage((*itClient), message, _server, 1);
						message = "";
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
            			code = "461";
					}
					else
					{
						unsigned int limitUsers;
						limitUsers = std::atoi(tokens[3].c_str());
						if (limitUsers < 2)
						{
							message = std::string("Limit of users minimum 2");
            				notice = true;
						}
						else
						{
							flags |= FLAG_L;
							c->setLimitUsers(limitUsers);
                            //message = std::string("Flag +l and limit users set succesfully");
            		        //notice = true;
							std::stringstream ss;
							ss << limitUsers;
							std::string limitUsersStr = ss.str();
							message = " MODE " + c->getName() + " +l " + limitUsersStr;
							c->everyOneMessage((*itClient), message, _server, 1);
							message = "";
						}
					}
				}
				else
				{
					if (tokens.size() != 3)
					{
						message = std::string("Flag -l doesnt need extra argument");
            			code = "461";
					}
					else
					{
						flags &= ~FLAG_L;
						c->setLimitUsers(100);
                        //message = std::string("Flag -l set succesfully");
            		    //notice = true;
						message = " MODE " + c->getName() + " -l ";
						c->everyOneMessage((*itClient), message, _server, 1);
						message = "";
					}			
				}
				c->setFlag(flags);
			}
		}
    }
    if (notice)
    {
        _server.messageServerNoticeClient(itClient, message);
    }
    else
    {
        _server.messageServerClient(itClient, code, (*itClient)->getNickname(), message);
    }
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

Parser::~Parser()
{
    
}