#include "ModbusIpProcess.h"
#include "ModbusRegisterValue.h"
#include <iostream>

using namespace std;

void print_binary(const std::string &binary)
{
    for (size_t i = 0; i < binary.size(); ++i)
    {
        printf("[%03u]", uint8_t(binary[i]));
    }
    printf("\n");
}

ModbusIpProcess::ModbusIpProcess(const std::string &hisAddress, uint16_t hisPort, uint8_t slaveAddress, uint16_t maxRegister, uint16_t maxCoilPerMessage, bool smallEndian, std::chrono::milliseconds timeout) : socket(false)
{
    printf("ModbusIp set host to %s port %u\n", hisAddress.c_str(), hisPort);
    socket.set_host(hisAddress, hisPort);
    config.isSmallEndian = smallEndian;
    config.maxCoilPerMessage = maxCoilPerMessage;
    config.maxRegisterPerMessage = maxRegister;
    config.slaveAddress = slaveAddress;
    config.timeout = timeout;
    myShadow = make_shared<Shadow>(*this);
}

ModbusIpProcess::~ModbusIpProcess()
{
    weak_ptr<Shadow> weak = myShadow;
    myShadow.reset();
    while (weak.lock() != nullptr)
    {
        this_thread::sleep_for(100ms);
    }
    stop();
}

void ModbusIpProcess::start()
{
    this->convert_variable_map_to_query();
    lock_guard<mutex> lock(threadData.threadMutex);
    if (threadData.theProcess == nullptr)
    {
        threadData.keepRunning = true;
        threadData.theProcess = new thread(thread_process, this);
    }
}

void ModbusIpProcess::stop()
{
    lock_guard<mutex> lock(threadData.threadMutex);
    threadData.keepRunning = false;
    cout << "Closing socket\n";
    socket.close();
    cout << "Socket close\n";
    if (threadData.theProcess != nullptr)
    {
        threadData.theProcess->join();
        threadData.theProcess = nullptr;
    }
}

shared_ptr<ModbusIpProcess::CoilStatusVariable> ModbusIpProcess::get_coil_status_variable(uint16_t coilAddress, shared_ptr<OperationalLogic> inLogic, shared_ptr<OperationalLogic> outLogic)
{
    auto i = address2CoilMap.find(coilAddress);
    if (i != address2CoilMap.end())
    {
        return i->second;
    }
    shared_ptr<CoilStatusVariable> retVal = make_shared<CoilStatusVariable>(myShadow, coilAddress, inLogic, outLogic);
    address2CoilMap[coilAddress] = retVal;
    return retVal;
}

shared_ptr<ModbusIpProcess::HoldingRegisterVariable> ModbusIpProcess::get_holding_register_variable(uint16_t registerAddress, GlobalEnum::ModbusDataType type, shared_ptr<OperationalLogic> inLogic, shared_ptr<OperationalLogic> outLogic)
{
    shared_ptr<HoldingRegisterVariable> retVal = make_shared<HoldingRegisterVariable>(myShadow, registerAddress, type, config.isSmallEndian, inLogic, outLogic);
    uint16_t count = ModbusRegisterValue::get_register_count(type);
    address2HoldingRegisterMap[registerAddress].variables.push_back(retVal);
    if (address2HoldingRegisterMap[registerAddress].count < count)
    {
        address2HoldingRegisterMap[registerAddress].count = count;
    }
    return retVal;
}

shared_ptr<ModbusIpProcess::DigitalInputVariable> ModbusIpProcess::get_digital_input_variable(uint16_t address, shared_ptr<OperationalLogic> inLogic, shared_ptr<OperationalLogic> outLogic)
{
    auto i = address2DigitalInputMap.find(address);
    if (i != address2DigitalInputMap.end())
    {
        return i->second;
    }
    shared_ptr<DigitalInputVariable> retVal = make_shared<DigitalInputVariable>(myShadow, address, inLogic, outLogic);
    address2DigitalInputMap[address] = retVal;
    return retVal;
}

