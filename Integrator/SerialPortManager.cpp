#include "SerialPortManager.h"

using namespace std;

SerialPortManager::SerialPortManager(ConfigStorage &config)
{
    auto result = config.get_serial_port_data();
    for (auto i = result.begin(); i != result.end(); ++i)
    {
        auto serialPort = make_shared<SyncedSerialPort>();
        SyncedSerialPort::Config portConfig;
        portConfig.baudrate = i->second.baudrate;
        portConfig.bitPerByte = i->second.bitPerByte;
        portConfig.hardwareFlowControl = i->second.hardwareFlowControl;
        portConfig.parityBit = i->second.paritybit;
        portConfig.softwareFlowControl = i->second.softwareFlowControl;
        portConfig.stopBit = i->second.stopbit;
        serialPort->open(i->first, portConfig);
        serialPort->set_delay(chrono::milliseconds(i->second.msDelay));
        name2SerialPortMap[i->first] = serialPort;
    }
}

SerialPortManager::~SerialPortManager()
{
}

std::shared_ptr<SyncedSerialPort> SerialPortManager::get_serial_port(const std::string &serialPortName)
{
    auto i = name2SerialPortMap.find(serialPortName);
    if(i == name2SerialPortMap.end()) {
        return nullptr;
    }
    return i->second;
}
