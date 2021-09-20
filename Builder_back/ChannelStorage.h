#ifndef _ChannelStorage_H_
#define _ChannelStorage_H_
#include "../MyLib/Sqlite/Sqlite3.h"
#include <string>
#include <memory>
#include <vector>

class ChannelStorage {
public:
    ChannelStorage(std::unique_ptr<Sqlite3> theDb);
    ~ChannelStorage();

    struct ChannelData {
        uint16_t channelId;
        uint16_t channelType;
    };

    struct 
    std::vector<ChannelData> get_channel_data();
    
private:
};

#endif