shared_ptr<ModbusIpProcess::InputRegisterVariable> ModbusIpProcess::get_input_register_variable(uint16_t registerAddress, GlobalEnum::ModbusDataType type, shared_ptr<OperationalLogic> inLogic, shared_ptr<OperationalLogic> outLogic)
{
    shared_ptr<InputRegisterVariable> retVal = make_shared<InputRegisterVariable>(myShadow, registerAddress, type, config.isSmallEndian, inLogic, outLogic);
    uint16_t count = ModbusRegisterValue::get_register_count(type);
    address2InputRegisterMap[registerAddress].variables.push_back(retVal);
    if (address2InputRegisterMap[registerAddress].count < count)
    {
        address2InputRegisterMap[registerAddress].count = count;
    }
    return retVal;
}

bool should_join(uint16_t newAddress, uint16_t startAddress, uint16_t count)
{
    // uint16_t offset = endAddress - startAddress;
    if (count % 8)
    {
        count /= 8;
        count += 1;
        count *= 8;
    }
    if (newAddress >= (startAddress + count + 7))
    {
        return false;
    }
    return true;
}

void ModbusIpProcess::convert_variable_map_to_query()
{
    {
        bool empty = true;
        CoilQueryData entry;
        for (auto i = address2CoilMap.begin(); i != address2CoilMap.end(); ++i)
        {
            if (empty)
            {
                entry.startAddress = i->first;
                entry.coilCount = 1;
                entry.variables.push_back(i->second);
                empty = false;
            }
            else if (i->first - entry.startAddress > config.maxCoilPerMessage)
            {
                // add entry to the query list
                coilQueryList.push_back(move(entry));
                // reinit entry
                entry.variables.clear();
                entry.startAddress = i->first;
                entry.coilCount = 1;
                entry.variables.push_back(i->second);
                // empty = true;
            }
            else if (!should_join(i->first, entry.startAddress, entry.coilCount))
            {
                // else if(i->first - (entry.startAddress + entry.coilCount) > 8) {
                // add entry to the query list
                coilQueryList.push_back(move(entry));
                // reinit entry
                entry.variables.clear();
                entry.startAddress = i->first;
                entry.coilCount = 1;
                entry.variables.push_back(i->second);
            }
            else
            {
                // add variable to entry
                entry.variables.push_back(i->second);
                entry.coilCount = i->first - entry.startAddress + 1;
            }
        }
        if (!empty)
        {
            coilQueryList.push_back(move(entry));
        }
        address2CoilMap.clear();
    }
    {
        bool empty = true;
        DigitalInputQueryData entry;
        for (auto i = address2DigitalInputMap.begin(); i != address2DigitalInputMap.end(); ++i)
        {
            if (empty)
            {
                entry.startAddress = i->first;
                entry.digitalInputCount = 1;
                entry.variables.push_back(i->second);
                empty = false;
            }
            else if (i->first - entry.startAddress > config.maxCoilPerMessage)
            {
                // add entry to the query list
                digitalInputQueryList.push_back(move(entry));
                // reinit entry
                entry.variables.clear();
                entry.startAddress = i->first;
                entry.digitalInputCount = 1;
                entry.variables.push_back(i->second);
                // empty = true;
            }
            else if (!should_join(i->first, entry.startAddress, entry.digitalInputCount))
            {
                // else if(i->first - (entry.startAddress + entry.coilCount) > 8) {
                // add entry to the query list
                digitalInputQueryList.push_back(move(entry));
                // reinit entry
                entry.variables.clear();
                entry.startAddress = i->first;
                entry.digitalInputCount = 1;
                entry.variables.push_back(i->second);
            }
            else
            {
                // add variable to entry
                entry.variables.push_back(i->second);
                entry.digitalInputCount = i->first - entry.startAddress + 1;
            }
        }
        if (!empty)
        {
            digitalInputQueryList.push_back(move(entry));
        }
        address2CoilMap.clear();
    }
    {
        bool empty = true;
        HoldingRegisterQueryData element;
        for (auto i = address2HoldingRegisterMap.begin(); i != address2HoldingRegisterMap.end(); ++i)
        {
            if (empty)
            {
                element.startAddress = i->first;
                element.registerCount = i->second.count;
                element.variables.insert(element.variables.end(), i->second.variables.begin(), i->second.variables.end());
                empty = false;
            }
            else if ((i->first + i->second.count) - element.startAddress > config.maxRegisterPerMessage)
            {
                // add entry to the query list
                holdingRegisterQueryList.push_back(move(element));
                // reinit entry
                element.variables.clear();
                element.startAddress = i->first;
                element.registerCount = i->second.count;
                element.variables.insert(element.variables.end(), i->second.variables.begin(), i->second.variables.end());
                // empty = true;
            }
            else if (i->first > (element.startAddress + element.registerCount + 1))
            {
                // add entry to the query list
                holdingRegisterQueryList.push_back(move(element));
                // reinit entry
                element.variables.clear();
                element.startAddress = i->first;
                element.registerCount = i->second.count;
                element.variables.insert(element.variables.end(), i->second.variables.begin(), i->second.variables.end());
            }
            else
            {
                // add variable to entry
                element.variables.insert(element.variables.end(), i->second.variables.begin(), i->second.variables.end());
                element.registerCount = i->first + i->second.count - element.startAddress;
            }
        }
        if (!empty)
        {
            holdingRegisterQueryList.push_back(move(element));
        }
        address2HoldingRegisterMap.clear();
    }
    {
        bool empty = true;
        InputRegisterQueryData element;
        for (auto i = address2InputRegisterMap.begin(); i != address2InputRegisterMap.end(); ++i)
        {
            if (empty)
            {
                element.startAddress = i->first;
                element.registerCount = i->second.count;
                element.variables.insert(element.variables.end(), i->second.variables.begin(), i->second.variables.end());
                empty = false;
            }
            else if ((i->first + i->second.count) - element.startAddress > config.maxRegisterPerMessage)
            {
                // add entry to the query list
                inputRegisterQueryList.push_back(move(element));
                // reinit entry
                element.variables.clear();
                element.startAddress = i->first;
                element.registerCount = i->second.count;
                element.variables.insert(element.variables.end(), i->second.variables.begin(), i->second.variables.end());
                // empty = true;
            }
            else if (i->first > (element.startAddress + element.registerCount + 1))
            {
                // add entry to the query list
                inputRegisterQueryList.push_back(move(element));
                // reinit entry
                element.variables.clear();
                element.startAddress = i->first;
                element.registerCount = i->second.count;
                element.variables.insert(element.variables.end(), i->second.variables.begin(), i->second.variables.end());
            }
            else
            {
                // add variable to entry
                element.variables.insert(element.variables.end(), i->second.variables.begin(), i->second.variables.end());
                element.registerCount = i->first + i->second.count - element.startAddress;
            }
        }
        if (!empty)
        {
            inputRegisterQueryList.push_back(move(element));
        }
        address2InputRegisterMap.clear();
    }
}

