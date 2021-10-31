#include "PointWriter.h"
#include "../../../MyLib/File/FileIOer.h"

using namespace std;

int main(int argc, char** argv)
{
    FileIOer file;
    file.open("Config.json");
    string configData = file.read_data();
    PointWriter pointWriter("127.0.0.1", "10520", configData);
    while(1)
    {
        this_thread::sleep_for(1s);
    }
}