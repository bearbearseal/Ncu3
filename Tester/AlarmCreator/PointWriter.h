#ifndef _PointWriter_H_
#define _PointWriter_H_
#include <vector>
#include <thread>
#include "../../../OtherLib/nlohmann/json.hpp"
#include "../../../MyLib/Basic/Value.h"
#include "../../../MyLib/TcpSocket/TcpSocket.h"
#include "../../../MyLib/File/LogThreadWriter.h"

class PointWriter
{
public:
    PointWriter(const std::string& ipAddress, const std::string& portNumber, const std::string& config);
    ~PointWriter();

private:
    static void the_process(PointWriter* pointWriter);

    uint8_t state = 0;
    TcpSocket mySocket;
    std::unique_ptr<std::thread> theProcess = nullptr;
    size_t dataIndex = 0;
    std::vector<std::string> writeDataList;
    LogThreadWriter log;
    bool run;
};

#endif