#include <thread>
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
    if(!jConfig.is_array()) {
        printf("Config not array.\n");
        return 0;
    }
    for(size_t i=0; i<jConfig.size(); ++i) {
        if (!jConfig[i].is_object())
        {
            printf("Item %lu is not object.\n", i);
            return 0;
        }
        if (!jConfig[i].contains("Id"))
        {
            printf("Item %lu doesnt Id.\n%s\n", i, jConfig[i].dump().c_str());
            return 0;
        }
    }

    /*
    vector<nlohmann::json> jPoints;
    auto branches = argExtractor.get_groups();
    for (size_t i = 0; i < branches.size(); ++i)
    {
        //vector<string> &list = points[branches[i]];
        auto leaf = argExtractor.get_group_argument(branches[i]);
        for (size_t j = 0; j < leaf.size(); ++j)
        {
            //list.push_back(leaf[j]);
            nlohmann::json jPoint;
            jPoint["Id"] = branches[i];
            nlohmann::json &jBranch = jPoint["Branch"];
            jBranch["Id"] = leaf[j];
            jPoints.push_back(move(jPoint));
        }
    }
    */
    uint8_t state = 0;
    TcpSocket mySocket;
    mySocket.set_host("127.0.0.1", 10520);
    nlohmann::json jQuery;
    jQuery["Command"] = "Read";
    nlohmann::json &jBranch = jQuery["Branch"];
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
        case 1: //Read points data
            for (size_t i = 0; i < jConfig.size(); ++i)
            {
                jBranch = jConfig[i];
                if (!mySocket.write(jQuery.dump() + "\n"))
                {
                    mySocket.close();
                    this_thread::sleep_for(100ms);
                    state = 0;
                    break; //break out of this loop to go connect again
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
                                if (jReply.contains("Status") && jReply.contains("Value"))
                                {
                                    nlohmann::json &jStatus = jReply["Status"];
                                    string status = jStatus.get<string>();
                                    if (status == "Good")
                                    {
                                        nlohmann::json &jReplyValue = jReply["Value"];
                                        if (jReplyValue.is_string())
                                        {
                                            printf("Read: %s\n", jReplyValue.get<string>().c_str());
                                        }
                                        else if (jReplyValue.is_number_integer())
                                        {
                                            printf("Read: %ld\n", jReplyValue.get<int64_t>());
                                        }
                                        else if (jReplyValue.is_number_float())
                                        {
                                            printf("Read: %f\n", jReplyValue.get<double>());
                                        }
                                        else
                                        {
                                            printf("Read: unknown: %s.\n", reply.second.c_str());
                                        }
                                    }
                                    else
                                    {
                                        printf("Read status is bad: %s.\n", reply.second.c_str());
                                    }
                                }
                                else
                                {
                                    printf("Read got bad reply: %s.\n", reply.second.c_str());
                                }
                            }
                            catch (nlohmann::json::parse_error &error)
                            {
                                printf("Cannot parse Read reply: %s.\n", reply.second.c_str());
                            }
                        }
                    }
                }
            }
            {
                int random = rand() % 1000;
                random += 500;
                this_thread::sleep_for(chrono::milliseconds(random));
            }
            break;
        }
    }
}