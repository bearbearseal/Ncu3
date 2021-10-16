#include "LoginUdpListener.h"

using namespace std;

LoginUdpListener::LoginUdpListener(uint16_t portNumber, std::shared_ptr<LoginJsonTalker> _jsonTalker) : UdpListener(portNumber), jsonTalker(_jsonTalker) {
    
}

LoginUdpListener::~LoginUdpListener() {

}

void LoginUdpListener::catch_message(std::string& input, const UdpSocket::Address& hisAddress) {
    this->write_message(jsonTalker->parse_command(input), hisAddress);
}
