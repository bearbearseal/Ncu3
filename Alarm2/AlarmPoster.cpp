#include "AlarmPoster.h"

using namespace std;

AlarmPoster::AlarmPoster()
{
    threadSocket = move(itc.create_fixed_socket(1, 2));
    masterSocket = move(itc.create_fixed_socket(2, 1));
}

AlarmPoster::~AlarmPoster()
{

}

void AlarmPoster::catch_alarm(uint32_t equipmentId, uint32_t pointId, const Value &theValue, AlarmDefinition::AlarmState state,
                     const Value &refValue, AlarmDefinition::Comparison compare, const chrono::time_point<chrono::system_clock> theMoment)
{

}

void AlarmPoster::thread_process(void* threadData)
{
    
}
