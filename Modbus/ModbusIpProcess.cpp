#include "ModbusIpProcess.h"
#include "ModbusRegisterValue.h"
#include <iostream>

using namespace std;

ModbusIpProcess::ModbusIpProcess(const std::string& hisAddress, uint16_t hisPort, uint8_t slaveAddress, uint16_t maxRegister, uint16_t maxCoilPerMessage, bool smallEndian, std::chrono::milliseconds timeout) : socket(false) {
    printf("ModbusIp set host to %s port %u\n", hisAddress.c_str(), hisPort);
    socket.set_host(hisAddress, hisPort);
    config.isSmallEndian = smallEndian;
    config.maxCoilPerMessage = maxCoilPerMessage;
    config.maxRegisterPerMessage = maxRegister;
    config.slaveAddress = slaveAddress;
    config.timeout = timeout;
    myShadow = make_shared<Shadow>(*this);
}

ModbusIpProcess::~ModbusIpProcess() {
    weak_ptr<Shadow> weak = myShadow;
    myShadow.reset();
    while(weak.lock() != nullptr) { this_thread::sleep_for(100ms); }
    stop();
}

void ModbusIpProcess::start() {
    this->convert_variable_map_to_query();
    lock_guard<mutex> lock(threadData.threadMutex);
    if(threadData.theProcess == nullptr) {
        threadData.keepRunning = true;
        threadData.theProcess = new thread(thread_process, this);
    }
}

void ModbusIpProcess::stop() {
    lock_guard<mutex> lock(threadData.threadMutex);
    threadData.keepRunning = false;
    cout<<"Closing socket\n";
    socket.close();
    cout<<"Socket close\n";
    if(threadData.theProcess != nullptr){
        threadData.theProcess->join();
        threadData.theProcess = nullptr;
    }
}

shared_ptr<ModbusIpProcess::CoilStatusVariable> ModbusIpProcess::get_coil_status_variable(uint16_t coilAddress) {
    auto i = address2CoilMap.find(coilAddress);
    if(i != address2CoilMap.end()) {
        return i->second;
    }
    shared_ptr<CoilStatusVariable> retVal = make_shared<CoilStatusVariable>(myShadow, coilAddress);
    address2CoilMap[coilAddress] = retVal;
    return retVal;
}

shared_ptr<ModbusIpProcess::HoldingRegisterVariable> ModbusIpProcess::get_holding_register_variable(uint16_t registerAddress, ModbusRegisterValue::DataType type) {
    shared_ptr<HoldingRegisterVariable> retVal = make_shared<HoldingRegisterVariable>(myShadow, registerAddress, type, config.isSmallEndian);
    uint16_t count = ModbusRegisterValue::get_register_count(type);
    address2HoldingRegisterMap[registerAddress].variables.push_back(retVal);
    if(address2HoldingRegisterMap[registerAddress].count < count) {
        address2HoldingRegisterMap[registerAddress].count = count;
    }
    return retVal;
}

bool should_join(uint16_t newAddress, uint16_t startAddress, uint16_t count){
    //uint16_t offset = endAddress - startAddress;
    if(count%8) {
        count /= 8;
        count += 1;
        count *= 8;
    }
    if(newAddress >= (startAddress + count + 7)){
        return false;
    }
    return true;
}