pair<bool, string> get_reply(TcpSocket &socket, uint16_t expectedLength, chrono::milliseconds timeout)
{
    string retVal;
    // chrono::time_point<chrono::steady_clock> beginTime = chrono::steady_clock::now();
    // do {
    // this_thread::sleep_for(10ms);
    this_thread::sleep_for(timeout);
    auto reply = socket.read();
    if (!reply.first)
    {
        printf("Cannot get reply.\n");
        return {false, retVal};
    }
    else
    {
        retVal += reply.second;
        if (retVal.size() >= expectedLength)
        {
            return {true, retVal};
        }
    }
    //}while(chrono::steady_clock::now() - beginTime < timeout);
    return {false, retVal};
}

pair<bool, string> ModbusIpProcess::query_holding_register_then_get_reply(const HoldingRegisterQueryData &queryData, uint16_t sequenceNumber, chrono::milliseconds waitTime)
{
    string retVal;
    auto query = ModbusIP::construct_read_holding_registers(sequenceNumber, config.slaveAddress, queryData.startAddress, queryData.registerCount);
    if (!socket.write(query.first))
    {
        return {false, retVal};
    }
    this_thread::sleep_for(waitTime);
    auto reply = socket.read();
    if (!reply.first)
    {
        printf("Read holding register no reply.\n");
        return {false, retVal};
    }
    else
    {
        retVal += reply.second;
        if (retVal.size() >= query.second)
        {
            return {true, retVal};
        }
    }
    return {false, retVal};
}

pair<bool, string> ModbusIpProcess::query_input_register_then_get_reply(const InputRegisterQueryData &queryData, uint16_t sequenceNumber, std::chrono::milliseconds waitTime)
{
    string retVal;
    auto query = ModbusIP::construct_read_input_registers(sequenceNumber, config.slaveAddress, queryData.startAddress, queryData.registerCount);
    if (!socket.write(query.first))
    {
        return {false, retVal};
    }
    this_thread::sleep_for(waitTime);
    auto reply = socket.read();
    if (!reply.first)
    {
        printf("Read holding register no reply.\n");
        return {false, retVal};
    }
    else
    {
        retVal += reply.second;
        if (retVal.size() >= query.second)
        {
            return {true, retVal};
        }
    }
    return {false, retVal};
}

