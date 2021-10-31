#ifndef _AlarmTalker_H_
#define _AlarmTalker_H_
#include "../../OtherLib/nlohmann/json.hpp"
#include "../../MyLib/TcpSocket/TcpListener.h"
#include "AlarmDefinition.h"
#include "AlarmStore.h"
#include "AlarmListener.h"

class AlarmTalker : public TcpListener
{
private:
    const std::string COMMAND_ReadActiveAlarm = "ReadActiveAlarm";
    const std::string COMMAND_ReadHistoryAlarm = "ReadHistoryAlarm";
    const std::string FIELD_Command = "Command";
    const std::string FIELD_Continuation = "Continuation";
    const std::string FIELD_Count = "Limit";
    const std::string FIELD_Begin = "BeginTime";
    const std::string FIELD_End = "EndTime";
    const std::string FIELD_Alarm = "Alarm";
    const std::string FIELD_Sequence = "Sequence";

    class Shadow
    {
    public:
        Shadow(AlarmTalker &_real) : real(_real) {}
        inline void send_alarm(const std::string &alarm)
        {
            real.broadcast(alarm);
        }

    private:
        AlarmTalker &real;
    };
    class _AlarmListener : public AlarmListener
    {
    public:
        _AlarmListener(std::weak_ptr<Shadow> _master) : master(_master) {}
        // AlarmListener method
        void catch_alarm(uint64_t alarmId, const std::unique_ptr<AlarmDefinition::AlarmMessage> &alarmMessage);

    private:
        std::weak_ptr<Shadow> master;
    };

public:
    AlarmTalker(uint16_t portNum, std::shared_ptr<AlarmStore> _alarmStore);
    virtual ~AlarmTalker();

protected:
    // TcpListener method
    void catch_message(std::string &data, size_t handle);

private:
    void alarm_data_to_json(nlohmann::json &dest, AlarmStore::ReadAlarmData data);
    std::string handle_read_active_alarm();

    std::shared_ptr<Shadow> myShadow;
    std::shared_ptr<AlarmStore> alarmStore;
};

#endif