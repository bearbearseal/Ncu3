#include "OpStorage.h"

using namespace std;

OpStorage::OpStorage(const std::string& sqliteFile, const std::string& _logicFolder) : logicFolder(_logicFolder)
{
    Sqlite3 theDb(sqliteFile);
    auto result = theDb.execute_query("Select Id, FilePath from InOutOperation");
    for(size_t i=0; i<result->get_row_count(); ++i) {
        int64_t id = result->get_integer(i, "Id").second;
        string filePath = result->get_string(i, "FilePath").second;
        id2FileMap.insert({id, move(filePath)});
    }
}

OpStorage::~OpStorage() {

}

shared_ptr<OperationalLogic> OpStorage::get_logic(size_t logicId) {
    if(!id2FileMap.count(logicId)) {
        return nullptr;
    }
    if(!logicMap.count(logicId)) {
        //Try to open the file
        string filename = logicFolder + "/" + id2FileMap[logicId];
        //Create the logic
        FileIOer aFile(filename);
        string content = aFile.read_data();
        creator.clear();
        logicMap.emplace(logicId, creator.load_logic(content));
    }
    return logicMap[logicId];
}