// return false if socket got problem
bool ModbusIpProcess::do_write_coil_query(uint16_t &sequenceNumber, chrono::milliseconds waitTime)
{
    // while(1) {
    uint16_t coilAddress;
    bool value;
    {
        lock_guard<mutex> lock(forceCoilData.mapMutex);
        auto entry = forceCoilData.valueMap.begin();
        if (entry == forceCoilData.valueMap.end())
        {
            return true;
        }
        coilAddress = entry->first;
        value = entry->second;
        forceCoilData.valueMap.erase(entry);
    }
    ++sequenceNumber;
    auto writeData = ModbusIP::construct_write_single_coil(sequenceNumber, config.slaveAddress, coilAddress, value);
    if (!socket.write(writeData.first))
    {
        lock_guard<mutex> lock(forceCoilData.mapMutex);
        forceCoilData.valueMap.clear(); // Socket error, dunno when can connect back, discard all pending write
        return false;
    }
    // auto beginTime = chrono::steady_clock::now();
    string reply;
    // chrono::duration<int, std::milli> delay(milliSecond);
    // do {
    // this_thread::sleep_for(10ms);
    this_thread::sleep_for(waitTime);
    auto replyData = socket.read();
    if (!replyData.first)
    {
        return false;
    }
    reply += replyData.second;
    return true;
    //}while(reply.size() < writeData.second && chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - beginTime) < waitTime);
    //}
}

bool ModbusIpProcess::do_write_holding_register_query(uint16_t &sequenceNumber, chrono::milliseconds waitTime)
{
    // while(1) {
    uint16_t registerAddress;
    std::vector<RegisterValue> values;
    {
        lock_guard<mutex> lock(writeHoldingRegisterData.mapMutex);
        auto entry = writeHoldingRegisterData.valueMap.begin();
        if (entry == writeHoldingRegisterData.valueMap.end())
        {
            return true;
        }
        registerAddress = entry->first;
        values = move(entry->second);
        writeHoldingRegisterData.valueMap.erase(entry);
    }
    ++sequenceNumber;
    // cout<<"Writing to holding registers\n";
    printf("Got write register request.\n");
    auto writeData = ModbusIP::construct_write_multiple_holding_registers(sequenceNumber, config.slaveAddress, registerAddress, values);
    if (!socket.write(writeData.first))
    {
        lock_guard<mutex> lock(writeHoldingRegisterData.mapMutex);
        writeHoldingRegisterData.valueMap.clear();
        return false;
    }
    // auto beginTime = chrono::steady_clock::now();
    string reply;
    // do {
    // this_thread::sleep_for(10ms);
    this_thread::sleep_for(waitTime);
    auto replyData = socket.read();
    if (!replyData.first)
    {
        return false;
    }
    reply += replyData.second;
    return true;
    //}while(reply.size() < writeData.second && chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - beginTime) < waitTime);
    //}
}

