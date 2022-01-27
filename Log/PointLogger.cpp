#include <chrono>
#include <ctime>
#include "PointLogger.h"
#include "../../MyLib/Timer/EventTimer.h"

using namespace std;

uint8_t get_second_from_time(time_t theTime)
{
    return uint8_t(theTime % 60);
}

uint8_t get_minute_from_time(time_t theTime)
{
    uint64_t totalMin = theTime / 60;
    return uint8_t(totalMin % 60);
}

uint8_t get_hour_from_time(time_t theTime)
{
    uint64_t totalHour = theTime / 3600;
    totalHour+=8;
    return uint8_t(totalHour % 24);
}

PointLogger::PointLogger(shared_ptr<VariableTree> _root, PointLogStorage& _storage) : storage(_storage)
{
    EventTimer::start();
    root = _root;
    timerListener = make_shared<TimerListener>(*this);
}

PointLogger::~PointLogger()
{
    weak_ptr<TimerListener> weak = timerListener;
    timerListener.reset();
    while(!weak.expired())
    {
        this_thread::sleep_for(1ms);
    }
}

uint32_t create_device_point_fusion(uint16_t device, uint16_t point)
{
    uint32_t retVal = device;
    retVal <<= 16;
    retVal += point;
    return retVal;
}

time_t get_next_start_time(time_t todaySec, time_t nowSec, const PointLogger::StartPattern &startPattern)
{
    tm timeStruct;
    localtime_r(&nowSec, &timeStruct);
    // Get the hour and minute
    if (startPattern.hour.has_value() && startPattern.minute.has_value())
    {
        if (startPattern.hour.value() > timeStruct.tm_hour ||
            (startPattern.hour.value() == timeStruct.tm_hour && startPattern.minute.value() > timeStruct.tm_min))
        {
            // next start time is today later
            timeStruct.tm_hour = startPattern.hour.value();
            timeStruct.tm_min = startPattern.minute.value();
            timeStruct.tm_sec = 0;
            printf("Next start time is :%02u%02u\n", timeStruct.tm_hour, timeStruct.tm_min);
            return timelocal(&timeStruct);
        }
        else
        {
            // next start time is tomorrow
            timeStruct.tm_hour = startPattern.hour.value();
            timeStruct.tm_min = startPattern.minute.value();
            timeStruct.tm_sec = 0;
            printf("Next start time is :%02u%02u tomorrow\n", timeStruct.tm_hour, timeStruct.tm_min);
            return timelocal(&timeStruct) + (24 * 3600);
        }
    }
    else if (startPattern.minute.has_value())
    {
        if (timeStruct.tm_min < startPattern.minute.value())
        {
            // Set start this hour later.
            printf("Next start time is :%02u%02u\n", timeStruct.tm_hour, startPattern.minute.value());
            timeStruct.tm_min = startPattern.minute.value();
            timeStruct.tm_sec = 0;
            return timelocal(&timeStruct);
        }
        else
        {
            // next start time is next hour
            printf("Next start time is :%02u%02u\n", timeStruct.tm_hour + 1, startPattern.minute.value());
            timeStruct.tm_min = startPattern.minute.value();
            timeStruct.tm_sec = 0;
            return timelocal(&timeStruct) + 3600;
        }
    }
    // Start immediately
    return nowSec;
}

void PointLogger::add_point(uint16_t device, uint16_t point, uint32_t secInterval, const std::vector<StartPattern> &startPatternList)
{
    auto theDevice = root->get_child(device);
    if (theDevice == nullptr)
    {
        return;
    }
    auto thePoint = theDevice->get_child(point);
    if (thePoint == nullptr)
    {
        return;
    }
    time_t startTime;
    // Get current time
    auto thisMoment = chrono::system_clock::now();
    time_t nowSec = chrono::system_clock::to_time_t(thisMoment);
    if (!startPatternList.empty())
    {
        time_t todaySec = nowSec - (nowSec % (24 * 3600));
        // Arrange the start pattern
        std::set<time_t> startTimeQueue;
        for (size_t i = 0; i < startPatternList.size(); ++i)
        {
            time_t nextStartSec = get_next_start_time(todaySec, nowSec, startPatternList[i]);
            // Put it into queue map
            startTimeQueue.emplace(nextStartSec);
        }
        // first element of startTimeQueue is the next start time.
        startTime = *(startTimeQueue.begin());
    }
    else
    {
        // start time
        startTime = nowSec;
    }
    uint32_t fusion = create_device_point_fusion(device, point);
    pointMap.insert({fusion, {device, point, secInterval, thePoint}});
    printf("Adding point=>%02u:%02u:%02u\n", get_hour_from_time(startTime), get_minute_from_time(startTime), get_second_from_time(startTime));
    EventTimer::add_time_event(startTime, timerListener, fusion, secInterval);
}

void PointLogger::handle_timer_event(time_t eventTime, uint32_t token)
{
    // Do the log action
    PointData &theData = pointMap[token];
    {
        printf("Handling event time=>%02u:%02u:%02u\n", get_hour_from_time(eventTime), get_minute_from_time(eventTime), get_second_from_time(eventTime));
        time_t nextTime = eventTime + theData.secInterval;
        printf("Next similar event=>%02u:%02u:%02u\n", get_hour_from_time(nextTime), get_minute_from_time(nextTime), get_second_from_time(nextTime));
        printf("Device: %u Point: %u\n", theData.device, theData.point);
        auto value = theData.valuePoint->get_value();
        printf("Value: %s\n", value.to_string().c_str());
        storage.store_record(PointLogStorage::RecordData{theData.device, theData.point, eventTime, value});
    }
}

void PointLogger::TimerListener::catch_time_event(time_t eventTime, uint32_t token)
{
    // Would it call some mutex lock again?
    master.handle_timer_event(eventTime, token);
    // timer.add_time_event(eventTime + interval, token);
}