void ModbusIpProcess::convert_variable_map_to_query() {
    bool empty = true;
    CoilQueryData entry;
    for(auto i = address2CoilMap.begin(); i != address2CoilMap.end(); ++i) {
        if(empty) {
            entry.startAddress = i->first;
            entry.coilCount = 1;
            entry.variables.push_back(i->second);
            empty = false;
        }
        else if(i->first - entry.startAddress > config.maxCoilPerMessage) {
            //add entry to the query list
            coilQueryList.push_back(move(entry));
            //reinit entry
            entry.variables.clear();
            entry.startAddress = i->first;
            entry.coilCount = 1;
            entry.variables.push_back(i->second);
            //empty = true;
        }
        else if(!should_join(i->first, entry.startAddress, entry.coilCount)) {
        //else if(i->first - (entry.startAddress + entry.coilCount) > 8) {
            //add entry to the query list
            coilQueryList.push_back(move(entry));
            //reinit entry
            entry.variables.clear();
            entry.startAddress = i->first;
            entry.coilCount = 1;
            entry.variables.push_back(i->second);
            //empty = true;
        }
        else {
            //add variable to entry
            entry.variables.push_back(i->second);
            entry.coilCount = i->first - entry.startAddress + 1;
        }
    }
    if(!empty) {
        coilQueryList.push_back(move(entry));
    }
    address2CoilMap.clear();
    empty = true;
    HoldingRegisterQueryData element;
    for(auto i = address2HoldingRegisterMap.begin(); i != address2HoldingRegisterMap.end(); ++i) {
        if(empty) {
            element.startAddress = i->first;
            element.registerCount = i->second.count;
            element.variables.insert(element.variables.end(), i->second.variables.begin(), i->second.variables.end());
            empty = false;
        }
        else if((i->first + i->second.count) - element.startAddress > config.maxRegisterPerMessage) {
            //add entry to the query list
            holdingRegisterQueryList.push_back(move(element));
            //reinit entry
            element.variables.clear();
            element.startAddress = i->first;
            element.registerCount = i->second.count;
            element.variables.insert(element.variables.end(), i->second.variables.begin(), i->second.variables.end());
            //empty = true;
        }
        else if(i->first > (element.startAddress + element.registerCount + 1)) {
            //add entry to the query list
            holdingRegisterQueryList.push_back(move(element));
            //reinit entry
            element.variables.clear();
            element.startAddress = i->first;
            element.registerCount = i->second.count;
            element.variables.insert(element.variables.end(), i->second.variables.begin(), i->second.variables.end());
            //empty = true;
        }
        else {
            //add variable to entry
            //element.variables.push_back(i->second);
            element.variables.insert(element.variables.end(), i->second.variables.begin(), i->second.variables.end());
            element.registerCount = i->first + i->second.count - element.startAddress;
        }
    }
    if(!empty) {
        holdingRegisterQueryList.push_back(move(element));
    }
    address2HoldingRegisterMap.clear();
}

pair<bool, string> get_reply(TcpSocket& socket, uint16_t expectedLength, chrono::milliseconds timeout) {
    string retVal;
    chrono::time_point<chrono::steady_clock> beginTime = chrono::steady_clock::now();
    do {
        this_thread::sleep_for(10ms);
        auto reply = socket.read();
        if(!reply.first) {
            return {false, retVal};
        }
        else {
            retVal += reply.second;
            if(retVal.size() >= expectedLength) {
                return {true, retVal};
            }
        }
    }while(chrono::steady_clock::now() - beginTime < timeout);
    return {false, retVal};
}

pair<bool, string> ModbusIpProcess::query_holding_register_then_get_reply(const HoldingRegisterQueryData& queryData, uint16_t sequenceNumber) {
    string retVal;
    auto query = ModbusIP::construct_read_multiple_holding_registers(sequenceNumber, config.slaveAddress, queryData.startAddress, queryData.registerCount);
    if(!socket.write(query.first)) {
        return {false, retVal};
    }
    auto beginTime = chrono::steady_clock::now();
    //chrono::duration<int, std::milli> theDuration;
    do {
        this_thread::sleep_for(10ms);
        auto reply = socket.read();
        if(!reply.first) {
            return {false, retVal};
        }
        else {
            retVal += reply.second;
            if(retVal.size() >= query.second) {
                return {true, retVal};
            }
        }
    }while(chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - beginTime) < 200ms);
    return {false, retVal};
}

//return false if socket got problem
bool ModbusIpProcess::do_write_coil_query(uint16_t& sequenceNumber) {
    while(1) {
        uint16_t coilAddress;
        bool value;
        {
            lock_guard<mutex> lock(forceCoilData.mapMutex);
            auto entry = forceCoilData.valueMap.begin();
            if(entry == forceCoilData.valueMap.end()) {
                return true;
            }
            coilAddress = entry->first;
            value = entry->second;
            forceCoilData.valueMap.erase(entry);
        }
        ++sequenceNumber;
        auto writeData = ModbusIP::construct_write_single_coil(sequenceNumber, config.slaveAddress, coilAddress, value);
        if(!socket.write(writeData.first)) {
            lock_guard<mutex> lock(forceCoilData.mapMutex);
            //forceCoilData.valueMap.emplace(coilAddress, value);
            forceCoilData.valueMap.clear(); //Socket error, dunno when can connect back, discard all pending write
            return false;
        }
        auto beginTime = chrono::steady_clock::now();
        string reply;
        //chrono::duration<int, std::milli> delay(milliSecond);
        do {
            this_thread::sleep_for(10ms);
            auto replyData = socket.read();
            if(!replyData.first) {
                return false;
            }
            reply += replyData.second;
        }while(reply.size() < writeData.second && chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - beginTime) < 200ms);
    }
}