void ModbusIpProcess::thread_process(ModbusIpProcess *me)
{
    printf("ModbusIP thread started...\n");
    uint16_t sequenceNumber = 0;
    auto coilIter = me->coilQueryList.begin();
    auto digitalInputIter = me->digitalInputQueryList.begin();
    auto holdingRegisterIter = me->holdingRegisterQueryList.begin();
    auto inputRegisterIter = me->inputRegisterQueryList.begin();
    while (me->threadData.keepRunning)
    {
        switch (me->threadData.mainState)
        {
        case 0: // Connect
            if (me->socket.open())
            {
                me->threadData.mainState = 1;
            }
            else
            {
                this_thread::sleep_for(20ms);
            }
            break;
        case 1: // wait connection established
            // cout<<"State 1\n";
            if (me->socket.connection_established())
            {
                cout << "ModbusIp Connected.\n";
                me->threadData.mainState = 10;
                me->threadData.subState = 0;
            }
            else
            {
                // cout << "ModbusIp Cannot connect.\n";
                this_thread::sleep_for(10s);
            }
            break;
        case 10: // Routine operation
            // cout<<"State 10\n";
            // Do write coil if got any
            if (!me->do_write_coil_query(sequenceNumber, me->config.timeout))
            {
                me->threadData.mainState = 20; // close socket
                continue;
            }
            // Do write register if got any
            if (!me->do_write_holding_register_query(sequenceNumber, me->config.timeout))
            {
                me->threadData.mainState = 20; // close socket
                continue;
            }
            switch (me->threadData.subState)
            {
            case 0: // Query Coil
                if (coilIter == me->coilQueryList.end())
                {
                    coilIter = me->coilQueryList.begin();
                    me->threadData.subState = 1; // Query Digital Input
                }
                else
                {
                    CoilQueryData &queryData = (*coilIter);
                    ++sequenceNumber;
                    auto query = ModbusIP::construct_read_coils(sequenceNumber, me->config.slaveAddress, queryData.startAddress, queryData.coilCount);
                    // clear socket buffer
                    me->socket.read(true);
                    if (!me->socket.write(query.first))
                    {
                        printf("Read coil failed!\n");
                        me->threadData.mainState = 20; // close socket
                        continue;
                    }
                    else
                    {
                        auto reply = get_reply(me->socket, query.second, me->config.timeout);
                        if (!reply.first)
                        {
                            ++me->threadData.failCount;
                            printf("No read coil reply.\n");
                            if (me->threadData.failCount > 3)
                            {
                                me->threadData.mainState = 20; // close socket
                            }
                            else
                            {
                                this_thread::sleep_for(100ms);
                            }
                            continue;
                        }
                        auto decoded = ModbusIP::decode_reply(reply.second);
                        if (decoded.functionCode == ModbusIP::READ_COIL_CODE && decoded.sequenceNumber == sequenceNumber)
                        {
                            me->threadData.failCount = 0;
                            // distribute the reply.
                            for (auto i = queryData.variables.begin(); i != queryData.variables.end(); ++i)
                            {
                                (*i)->update_value_from_source(queryData.startAddress, decoded.get_coils());
                            }
                        }
                        else
                        {
                            printf("Error reply function code: %u vs %u sequence number %u vs %u.\n", decoded.functionCode, ModbusIP::READ_COIL_CODE, decoded.sequenceNumber, sequenceNumber);
                        }
                    }
                    ++coilIter;
                }
                break;
            case 1: // Query Digital Input
                if (digitalInputIter == me->digitalInputQueryList.end())
                {
                    digitalInputIter = me->digitalInputQueryList.begin();
                    me->threadData.subState = 2; // Query Holding Register
                }
                else
                {
                    DigitalInputQueryData &queryData = (*digitalInputIter);
                    ++sequenceNumber;
                    auto query = ModbusIP::construct_read_digital_input(sequenceNumber, me->config.slaveAddress, queryData.startAddress, queryData.digitalInputCount);
                    // clear socket buffer
                    me->socket.read(true);
                    if (!me->socket.write(query.first))
                    {
                        printf("Read digital input failed!\n");
                        me->threadData.mainState = 20; // close socket
                        continue;
                    }
                    else
                    {
                        auto reply = get_reply(me->socket, query.second, me->config.timeout);
                        if (!reply.first)
                        {
                            ++me->threadData.failCount;
                            printf("No read digital input reply.\n");
                            if (me->threadData.failCount > 3)
                            {
                                me->threadData.mainState = 20; // close socket
                            }
                            else
                            {
                                this_thread::sleep_for(100ms);
                            }
                            continue;
                        }
                        auto decoded = ModbusIP::decode_reply(reply.second);
                        if (decoded.functionCode == ModbusIP::READ_INPUT_CODE && decoded.sequenceNumber == sequenceNumber)
                        {
                            me->threadData.failCount = 0;
                            // distribute the reply.
                            for (auto i = queryData.variables.begin(); i != queryData.variables.end(); ++i)
                            {
                                (*i)->update_value_from_source(queryData.startAddress, decoded.get_input_status());
                            }
                        }
                        else
                        {
                            printf("Error reply function code: %u vs %u sequence number %u vs %u.\n", decoded.functionCode, ModbusIP::READ_COIL_CODE, decoded.sequenceNumber, sequenceNumber);
                        }
                    }
                    ++coilIter;
                }
                break;
            case 2: // Query Holding Register
                if (holdingRegisterIter == me->holdingRegisterQueryList.end())
                {
                    holdingRegisterIter = me->holdingRegisterQueryList.begin();
                    me->threadData.subState = 3; // Query Input Register
                }
                else
                {
                    HoldingRegisterQueryData &queryData = (*holdingRegisterIter);
                    ++sequenceNumber;
                    // clear socket buffer
                    me->socket.read(true);
                    auto reply = me->query_holding_register_then_get_reply(queryData, sequenceNumber, me->config.timeout);
                    if (!reply.first)
                    {
                        printf("No read holding register reply.\n");
                        ++me->threadData.failCount;
                        if (me->threadData.failCount > 3)
                        {
                            me->threadData.mainState = 20; // close socket
                        }
                        else
                        {
                            this_thread::sleep_for(100ms);
                        }
                        continue;
                    }
                    auto decoded = ModbusIP::decode_reply(reply.second);
                    // printf("Read holding register value decoded.\n");
                    if (decoded.functionCode == ModbusIP::READ_HOLDING_REGISTER_CODE && decoded.sequenceNumber == sequenceNumber)
                    {
                        me->threadData.failCount = 0;
                        // distribute the reply
                        for (auto i = queryData.variables.begin(); i != queryData.variables.end(); ++i)
                        {
                            // printf("Updating value from source.\n");
                            (*i)->update_value_from_source(queryData.startAddress, decoded.get_holding_register());
                        }
                    }
                    else
                    {
                        printf("Error reply function code: %u vs %u sequence number %u vs %u.\n", decoded.functionCode, ModbusIP::READ_HOLDING_REGISTER_CODE, decoded.sequenceNumber, sequenceNumber);
                    }
                    // this_thread::sleep_for(10s);
                    ++holdingRegisterIter;
                }
                break;
            case 3: // Query Input Register
                if (inputRegisterIter == me->inputRegisterQueryList.end())
                {
                    inputRegisterIter = me->inputRegisterQueryList.begin();
                    me->threadData.subState = 0; // Query Coil
                }
                else
                {
                    InputRegisterQueryData &queryData = (*inputRegisterIter);
                    ++sequenceNumber;
                    // clear socket buffer
                    me->socket.read(true);
                    auto reply = me->query_input_register_then_get_reply(queryData, sequenceNumber, me->config.timeout);
                    if (!reply.first)
                    {
                        printf("No read rinput egister reply.\n");
                        ++me->threadData.failCount;
                        if (me->threadData.failCount > 3)
                        {
                            me->threadData.mainState = 20; // close socket
                        }
                        else
                        {
                            this_thread::sleep_for(100ms);
                        }
                        continue;
                    }
                    auto decoded = ModbusIP::decode_reply(reply.second);
                    if (decoded.functionCode == ModbusIP::READ_INPUT_REGISTER_CODE && decoded.sequenceNumber == sequenceNumber)
                    {
                        me->threadData.failCount = 0;
                        // distribute the reply
                        for (auto i = queryData.variables.begin(); i != queryData.variables.end(); ++i)
                        {
                            (*i)->update_value_from_source(queryData.startAddress, decoded.get_input_register());
                        }
                    }
                    else
                    {
                        printf("Error reply function code: %u vs %u sequence number %u vs %u.\n", decoded.functionCode, ModbusIP::READ_HOLDING_REGISTER_CODE, decoded.sequenceNumber, sequenceNumber);
                    }
                    // this_thread::sleep_for(10s);
                    ++holdingRegisterIter;
                }
                break;
            }
            break;
        case 20: // Close socket
        default:
            // cout<<"State 20\n";
            cout << "ModbusIp close socket.\n";
            me->socket.close();
            me->threadData.mainState = 0;
            me->threadData.failCount = 0;
            this_thread::sleep_for(1s);
            break;
        }
        this_thread::sleep_for(10ms);
    }
    printf("Thread stop.\n");
}

