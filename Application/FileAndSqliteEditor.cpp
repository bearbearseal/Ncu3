#include "../../MyLib/Sqlite/Sqlite3UdpListener.h"
#include "../../MyLib/File/UdpFileBrowser.h"
#include "LoginJsonTalker.h"
#include "LoginUdpListener.h"
#include <thread>
#include <memory>

using namespace std;

int main()
{
    Sqlite3UdpListener sqlite3UdpListener(10520, make_shared<Sqlite3JsonTalker>("/var/sqlite/NcuConfig.db"));
    UdpFileBrowser udpFileBrowser(10521, "/var/InOutOpt");
    LoginUdpListener loginUdpListener(10522, make_shared<LoginJsonTalker>("/var/sqlite/UserAccount.db"));
    loginUdpListener.start();
    sqlite3UdpListener.start();
    udpFileBrowser.start();
    while (1)
    {
        this_thread::sleep_for(1s);
    }
}