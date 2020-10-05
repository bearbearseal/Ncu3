#include <iostream>
#include "AlarmStorage.h"
#include "../Basic/DataConverter.h"

using namespace std;

AlarmStorage::AlarmStorage(const std::string& dbName) {
    theDb = make_unique<Sqlite3>(dbName);
    auto result = move(theDb->execute_query("Select Equipment, Property, Type, Code, Id from ActiveAlarm"));
    for(size_t i = 0; i < result->get_row_count(); ++i) {
        ConditionPriorId& element = conditionMap[result->get_string(i, 0).second][result->get_string(i, 1).second];
        element.condition.type = AlarmDefinition::ConditionType(result->get_integer(i, 2).second);
        element.condition.code = result->get_integer(i, 3).second;
        element.id = result->get_integer(i, 4).second;
    }
}

AlarmStorage::~AlarmStorage() {

}

//Only method that would lock both mutexes
size_t AlarmStorage::store_alarm(const AlarmDefinition::AlarmMessage& alarmMessage) {
    //store to db
    uint64_t priorId;
    auto i = conditionMap.find(alarmMessage.equipment);
    if(i == conditionMap.end()) {
        priorId = 0;
    }
    else {
        auto j = i->second.find(alarmMessage.source);
        if(j == i->second.end()) {
            priorId = 0;
        }
        else {
            priorId = j->second.id;
        }
    }
    string queryString = "Insert into UnreportedAlarm (PriorId, Equipment, Property, Value, TimeMilliSec, Message, Type, Code)Values(";
    queryString += to_string(priorId) + ",'";
    queryString += alarmMessage.equipment.get_string() + "','";
    queryString += alarmMessage.source.get_string() + "',";
    if(alarmMessage.leftValue.is_empty()) {
        queryString += "0,";
    }
    else {
        queryString += alarmMessage.leftValue.get_string() + ",";
    }
    queryString += to_string(DataConverter::ChronoSystemTime(alarmMessage.theMoment).to_milli_second()) + ",";
    queryString += "'"+alarmMessage.message+"',";
    queryString += to_string(size_t(alarmMessage.condition.type)) + ",";
    queryString += to_string(alarmMessage.condition.code) + ");";
    queryString += "SELECT Last_Insert_Rowid();";
    cout<<"SQL statement: "<<queryString;
    uint64_t newId;
    auto result = move(theDb->execute_query(queryString));
    newId = result->get_integer(0, 0).second;
    //Replace to Active alarm db and map
    string updateString = "Insert or replace into ActiveAlarm (Equipment, Property, Type, Code, Id) values('";
    updateString += alarmMessage.equipment.get_string() + "', '";
    updateString += alarmMessage.source.get_string() + "', ";
    updateString += to_string(size_t(alarmMessage.condition.type)) + ", ";
    updateString += to_string(alarmMessage.condition.code) + ",";
    updateString += to_string(newId) + ");";
    theDb->execute_update(updateString);
    ConditionPriorId& entry = conditionMap[alarmMessage.equipment][alarmMessage.source];
    entry.condition = alarmMessage.condition;
    entry.id = newId;
    return newId;
}

unordered_map<HashKey::EitherKey, AlarmDefinition::Condition, HashKey::EitherKey> AlarmStorage::get_active_condition(const HashKey::EitherKey& equipmentId) {
    unordered_map<HashKey::EitherKey, AlarmDefinition::Condition, HashKey::EitherKey> retVal;
    auto i = conditionMap.find(equipmentId);
    if(i == conditionMap.end()) {
        cout<<"No conditions, Equipment: "<<equipmentId.to_string()<<endl;
        return retVal;
    }
    for(auto j = i->second.begin(); j != i->second.end(); ++j) {
        retVal.emplace(j->first, j->second.condition);
    }
    return retVal;
}

/*
std::vector<AlarmStorage::PropertyCondition> AlarmStorage::get_property_condition() const {
    vector<PropertyCondition> retVal;
    for(auto i = conditionMap.begin(); i != conditionMap.end(); ++i) {
        for(auto j = i->second.begin(); j != i->second.end(); ++j) {
            retVal.push_back({i->first, j->first, j->second.condition});
        }
    }
    return retVal;
}
*/
list<AlarmStorage::UnreportedAlarm> AlarmStorage::get_unreported_alarms(size_t count) const {
    list<AlarmStorage::UnreportedAlarm> retVal;
    string queryString = "Select Id, PriorId, Equipment, Property, Value, TimeMilliSec, Message, Type, Code from UnreportedAlarm limit ";
    queryString += to_string(count);
    auto result = move(theDb->execute_query(queryString));
    for(auto i = 0; i < result->get_row_count(); ++i) {
        AlarmStorage::UnreportedAlarm element;
        element.id = result->get_integer(i, "Id").second;
        element.priorId = result->get_integer(i, "PriorId").second;
        element.equipment = result->get_string(i, "Equipment").second;
        element.property = result->get_string(i, "Property").second;
        element.value = result->get_float(i, "Value").second;
        element.timeMilliSec = result->get_integer(i, "TimeMilliSec").second;
        element.message = result->get_string(i, "Message").second;
        element.type = result->get_integer(i, "Type").second;
        element.code = result->get_integer(i, "Code").second;
        retVal.push_back(element);
    }
    return retVal;
}

void AlarmStorage::remove_unreported_alarm(uint64_t id) {
    string updateString = "Delete from UnreportedAlarm where id=";
    updateString += to_string(id);
    theDb->execute_update(updateString);
}

void AlarmStorage::remove_unreported_alarms(std::vector<uint64_t> id) {
    for(uint64_t singleId : id) {
        string updateString = "Delete from UnreportedAlarm where id=";
        updateString += to_string(singleId);
        theDb->execute_update(updateString);
    }
}