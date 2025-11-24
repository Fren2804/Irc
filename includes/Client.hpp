#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

class Client
{
    private:
        bool _logged;
        bool _complete;
        int _fd;
        int _id;
        std::string _nickname;
        std::string _username;
        std::string _realName;
        std::string _prefix;

        std::string _buffer;

        bool        _fileMode;
        size_t      _bytesPending;
        Client* _receiver;
        std::string _filename;

    public:
        Client(int fd, int id);
        ~Client();

        void buildPrefix();

        void setLogged(bool logged);
        void setComplete(bool complete);
        void setFd(int fd);
        void setId(int id);
        void setNickname(const std::string& nickname);
        void setUsername(const std::string& username);
        void setRealName(const std::string& realName);
        void setPrefix(const std::string& prefix);
        void setBuffer(const std::string& buffer);
        void setFileMode(bool fileMode);
        void setBytesPending(size_t bytesPending);
        void setReceiver(Client* receiver);
        void setFilename(const std::string& filename);

        bool getLogged();
        bool getComplete();
        int getFd();
        int getId();
        const std::string& getNickname();
        const std::string& getUsername();
        const std::string& getRealName();
        const std::string& getPrefix();
        const std::string& getBuffer();
        bool getFileMode();
        size_t getBytesPending();
        Client* getReceiver();
        const std::string& getFilename();
};

#endif