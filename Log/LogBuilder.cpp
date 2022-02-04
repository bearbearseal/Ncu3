#include "LogBuilder.h"

using namespace std;

vector<PointLogger::StartPattern> construct_start_pattern(vector<pair<optional<uint8_t>, optional<uint8_t>>> patterns)
{
    vector<PointLogger::StartPattern> retVal;
    for(size_t i=0; i<patterns.size(); ++i)
    {
        PointLogger::StartPattern entry;
        if(patterns[i].first.has_value())
        {
            entry.hour = patterns[i].first.value();
        }
        if(patterns[i].second.has_value())
        {
            entry.minute = patterns[i].second.value();
        }
        retVal.push_back(entry);
    }
    return retVal;
}

void LogBuilder::populate_log_points(PointLogger& pointLogger, ConfigStorage& configStorage)
{
    auto logConfig = configStorage.get_point_reading_log();
    printf("Log config got %zu entries.\n", logConfig.size());
    for(size_t i=0; i<logConfig.size(); ++i)
    {
        auto patternList = construct_start_pattern(logConfig[i].patterns);
        printf("device %u point %u rate %u\n", logConfig[i].device, logConfig[i].point, logConfig[i].rate*60);
        pointLogger.add_point(logConfig[i].device, logConfig[i].point, logConfig[i].rate*60, patternList);
    }
}
