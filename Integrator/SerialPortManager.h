#ifndef _SerialPortManager_H_
#define _SerialPortManager_H_
#include <memory>
#include <unordered_map>

#include "../Storage/ConfigStorage.h"
#include "../../MyLib/SerialPort/SyncedSerialPort.h"

class SerialPortManager {
public:
    SerialPortManager(ConfigStorage& config);
    virtual ~SerialPortManager();

    std::shared_ptr<SyncedSerialPort> get_serial_port(const std::string& serialPortName);

private:
    std::unordered_map<std::string, std::shared_ptr<SyncedSerialPort>> name2SerialPortMap;
};

#endif