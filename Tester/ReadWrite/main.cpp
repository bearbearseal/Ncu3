#include "../../../MyLib/TcpSocket/TcpSocket.h"
#include "../../../OtherLib/nlohmann/json.hpp"
#include <chrono>
#include <cstdlib>
#include <thread>

using namespace std;

int write_process(void *parameters)
{
    TcpSocket mySocket;
    mySocket.set_host("127.0.0.1", 10520);
    nlohmann::json jArray[] = {1, -2, 3, "abc", "xyz", -65.045, 9.9};
    uint8_t state = 0;
    size_t index = 0;

    while (1)
    {
        switch (state)
        {
        case 0: //Connect
            mySocket.open();
            this_thread::sleep_for(100ms);
            while (!mySocket.connection_established())
            {
                this_thread::sleep_for(100ms);
            }
            state = 1;
        case 1: //write
            {
                nlohmann::json jValue;
                jValue["Command"] = "Write";
                jValue["Value"] = jArray[index];
                ++index;
                index %= (sizeof(jArray)/sizeof(jArray[0]));
                nlohmann::json& jBranch = jValue["Branch"];
                jBranch["Id"] = "NewBranch";
                nlohmann::json& jLeaf = jBranch["Branch"];
                jLeaf["Id"] = "123";
                if(!mySocket.write(jValue.dump() + "\n")) {
                    mySocket.close();
                    this_thread::sleep_for(100ms);
                    state = 0;
                }
                else {
                    this_thread::sleep_for(2s);
                }
            }
            break;
        }
    }
    return 0;
}

int main()
{
    TcpSocket mySocket;
    mySocket.set_host("127.0.0.1", 10520);
    uint8_t process = 1;
    thread* writeProcess = nullptr;
connect:
    mySocket.open();
    this_thread::sleep_for(100ms);
    while (!mySocket.connection_established())
    {
        this_thread::sleep_for(100ms);
    }
    switch (process)
    {
    case 1:
        goto create_branch;
        break;
    case 2:
        goto create_leaf;
        break;
    case 3:
        goto read_value;
        break;
    default:
        break;
    }
create_branch:
{
    nlohmann::json jValue;
    jValue["Command"] = "CreateBranch";
    jValue["NewId"] = "NewBranch";
    if (!mySocket.write(jValue.dump() + "\n"))
    {
        mySocket.close();
        this_thread::sleep_for(100ms);
        goto connect;
    }
    for (uint32_t i = 0; i < 10; ++i)
    {
        this_thread::sleep_for(100ms);
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
                        goto create_leaf;
                    }
                }
                else
                {
                    goto create_branch;
                }
            }
            catch (nlohmann::json::parse_error &error)
            {
                printf("Cannot parse CreateBranch reply\n%s\n", reply.second.c_str());
                goto create_branch;
            }
        }
    }
    goto create_branch;
}
create_leaf:
{
    process = 2;
    nlohmann::json jValue;
    jValue["Command"] = "CreateLeaf";
    jValue["NewId"] = "123";
    nlohmann::json &jBranch = jValue["Branch"];
    jBranch["Id"] = "NewBranch";
    if (!mySocket.write(jValue.dump() + "\n"))
    {
        mySocket.close();
        this_thread::sleep_for(100ms);
        goto connect;
    }
    for (uint32_t i = 0; i < 10; ++i)
    {
        this_thread::sleep_for(100ms);
        auto reply = mySocket.read(true);
        if (reply.first)
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
                        //Can create write process now
                        writeProcess = new thread(write_process, nullptr);
                        goto read_value;
                    }
                }
                else
                {
                    goto create_leaf;
                }
            }
            catch (nlohmann::json::parse_error &error)
            {
                printf("Cannot parse CreateLeaf reply.\n");
                goto create_leaf;
            }
        }
    }
    goto create_leaf;
}
read_value:
{
    process = 3;
    nlohmann::json jValue;
    jValue["Command"] = "Read";
    nlohmann::json &jBranch = jValue["Branch"];
    jBranch["Id"] = "NewBranch";
    nlohmann::json &jSubBranch = jBranch["Branch"];
    jSubBranch["Id"] = "123";
    if (!mySocket.write(jValue.dump() + "\n"))
    {
        mySocket.close();
        this_thread::sleep_for(100ms);
        goto connect;
    }
    int random = rand() % 100;
    random += 100;
    this_thread::sleep_for(chrono::milliseconds(random));
    for (uint32_t i = 0; i < 10; ++i)
    {
        this_thread::sleep_for(100ms);
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
                            printf("Read: unknown.\n");
                        }
                    }
                    else
                    {
                        printf("Read status is bad.\n");
                    }
                }
                else
                {
                    printf("Read got bad reply.\n");
                }
            }
            catch (nlohmann::json::parse_error &error)
            {
                printf("Cannot parse Read reply: %s.\n", reply.second.c_str());
            }
        }
    }
    goto read_value;
}
}
