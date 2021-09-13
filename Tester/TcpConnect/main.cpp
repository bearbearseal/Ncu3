#include "../../MyLib/TcpSocket/TcpSocket.h"
#include <chrono>
#include <cstdlib>
#include <thread>

using namespace std;

int main() {
    TcpSocket mySocket;
    mySocket.set_host("127.0.0.1", 10520);
    while(1) {
        int random = rand()%1000;
        random += 200;
        chrono::milliseconds msDelay(random);
        mySocket.open();
        this_thread::sleep_for(500ms);
        //if(mySocket.connected())
        if(mySocket.connection_established()) 
        {
            printf("Connected.\n");
            mySocket.close();
        }
        else {
            printf("Not connected.\n");
        }
        this_thread::sleep_for(msDelay);
    }
}