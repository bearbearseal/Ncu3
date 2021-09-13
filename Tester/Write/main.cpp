#include <thread>
#include <vector>
#include "../../../MyLib/ArgumentExtractor/ArgumentExtractor.h"
#include "../../../MyLib/TcpSocket/TcpSocket.h"
#include "../../../OtherLib/nlohmann/json.hpp"
#include "../../../MyLib/File/FileIOer.h"

using namespace std;

int main(int argc, char **argv)
{
    ArgumentExtractor argExtractor(argc, argv);
    if (!argExtractor.has_group("c"))
    {
        printf("No config files.\n");
        return 0;
    }
    auto filename = argExtractor.get_group_argument("c");
    if (filename.empty())
    {
        printf("No filename provided.\n");
        return 0;
    }
    FileIOer aFile;
    if (!aFile.file_exist(filename[0]))
    {
        printf("File does not exist.\n");
        return 0;
    }
    aFile.open(filename[0].c_str());
    string config = aFile.read_data();
    nlohmann::json jConfig;
    try
    {
        jConfig = nlohmann::json::parse(config);
    }
    catch (nlohmann::json::parse_error &error)
    {
        printf("Cannot parse config file.\n%s\n", config.c_str());
        return 0;
    }
    if (!jConfig.is_array())
    {
        printf("Config not array.\n");
        return 0;
    }
    for (size_t i = 0; i < jConfig.size(); ++i)
    {
        if (!jConfig[i].is_object())
        {
            printf("Item %lu is not object.\n", i);
            return 0;
        }
        if (!jConfig[i].contains("Point") || !jConfig[i].contains("Values"))
        {
            printf("Item %lu doesnt contain all needed properties.\n%s\n", i, jConfig[i].dump().c_str());
            return 0;
        }
        if (!jConfig[i]["Point"].is_object() || !jConfig[i]["Values"].is_array())
        {
            printf("Item %lu property has wrong type.\n", i);
            return 0;
        }
        if (!jConfig[i]["Point"].contains("Id"))
        {
            printf("Item %lu point does not contain Id.\n", i);
            return 0;
        }
        if (jConfig[i]["Values"].empty())
        {
            printf("Item %lu values should have at least 1 values.\n%s\n", i, jConfig[i]["Values"].dump().c_str());
            return 0;
        }
    }

    uint8_t state = 0;
    TcpSocket mySocket;
    mySocket.set_host("127.0.0.1", 10520);
    while (true)
    {
        switch (state)
        {
        case 0: //Connect
            mySocket.open();
            do
            {
                this_thread::sleep_for(100ms);
            } while (!mySocket.connection_established());
            state = 1;

        case 1:
            for (size_t i = 0; i < jConfig.size(); ++i)
            {
                for (size_t j = 0; j < jConfig[i]["Values"].size(); ++j)
                {
                    int random = rand() % 2000;
                    random += 200;
                    this_thread::sleep_for(chrono::milliseconds(random));
                    nlohmann::json jWrite;
                    jWrite["Command"] = "Write";
                    jWrite["Branch"] = jConfig[i]["Point"];
                    jWrite["Value"] = jConfig[i]["Values"][j];
                    if (!mySocket.write(jWrite.dump() + "\n"))
                    {
                        mySocket.close();
                        this_thread::sleep_for(100ms);
                        state = 0;
                        break;
                    }
                    else
                    {
                        for (size_t z = 0; z < 10; ++z)
                        {
                            this_thread::sleep_for(20ms);
                            auto reply = mySocket.read(true);
                            if (!reply.second.empty())
                            {
                                try
                                {
                                    nlohmann::json jReply = nlohmann::json::parse(reply.second);
                                    if (jReply.contains("Status"))
                                    {
                                        nlohmann::json &jStatus = jReply["Status"];
                                        string status = jStatus.get<string>();
                                        if (status == "Good")
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            printf("Write status is bad: %s.\n", reply.second.c_str());
                                        }
                                    }
                                    else
                                    {
                                        printf("Write got bad reply: %s.\n", reply.second.c_str());
                                    }
                                }
                                catch (nlohmann::json::parse_error &error)
                                {
                                    printf("Cannot parse Write reply: %s.\n", reply.second.c_str());
                                }
                            }
                        }
                    }
                }
            }
            break;
        }
    }
}