void ModbusIpProcess::force_coil(uint16_t coilAddress, bool value)
{
    lock_guard<mutex> lock(forceCoilData.mapMutex);
    forceCoilData.valueMap[coilAddress] = value;
}

void ModbusIpProcess::write_multiple_holding_register(uint16_t registerAddress, const std::vector<RegisterValue> &values)
{
    // printf("Got write multiple holding register request.\n");
    lock_guard<mutex> lock(writeHoldingRegisterData.mapMutex);
    writeHoldingRegisterData.valueMap[registerAddress] = values;
}

ModbusIpProcess::CoilStatusVariable::CoilStatusVariable(shared_ptr<Shadow> _master, uint16_t _coilAddress, std::shared_ptr<OperationalLogic> _inLogic, std::shared_ptr<OperationalLogic> _outLogic) : OperationVariable(_inLogic, _outLogic), master(_master), coilAddress(_coilAddress) {}

ModbusIpProcess::CoilStatusVariable::~CoilStatusVariable() {}

bool ModbusIpProcess::CoilStatusVariable::_write_value(const Value &newValue)
{
    auto shared = master.lock();
    if (shared != nullptr)
    {
        shared->force_coil(coilAddress, (bool)newValue.get_int());
        return true;
    }
    return false;
}

void ModbusIpProcess::CoilStatusVariable::update_value_from_source(uint16_t firstAddress, const vector<bool> &values)
{
    size_t index = coilAddress - firstAddress;
    if (index >= values.size())
    {
        return;
    }
    this->update_value_to_cache(values[index]);
}

