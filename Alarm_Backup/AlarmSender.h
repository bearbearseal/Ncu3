#ifndef _AlarmSender_H_
#define _AlarmSender_H_

class AlarmSender {
public:
    AlarmSender();
    virtual ~AlarmSender();

    virtual void send_alarm() = 0;
    virtual void verify_reply() = 0;
};

#endif