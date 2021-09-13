#include "EquipmentManager.h"

using namespace std;

EquipmentManager::EquipmentManager(ConfigStorage &configStorage, ChannelManager &channelManager, OpStorage &opStorage)
{
    auto idAndName = configStorage.get_equipment_data();
    for (auto i = idAndName.begin(); i != idAndName.end(); ++i)
    {
        id2EquipmentMap[i->equipmentId] = make_shared<Equipment>();
        name2EquipmentMap[i->name] = id2EquipmentMap[i->equipmentId];
    }
    auto propertyData = configStorage.get_property_data();
    for (auto i = propertyData.begin(); i != propertyData.end(); ++i)
    {
        if (!id2EquipmentMap.count(i->first))
        {
            id2EquipmentMap[i->first] = make_shared<Equipment>();
        }
        //printf("Number: %lu\n", i->second.size());
        for (size_t j = 0; j < i->second.size(); ++j)
        {
            //printf("Getting %u %u\n", i->second[j].deviceId, i->second[j].pointId);
            auto point = channelManager.get_point(i->second[j].deviceId, i->second[j].pointId);
            if (point != nullptr)
            {
                //printf("Creating leaf: %s\n", i->second[j].name.c_str());
                id2EquipmentMap[i->first]->create_leaf(i->second[j].name, point, opStorage.get_logic(i->second[j].inOperation), opStorage.get_logic(i->second[j].outOperation));
            }
        }
    }
}

EquipmentManager::~EquipmentManager()
{
}

void EquipmentManager::attach_equipments(std::shared_ptr<VariableTree> theTree, bool attachId, bool attachName)
{
    if (attachId)
    {
        for (auto i = id2EquipmentMap.begin(); i != id2EquipmentMap.end(); ++i)
        {
            theTree->add_child(i->first, i->second);
        }
        id2EquipmentMap.clear();
    }
    if (attachName)
    {
        for (auto i = name2EquipmentMap.begin(); i != name2EquipmentMap.end(); ++i)
        {
            theTree->add_child(i->first, i->second);
        }
        name2EquipmentMap.clear();
    }
}
