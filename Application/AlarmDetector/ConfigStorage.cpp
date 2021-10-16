#include "ConfigStorage.h"

using namespace std;

ConfigStorage::ConfigStorage(const std::string& dbFilePath) : theDb(dbFilePath)
{

}

ConfigStorage::~ConfigStorage() 
{

}

unordered_map<uint32_t, ConfigStorage::NodeAlarmLogicData> ConfigStorage::get_node_alarm_logic_data()
{
    unordered_map<uint32_t, NodeAlarmLogicData> retVal;
    auto result = theDb.execute_query("Select Id, Compare, Value, WaitTime, Threshold, Category, Message, AlarmCode from NodeAlarmLogic");
    for(size_t i=0; i<result->get_row_count(); ++i)
    {
        NodeAlarmLogicData& entry = retVal[result->get_integer(i, "Id").second];
        entry.comparison = result->get_string(i, "Compare").second;
        entry.value = result->get_float(i, "Value").second;
        entry.msWaitTime = result->get_integer(i, "WaitTime").second;
        entry.threshold = result->get_float(i, "Threshold").second;
        entry.category = result->get_integer(i, "Category").second;
        entry.message = result->get_string(i, "Message").second;
        entry.code = result->get_integer(i, "AlarmCode").second;
    }
    return retVal;
}

vector<ConfigStorage::PropertyAlarmData> ConfigStorage::get_property_alarm_data()
{
    vector<PropertyAlarmData> retVal;
    auto result = theDb.execute_query("Select Equipment, Property, AlarmLogicId, Priority, Category, Message, AlarmCode from PropertyAlarm");
    retVal.resize(result->get_row_count());
    for(size_t i=0; i<result->get_row_count(); ++i)
    {
        PropertyAlarmData& entry = retVal[i];
        entry.equipmentId.from_formatted_string(result->get_string(i, "Equipment").second);
        entry.propertyId.from_formatted_string(result->get_string(i, "Property").second);
        entry.alarmId = result->get_integer(i, "AlarmLogicId").second;
        entry.priority = result->get_integer(i, "Priority").second;
        entry.category = result->get_integer(i, "Category").second;
        entry.message = result->get_string(i, "Message").second;
        entry.code = result->get_integer(i, "AlarmCode").second;
    }
    return retVal;
}
