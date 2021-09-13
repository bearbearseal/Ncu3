#include "../../../MyLib/TcpSocket/TcpSocket.h"
#include "../../../OtherLib/nlohmann/json.hpp"
#include "../../../MyLib/File/FileIOer.h"
#include "../../../MyLib/ArgumentExtractor/ArgumentExtractor.h"
#include <chrono>
#include <thread>

using namespace std;

int main(int argc, char **argv)
{
    ArgumentExtractor argExtractor(argc, argv);
    if (!argExtractor.has_group(""))
    {
        printf("No config files.\n");
        return 0;
    }
    auto filename = argExtractor.get_group_argument("");
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
        if (!jConfig[i].contains("Delay") || !jConfig[i].contains("Point") || !jConfig[i].contains("Value"))
        {
            printf("Item %lu doesnt contain all needed properties.\n%s\n", i, jConfig[i].dump().c_str());
            return 0;
        }
        if (!jConfig[i]["Point"].is_object() || jConfig[i]["Value"].is_array() || jConfig[i]["Value"].is_object())
        {
            printf("Item %lu property has wrong type.\n", i);
            return 0;
        }
        if (!jConfig[i]["Delay"].is_number_unsigned())
        {
            printf("Item %lu Delay should be unsigned integer.\n%s\n", i, jConfig[i]["Delay"].dump().c_str());
            return 0;
        }
        if (!jConfig[i]["Point"].contains("Id"))
        {
            printf("Item %lu point does not contain Id.\n", i);
            return 0;
        }
    }

    uint8_t state = 0;
    TcpSocket mySocket;
    mySocket.set_host("127.0.0.1", 10520);
    nlohmann::json jWrite;
    jWrite["Command"] = "Write";
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
                printf("Going to delay %lu seconds.\n", jConfig[i]["Delay"].get<uint64_t>());
                this_thread::sleep_for(chrono::seconds(jConfig[i]["Delay"].get<uint64_t>()));
                jWrite["Branch"] = jConfig[i]["Point"];
                jWrite["Value"] = jConfig[i]["Value"];
                printf("Writing: %s\n", jWrite.dump().c_str());
                if (!mySocket.write(jWrite.dump() + "\n"))
                {
                    mySocket.close();
                    this_thread::sleep_for(100ms);
                    state = 0;
                    break;
                }
                else
                {
                    for (size_t j = 0; j < 10; ++j)
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
            break;
        }
    }
}
