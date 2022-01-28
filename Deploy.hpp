#include "VariableTree/VariableTree.h"
#include "VariableTree/TcpTalker.h"
#include "Storage/ConfigStorage.h"
#include "Integrator/ChannelManager.h"
#include "Integrator/SerialPortManager.h"
#include "InOutOperation/OpStorage.h"
#include "Integrator/Integrator.h"
#include "Alarm/AlarmVerifyStorePost.h"
#include "Alarm/AlarmTalker.h"
#include "Log/PointLogStorage.h"

#include <thread>

using namespace std;

namespace Deploy {
   void run_equipment_alarm() {
        ConfigStorage configData("/var/sqlite/NcuConfig.db");
        Integrator integrator(configData);
        SerialPortManager serialPortManager(configData);
        OpStorage opStorage("/var/sqlite/NcuConfig.db", "/var/InOutOp");
        ChannelManager channelManager(configData, serialPortManager, opStorage);

        shared_ptr<VariableTree> root = make_shared<VariableTree>();
        channelManager.attach_to_tree(root);

        shared_ptr<AlarmVerifyStorePost> alarmVerifyStorePost = make_shared<AlarmVerifyStorePost>("/var/sqlite/NcuAlarm.db");
        AlarmTalker alarmTalker(10521, alarmVerifyStorePost);
        AlarmProcessor alamProcessor(configData, root, alarmVerifyStorePost);
        alarmTalker.start();

        ScheduleManager scheduleManager(configData);
        integrator.join_schedule_and_points(scheduleManager, root);
        scheduleManager.start();

        TcpTalker tcpTalker(10520);
        tcpTalker.set_target(root);
        tcpTalker.start();
        channelManager.start();
        while(1) {
            this_thread::sleep_for(1s);
        }
   }

    void run_equipment_alarm_schedule_log()
    {
        
    }
};