#include "AlarmLogicGroup.h"

using namespace std;

AlarmLogicGroup::AlarmLogicGroup(const std::vector<AlarmData>& logicData)
{
    for(size_t i=0; i<logicData.size(); ++i)
    {
        verifierList.push_back({AlarmVerifier(logicData[i].compare, logicData[i].refValue), logicData[i]});
    }
}

AlarmLogicGroup::~AlarmLogicGroup()
{

}

optional<AlarmLogicGroup::AlarmData> AlarmLogicGroup::check_alarm(const Value& value)
{
    for(size_t i=0; i<verifierList.size(); ++i)
    {
        if(verifierList[i].first.verify(value))
        {
            return verifierList[i].second;
        }
    }
    return {};
}