bool ModbusIpProcess::do_write_holding_register_query(uint16_t& sequenceNumber) {
    while(1) {
        uint16_t registerAddress;
        std::vector<RegisterValue> values;
        {
            lock_guard<mutex> lock(writeHoldingRegisterData.mapMutex);
            auto entry = writeHoldingRegisterData.valueMap.begin();
            if(entry == writeHoldingRegisterData.valueMap.end()) {
                return true;
            }
            registerAddress = entry->first;
            values = move(entry->second);
            writeHoldingRegisterData.valueMap.erase(entry);
        }
        ++sequenceNumber;
        //cout<<"Writing to holding registers\n";
        auto writeData = ModbusIP::construct_write_multiple_holding_registers(sequenceNumber, config.slaveAddress, registerAddress, values);
        for(unsigned i = 0; i < writeData.first.size(); ++i) {
            printf("[%03u]", uint8_t(writeData.first[i]));
        }
        printf("\n");
        if(!socket.write(writeData.first)) {
            lock_guard<mutex> lock(writeHoldingRegisterData.mapMutex);
            writeHoldingRegisterData.valueMap.clear();
            return false;
        }
        auto beginTime = chrono::steady_clock::now();
        string reply;
        do {
            this_thread::sleep_for(10ms);
            auto replyData = socket.read();
            if(!replyData.first) {
                return false;
            }
            reply += replyData.second;
        }while(reply.size() < writeData.second && chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - beginTime) < 200ms);
    }
}

void ModbusIpProcess::thread_process(ModbusIpProcess* me) {
    printf("Thread started...\n");
    uint16_t sequenceNumber = 0;
    auto coilIter = me->coilQueryList.begin();
    auto registerIter = me->holdingRegisterQueryList.begin();
    while(me->threadData.keepRunning) {
        switch(me->threadData.mainState) {
            case 0: //Connect
                if(me->socket.open()) {
                    me->threadData.mainState = 1;
                }
                else {
                    this_thread::sleep_for(20ms);
                }
                break;
            case 1: //wait connection established
                //cout<<"State 1\n";
                if(me->socket.connection_established()) {
                    cout<<"ModbusIp Connected.\n";
                    me->threadData.mainState = 10;
                    me->threadData.subState = 0;
                }
                else {
                    cout<<"ModbusIp Cannot connect.\n";
                    this_thread::sleep_for(10s);
                }
                break;
            case 10:    //Routine operation
                //cout<<"State 10\n";
                //Do write coil if got any
                if(!me->do_write_coil_query(sequenceNumber)) {
                    me->threadData.mainState = 20; //close socket
                    continue;
                }
                if(!me->do_write_holding_register_query(sequenceNumber)) {
                    me->threadData.mainState = 20; //close socket
                    continue;
                }
                //Do write register if got any
                switch(me->threadData.subState) {
                    case 0: //Query Coil
                        //cout<<"SubState 0\n";
                        if(coilIter == me->coilQueryList.end()) {
                            coilIter = me->coilQueryList.begin();
                            me->threadData.subState = 1;    //Query Register
                        }
                        else {
                            CoilQueryData& queryData = (*coilIter);
                            ++sequenceNumber;
                            auto query = ModbusIP::construct_read_coils(sequenceNumber, me->config.slaveAddress, queryData.startAddress, queryData.coilCount);
                            if(!me->socket.write(query.first)) {
                                printf("Read coil failed!\n");
                                me->threadData.mainState = 20; //close socket
                                continue;
                            }
                            else {
                                auto reply = get_reply(me->socket, query.second, me->config.timeout);
                                if(!reply.first){
                                    printf("No read coil reply.\n");
                                    me->threadData.mainState = 20;  //close socket
                                    continue;
                                }
                                /*
                                printf("reply:");
                                for(unsigned i = 0; i < reply.second.size(); ++i) {
                                    printf("[%02X]", reply.second[i]);
                                }
                                printf("\n");
                                */
                                auto decoded = ModbusIP::decode_reply(reply.second);
                                if(decoded.functionCode == ModbusIP::READ_COIL_CODE && decoded.sequenceNumber == sequenceNumber) {
                                    //distribute the reply.
                                    //printf("Reply is valid.\n");
                                    for(auto i = queryData.variables.begin(); i != queryData.variables.end(); ++i) {
                                        (*i)->update_value_from_source(queryData.startAddress, decoded.get_coils());
                                    }
                                }
                                //this_thread::sleep_for(10s);
                            }
                            ++coilIter;
                        }
                        break;
                    case 1: //Query Register
                        //cout<<"SubState 1\n";
                        if(registerIter == me->holdingRegisterQueryList.end()){
                            registerIter = me->holdingRegisterQueryList.begin();
                            me->threadData.subState = 0;    //Query Coil
                        }
                        else {
                            HoldingRegisterQueryData& queryData = (*registerIter);
                            ++sequenceNumber;
                            auto reply = me->query_holding_register_then_get_reply(queryData, sequenceNumber);
                            if(!reply.first) {
                                printf("No read register reply.\n");
                                me->threadData.mainState = 20;  //close socket
                                continue;
                            }
                            auto decoded = ModbusIP::decode_reply(reply.second);
                            if(decoded.functionCode == ModbusIP::READ_HOLDING_REGISTER_CODE && decoded.sequenceNumber == sequenceNumber) {
                                //distribute the reply
                                for(auto i = queryData.variables.begin(); i != queryData.variables.end(); ++i) {
                                    (*i)->update_value_from_source(queryData.startAddress, decoded.get_holding_register());
                                }
                            }
                            //this_thread::sleep_for(10s);
                            ++registerIter;
                        }
                        break;
                }
                break;
            case 20:    //Close socket
            default:
                //cout<<"State 20\n";
                cout<<"ModbusIp close socket.\n";
                me->socket.close();
                me->threadData.mainState = 0;
                this_thread::sleep_for(1s);
                break;
        }
        this_thread::sleep_for(100ms);
    }
    printf("Thread stop.\n");
}

