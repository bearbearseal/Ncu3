#include "PointLogger.h"
#include "../VariableTree/VariableTree.h"
#include "../../MyLib/Basic/RamVariable.h"
#include "../VariableTree/TcpTalker.h"
#include "../../MyLib/Timer/EventTimer.h"
#include <memory>

using namespace std;

int main()
{
    PointLogStorage logStorage("/var/sqlite/PointLog.db");

    shared_ptr<VariableTree> root = make_shared<VariableTree>();
    auto branch1 = root->create_branch(1);
    branch1->create_leaf(1, make_shared<RamVariable>());
    branch1->create_leaf(2, make_shared<RamVariable>());
    branch1->create_leaf(3, make_shared<RamVariable>());
    branch1->create_leaf(4, make_shared<RamVariable>());
    auto branch2 = root->create_branch(2);
    branch2->create_leaf(1, make_shared<RamVariable>());
    branch2->create_leaf(2, make_shared<RamVariable>());
    branch2->create_leaf(3, make_shared<RamVariable>());
    branch2->create_leaf(4, make_shared<RamVariable>());
    auto branch3 = root->create_branch(3);
    branch3->create_leaf(1, make_shared<RamVariable>());
    branch3->create_leaf(2, make_shared<RamVariable>());
    branch3->create_leaf(3, make_shared<RamVariable>());
    branch3->create_leaf(4, make_shared<RamVariable>());
    auto branch4 = root->create_branch(4);
    branch4->create_leaf(1, make_shared<RamVariable>());
    branch4->create_leaf(2, make_shared<RamVariable>());
    branch4->create_leaf(3, make_shared<RamVariable>());
    branch4->create_leaf(4, make_shared<RamVariable>());

    vector<PointLogger::StartPattern> patternList1;
    patternList1.push_back({{}, 0});
    patternList1.push_back({{}, 15});
    patternList1.push_back({{}, 30});
    patternList1.push_back({{}, 45});
    vector<PointLogger::StartPattern> patternList2;
    patternList2.push_back({0, 0});
    patternList2.push_back({6, 0});
    patternList2.push_back({12, 0});
    patternList2.push_back({24, 0});
    vector<PointLogger::StartPattern> patternList3;
    patternList3.push_back({11, 11});

    PointLogger pointLogger(root, logStorage);
    pointLogger.add_point(1, 1, 60, patternList1);
    //pointLogger.add_point(1, 2, 120, patternList2);
    pointLogger.add_point(1, 3, 60, {});
    //pointLogger.add_point(1, 4, 120, {});
    pointLogger.add_point(2, 1, 60, patternList3);
    //pointLogger.add_point(2, 2, 120, patternList1);
    //pointLogger.add_point(2, 3, 60, {});
    //pointLogger.add_point(2, 4, 120, {});
    pointLogger.add_point(3, 1, 60, patternList2);
    //pointLogger.add_point(3, 2, 120, patternList3);
    //pointLogger.add_point(3, 3, 60, {});
    //pointLogger.add_point(3, 4, 120, {});
    //pointLogger.add_point(4, 1, 60, patternList1);
    //pointLogger.add_point(4, 2, 120, patternList2);
    //pointLogger.add_point(4, 3, 60, {});
    //pointLogger.add_point(4, 4, 120, {});

    TcpTalker tcpTalker(10520);
    tcpTalker.set_target(root);
    tcpTalker.start();
    printf("Starting.\n");
    while (1)
    {
        this_thread::sleep_for(1s);
    }
}