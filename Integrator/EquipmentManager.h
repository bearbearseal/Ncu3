#ifndef _EquipmentBuilder_H_
#define _EquipmentBuilder_H_

#include <memory>
#include <unordered_map>

#include "../Storage/ConfigStorage.h"
#include "ChannelManager.h"
#include "../Equipment/Equipment.h"
#include "../InOutOperation/OpStorage.h"
#include "../VariableTree/VariableTree.h"


//Equipment manager group points into equipments
class EquipmentManager {
public:
    EquipmentManager(ConfigStorage& configStorage, ChannelManager& channelManager, OpStorage& opStorage);
    virtual ~EquipmentManager();

    //attachId = true to attach tree with Id, attachName = true to attacj tree with name
    void attach_equipments(std::shared_ptr<VariableTree> theTree, bool attachId = true, bool attachName = true);

private:
    std::unordered_map<uint16_t, std::shared_ptr<Equipment>> id2EquipmentMap;
    std::unordered_map<std::string, std::shared_ptr<Equipment>> name2EquipmentMap;
};

#endif