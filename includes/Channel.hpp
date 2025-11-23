#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include "Client.hpp"

#define FLAG_I (1 << 0)
#define FLAG_T (1 << 1)
#define FLAG_K (1 << 2)
#define FLAG_L (1 << 3)

//Active flag _flags |= FLAG_I
//Desactive flag _flags &= ~FLAG_I
//Check _flags & FLAG_I

class Server;

class Channel
{
    public:
        typedef std::vector<Client*>::iterator iteratorClientChannel;
    private:
        std::vector<Client*> _users;
        std::vector<Client*> _operators;

        const std::string _name;
        std::string _topic;
        // Bit 0 -> flag i
        // Bit 1 -> flag t
        // Bit 2 -> flag k
        // Bit 3 -> flag l
        //0000
        char _flags;
        std::string _password;
        unsigned int _quantityUsers;
        unsigned int _limitUsers;

    public:
        Channel();
        Channel(const std::string& name);

        void everyOneMessage(Client *c, const std::string& message, Server& server, int action);
        void clientJoinMessage(Client *c, Server& server);

        void clientQuit(Client *c, const std::string& message, Server& server);

        void setUser(Client *c);
        void setOperator(Client *c);

        void removeUser(Client *c);
        void removeOperator(Client *c);

        bool findUser(Client *c);
        bool findOperator(Client *c);

        void setTopic(const std::string& topic);
        void setFlag(char flag);
        void setPassword(const std::string& topic);
        void setQuantityUsers(unsigned int quantityUsers);
        void setLimitUsers(unsigned int limitUsers);

        const std::string& getName();
        const std::string& getTopic();
        char getFlag();
        const std::string& getPassword();
        unsigned int getQuantityUsers();
        unsigned int getLimitUsers();

        ~Channel();
};

#endif