void ModbusIpProcess::force_coil(uint16_t coilAddress, bool value) {
    lock_guard<mutex> lock(forceCoilData.mapMutex);
    forceCoilData.valueMap[coilAddress] = value;
}

void ModbusIpProcess::write_multiple_holding_register(uint16_t registerAddress, const std::vector<RegisterValue>& values) {
    lock_guard<mutex> lock(writeHoldingRegisterData.mapMutex);
    writeHoldingRegisterData.valueMap[registerAddress] = values;
}


ModbusIpProcess::CoilStatusVariable::CoilStatusVariable(shared_ptr<Shadow> _master, uint16_t _coilAddress) : master(_master), coilAddress(_coilAddress) {

}

ModbusIpProcess::CoilStatusVariable::~CoilStatusVariable() {

}

void ModbusIpProcess::CoilStatusVariable::_write_value(const Value& newValue) {
    auto shared = master.lock();
    if(shared != nullptr) {
        shared->force_coil(coilAddress, (bool) newValue.get_int());
    }
}

void ModbusIpProcess::CoilStatusVariable::update_value_from_source(uint16_t firstAddress, const vector<bool>& values) {
	size_t index = coilAddress - firstAddress;
    //printf("My address %u, taking the %uth value.\n", coilAddress, index);
	if (index >= values.size())
	{
		return;
	}
    this->update_value_to_cache(values[index]);
}

ModbusIpProcess::HoldingRegisterVariable::HoldingRegisterVariable(
    shared_ptr<Shadow> _master, uint16_t _firstAddress, ModbusRegisterValue::DataType _type, bool _smallEndian) : master(_master) {
    firstAddress = _firstAddress;
    type = _type;
    smallEndian = _smallEndian;
}

ModbusIpProcess::HoldingRegisterVariable::~HoldingRegisterVariable() {

}

void ModbusIpProcess::HoldingRegisterVariable::_write_value(const Value& newValue) {
	ModbusRegisterValue setValue(type, smallEndian);
	std::vector<RegisterValue> converted = setValue.convert_to_register_value(newValue);
    auto shared = master.lock();
    if(shared != nullptr) {
        shared->write_multiple_holding_register(firstAddress, converted);
    }
}

void ModbusIpProcess::HoldingRegisterVariable::update_value_from_source(uint16_t _registerAddress, const vector<RegisterValue>& values) {
    //cout<<"Holding register updating value.\n";
	size_t index = firstAddress - _registerAddress;
	if (index >= values.size())
	{
		return;
	}
	ModbusRegisterValue modbusValue(type, smallEndian);
    modbusValue.set_register_value(values, index);
    //cout<<"Register setting value to "<<modbusValue.get_value().to_string()<<endl;
    this->update_value_to_cache(modbusValue.get_value());
}
