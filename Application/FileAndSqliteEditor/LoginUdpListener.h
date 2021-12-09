#ifndef _LoginUdpListener_H_
#define _LoginUdpListener_H_
#include <memory>
#include "../../../MyLib/UdpSocket/UdpListener.h"
#include "LoginJsonTalker.h"

class LoginUdpListener : public UdpListener {
public:
    LoginUdpListener(uint16_t portNumber, std::shared_ptr<LoginJsonTalker> _jsonTalker);
    ~LoginUdpListener();

protected:
    void catch_message(std::string& data, const UdpSocket::Address& hisAddress);

private:
    std::shared_ptr<LoginJsonTalker> jsonTalker;
};

#endif