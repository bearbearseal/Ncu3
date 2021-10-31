#include "PointWriter.h"
#include "../../../OtherLib/nlohmann/json.hpp"
#include <time.h>
#include <stdlib.h>

using namespace std;

/*
Accept data format
[{"Device":<>, "Point":<>, "Value":<>}, {"Device":<>, "Point":<>, "Value":<>}]
*/

PointWriter::PointWriter(const std::string &ipAddress, const std::string &portNumber, const std::string &config) : log("Log.txt")
{
    mySocket.set_host(ipAddress, stoi(portNumber));
    state = 0;
    nlohmann::json theJson;
    try
    {
        theJson = nlohmann::json::parse(config);
    }
    catch (nlohmann::json::parse_error &error)
    {
        printf("Cannot parse config: %s\n", config.c_str());
        return;
    }
    if(!theJson.is_array())
    {
        return;
    }
    for(size_t i=0 ;i<theJson.size(); ++i)
    {
        nlohmann::json& jEntry = theJson[i];
        if(!jEntry.is_object())
        {
            continue;
        }
        if(!jEntry.contains("Device") || !jEntry.contains("Point") || !jEntry.contains("Value"))
        {
            continue;
        }
        nlohmann::json& jDevice = jEntry["Device"];
        nlohmann::json& jPoint = jEntry["Point"];
        nlohmann::json& jValue = jEntry["Value"];
        if(!jDevice.is_number_unsigned() || !jPoint.is_number_unsigned() || !jValue.is_number())
        {
            continue;
        }
        nlohmann::json jElement;
        jElement["Device"] = jDevice;
        jElement["Point"] = jPoint;
        jElement["Value"] = jValue;
        writeDataList.push_back(jElement.dump() + '\n');
    }
    run = true;
    theProcess = make_unique<thread>(the_process, this);
}

PointWriter::~PointWriter()
{
    run = false;
    theProcess->join();
}

void PointWriter::the_process(PointWriter *me)
{
    me->log.clear();
    printf("Thread started.\n");
    while (me->run)
    {
        switch (me->state)
        {
        case 0: // try connect
            me->mySocket.open();
            do
            {
                this_thread::sleep_for(100ms);
            } while (!me->mySocket.connection_established());
            me->state = 10;

        case 10: // write data
            if (!me->mySocket.write(me->writeDataList[me->dataIndex]))
            {
                me->mySocket.close();
                this_thread::sleep_for(100ms);
                me->state = 0;
            }
            else
            {
                me->state = 20;
                for (size_t i = 0; i < 10; ++i)
                {
                    this_thread::sleep_for(20ms);
                    auto reply = me->mySocket.read(true);
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
                                    me->log.add_log("Write status is bad: %s.\n", reply.second.c_str());
                                    printf("Write status is bad: %s.\n", reply.second.c_str());
                                }
                            }
                            else
                            {
                                me->log.add_log("Write got bad reply: %s.\n", reply.second.c_str());
                                printf("Write got bad reply: %s.\n", reply.second.c_str());
                            }
                        }
                        catch (nlohmann::json::parse_error &error)
                        {
                            me->log.add_log("Cannot parse write reply: %s.\n", reply.second.c_str());
                            printf("Cannot parse write reply: %s.\n", reply.second.c_str());
                        }
                    }
                }
            }
            break;

        case 20:
            ++me->dataIndex;
            me->dataIndex %= me->writeDataList.size();
            {
                uint16_t delay = rand()%10000;
                this_thread::sleep_for(chrono::seconds(delay));
                me->state = 10;
            }
            break;
        }
    }
    printf("Thread ended.\n");
}
