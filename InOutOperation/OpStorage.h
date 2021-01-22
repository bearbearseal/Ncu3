#ifndef _OpStorage_H_
#define _OpStorage_H_
#include <string>
#include <memory>
#include <unordered_map>
#include "../../MyLib/Sqlite/Sqlite3.h"
#include "../../MyLib/File/FileIOer.h"
#include "OpCreator.h"

//Load from sqlite and create logics accordingly
//Get logic by id
class OpStorage {
public:
    OpStorage(const std::string& sqliteFile, const std::string& logicFolder);
    ~OpStorage();

    std::shared_ptr<OperationalLogic> get_logic(size_t logicId);

private:
    OpCreator creator;

    const std::string logicFolder;
    std::unordered_map<int64_t, std::shared_ptr<OperationalLogic>> logicMap;
    std::unordered_map<int64_t, std::string> id2FileMap;
};

#endif