ModbusIpProcess::DigitalInputVariable::DigitalInputVariable(std::shared_ptr<Shadow> _master, uint16_t _address, shared_ptr<OperationalLogic> _inLogic, shared_ptr<OperationalLogic> _outLogic) : OperationVariable(_inLogic, _outLogic), master(_master), address(_address) {}

ModbusIpProcess::DigitalInputVariable::~DigitalInputVariable() {}

void ModbusIpProcess::DigitalInputVariable::update_value_from_source(uint16_t firstAddress, const std::vector<bool> &values)
{
    size_t index = address - firstAddress;
    if (index >= values.size())
    {
        return;
    }
    this->update_value_to_cache(values[index]);
}

ModbusIpProcess::HoldingRegisterVariable::HoldingRegisterVariable(
    shared_ptr<Shadow> _master, uint16_t _firstAddress, GlobalEnum::ModbusDataType _type, bool _smallEndian, shared_ptr<OperationalLogic> _inLogic, shared_ptr<OperationalLogic> _outLogic) : OperationVariable(_inLogic, _outLogic), master(_master)
{
    firstAddress = _firstAddress;
    type = _type;
    smallEndian = _smallEndian;
}

ModbusIpProcess::HoldingRegisterVariable::~HoldingRegisterVariable() {}

bool ModbusIpProcess::HoldingRegisterVariable::_write_value(const Value &newValue)
{
    // printf("HR Variable got write value requst.\n");
    ModbusRegisterValue setValue(type, smallEndian);
    std::vector<RegisterValue> converted = setValue.convert_to_register_value(newValue);
    auto shared = master.lock();
    if (shared != nullptr)
    {
        shared->write_multiple_holding_register(firstAddress, converted);
        return true;
    }
    return false;
}

void ModbusIpProcess::HoldingRegisterVariable::update_value_from_source(uint16_t _registerAddress, const vector<RegisterValue> &values)
{
    size_t index = firstAddress - _registerAddress;
    if (index >= values.size())
    {
        return;
    }
    ModbusRegisterValue modbusValue(type, smallEndian);
    modbusValue.set_register_value(values, index);
    this->update_value_to_cache(modbusValue.get_value());
}

ModbusIpProcess::InputRegisterVariable::InputRegisterVariable(
    std::shared_ptr<Shadow> _master, uint16_t _firstAddress, GlobalEnum::ModbusDataType _type, bool _smallEndian, shared_ptr<OperationalLogic> _inLogic, shared_ptr<OperationalLogic> _outLogic) : OperationVariable(_inLogic, _outLogic), master(_master)
{
    firstAddress = _firstAddress;
    type = _type;
    smallEndian = _smallEndian;
}

ModbusIpProcess::InputRegisterVariable::~InputRegisterVariable() {}

void ModbusIpProcess::InputRegisterVariable::update_value_from_source(uint16_t _registerAddress, const std::vector<RegisterValue> &values)
{
    size_t index = firstAddress - _registerAddress;
    if (index >= values.size())
    {
        return;
    }
    ModbusRegisterValue modbusValue(type, smallEndian);
    modbusValue.set_register_value(values, index);
    this->update_value_to_cache(modbusValue.get_value());
}
