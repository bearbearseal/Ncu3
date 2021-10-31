#include "AlarmLogicGroup.h"

using namespace std;

AlarmLogicGroup::AlarmLogicGroup(const std::vector<AlarmData>& logicData)
{
    for(size_t i=0; i<logicData.size(); ++i)
    {
        checkerList.push_back({AlarmStateChecker(logicData[i].compare, logicData[i].refValue), logicData[i]});
    }
}

AlarmLogicGroup::~AlarmLogicGroup()
{

}

AlarmLogicGroup::AlarmData AlarmLogicGroup::check_alarm(const Value& value) const
{
    for(size_t i=0; i<checkerList.size(); ++i)
    {
        if(checkerList[i].first.verify(value))
        {
            return checkerList[i].second;
        }
    }
    return {};
}
