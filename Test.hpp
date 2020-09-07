//#include "TreeTemplate.h"
#include "VariableTree/VariableTree.h"
#include "Basic/RamVariable.h"
#include "VariableTree/TcpTalker.h"
#include "Modbus/ModbusIpProcess.h"
#include <thread>
#include <stdlib.h>
#include <list>

using namespace std;
class BranchListener : public VariableTree::AddRemoveListener {
public:
	BranchListener(const string& _myId) : myId(_myId) {}
	~BranchListener() {}

	void catch_add_child_event(const std::vector<HashKey::EitherKey>& branch, const HashKey::EitherKey& newChild) {
		printf("%s caught add child event:\n\t", myId.c_str());
		for (auto i = branch.rbegin(); i != branch.rend(); ++i) {
			printf("{%s}=>", i->to_string().c_str());
		}
		printf("[%s]\n", newChild.to_string().c_str());
	}

	void catch_remove_child_event(const std::vector<HashKey::EitherKey>& branch, const HashKey::EitherKey& lostChild) {
		printf("%s caught remove child event:\n\t", myId.c_str());
		for (auto i = branch.rbegin(); i != branch.rend(); ++i) {
			printf("{%s}=>", i->to_string().c_str());
		}
		printf("[%s]\n", lostChild.to_string().c_str());
	}

private:
	const string myId;
};

class ValueListener : public VariableTree::ValueChangeListener {
public:
	ValueListener(const string& _myId) : myId(_myId) {}
	~ValueListener() {}

	void catch_value_change_event(const std::vector<HashKey::EitherKey>& branches, const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment) {
		printf("%s caught value changed event:\n\t", myId.c_str());
		for (auto i = branches.rbegin(); i != branches.rend(); ++i) {
			printf("{%s}=>", i->to_string().c_str());
		}
		printf("[%s]\n", newValue.to_string().c_str());
	}

private:
	const string myId;
};

namespace Test {
	void variable_tree() {
		VariableTree variableTree;
		std::shared_ptr<RamVariable> variable1 = make_shared<RamVariable>();
		variableTree.create_leaf(123, variable1);
		for (unsigned i = 0; i < 5; ++i) {
			variable1->write_value(i * 3);
			std::this_thread::sleep_for(1s);
			printf("Loop %u\n", i);
		}
	}

	void run_Variable_Tree() {
		auto branchListener = make_shared<BranchListener>("Listener_A");
		auto valueListener = make_shared<ValueListener>("Listener_B");
		auto subBranchListener = make_shared<BranchListener>("Listener_C");
		auto subValueListener = make_shared<ValueListener>("Listener_D");
		auto subValueListener2 = make_shared<ValueListener>("Listener_E");
		VariableTree variableTree;
		variableTree.add_add_remove_listener(branchListener);
		variableTree.add_value_change_listener(valueListener);

		variableTree.create_branch("100");
		variableTree.create_branch("101");
		variableTree.create_branch(102);
		variableTree.create_branch(103);
		variableTree.create_leaf("104", make_shared<RamVariable>());
		variableTree.create_leaf(105, make_shared<RamVariable>());
		variableTree.get_child(105)->add_value_change_listener(subValueListener);

		auto aBranch = variableTree.get_child("100");
		aBranch->create_leaf(200, make_shared<RamVariable>());
		aBranch = aBranch->create_branch("201");
		aBranch->create_leaf(300, make_shared<RamVariable>());

		aBranch = variableTree.get_child("101");
		aBranch->create_leaf("202", make_shared<RamVariable>());
		aBranch = aBranch->create_branch(203);
		aBranch->create_leaf("301", make_shared<RamVariable>());

		aBranch = variableTree.get_child(102);
		aBranch->create_leaf(204, make_shared<RamVariable>());
		aBranch->create_leaf("205", make_shared<RamVariable>());

		aBranch = variableTree.get_child(103);
		auto anotherBranch = aBranch->create_branch(206);
		anotherBranch->create_leaf(302, make_shared<RamVariable>());
		anotherBranch->create_leaf("303", make_shared<RamVariable>());
		anotherBranch->create_leaf(304, make_shared<RamVariable>());
		anotherBranch = aBranch->create_branch("207");
		anotherBranch->add_add_remove_listener(subBranchListener);
		anotherBranch->create_leaf("305", make_shared<RamVariable>());

		anotherBranch->create_leaf(306, make_shared<RamVariable>());
		anotherBranch->create_leaf("307", make_shared<RamVariable>());
		anotherBranch = anotherBranch->create_branch(308);
		anotherBranch->add_value_change_listener(subValueListener2);
		anotherBranch = anotherBranch->create_branch("400");
		anotherBranch->create_leaf(500, make_shared<RamVariable>());

		printf("Before:\n");
		printf("s-100=>i-200: %s\n", variableTree.get_child("100")->get_child(200)->get_value().to_string().c_str());
		printf("s-100=>s-201=>i-300: %s\n", variableTree.get_child("100")->get_child("201")->get_child(300)->get_value().to_string().c_str());
		printf("s-101=>s-202: %s\n", variableTree.get_child("101")->get_child("202")->get_value().to_string().c_str());
		printf("s-101=>i-203=>s-301: %s\n", variableTree.get_child("101")->get_child(203)->get_child("301")->get_value().to_string().c_str());
		printf("i-102=>i-204: %s\n", variableTree.get_child(102)->get_child(204)->get_value().to_string().c_str());
		printf("i-102=>s-205: %s\n", variableTree.get_child(102)->get_child("205")->get_value().to_string().c_str());
		printf("i-103=>i-206=>i-302: %s\n", variableTree.get_child(103)->get_child(206)->get_child(302)->get_value().to_string().c_str());
		printf("i-103=>i-206=>s-303: %s\n", variableTree.get_child(103)->get_child(206)->get_child("303")->get_value().to_string().c_str());
		printf("i-103=>i-206=>i-304: %s\n", variableTree.get_child(103)->get_child(206)->get_child(304)->get_value().to_string().c_str());
		printf("s-103=>s-207=>s-305: %s\n", variableTree.get_child(103)->get_child("207")->get_child("305")->get_value().to_string().c_str());
		printf("s-103=>s-207=>i-306: %s\n", variableTree.get_child(103)->get_child("207")->get_child(306)->get_value().to_string().c_str());
		printf("s-103=>s-207=>s-307: %s\n", variableTree.get_child(103)->get_child("207")->get_child("307")->get_value().to_string().c_str());
		printf("s-104: %s\n", variableTree.get_child("104")->get_value().to_string().c_str());
		printf("i-105: %s\n", variableTree.get_child(105)->get_value().to_string().c_str());

		variableTree.get_child("100")->get_child(200)->set_value("102");
		variableTree.get_child("100")->get_child("201")->get_child(300)->set_value(103);
		variableTree.get_child("101")->get_child("202")->set_value(104.0);
		variableTree.get_child("101")->get_child(203)->get_child("301")->set_value("105");
		variableTree.get_child(102)->get_child(204)->set_value(106);
		variableTree.get_child(102)->get_child("205")->set_value(107.0);
		variableTree.get_child(103)->get_child(206)->get_child(302)->set_value("108");
		variableTree.get_child(103)->get_child(206)->get_child("303")->set_value(109);
		variableTree.get_child(103)->get_child(206)->get_child(304)->set_value(110.0);
		variableTree.get_child(103)->get_child("207")->get_child("305")->set_value("111");
		variableTree.get_child(103)->get_child("207")->get_child(306)->set_value(112);
		variableTree.get_child(103)->get_child("207")->get_child("307")->set_value(113.0);
		variableTree.get_child("104")->set_value(100);
		variableTree.get_child(105)->set_value(101.0);
		variableTree.get_child(103)->get_child("207")->get_child(308)->get_child("400")->get_child(500)->set_value("abcdefg");

		anotherBranch->remove_child(500);

		printf("\nAfter:\n");
		printf("s-100=>i-200: %s\n", variableTree.get_child("100")->get_child(200)->get_value().to_string().c_str());
		printf("s-100=>s-201=>i-300: %s\n", variableTree.get_child("100")->get_child("201")->get_child(300)->get_value().to_string().c_str());
		printf("s-101=>s-202: %s\n", variableTree.get_child("101")->get_child("202")->get_value().to_string().c_str());
		printf("s-101=>i-203=>s-301: %s\n", variableTree.get_child("101")->get_child(203)->get_child("301")->get_value().to_string().c_str());
		printf("i-102=>i-204: %s\n", variableTree.get_child(102)->get_child(204)->get_value().to_string().c_str());
		printf("i-102=>s-205: %s\n", variableTree.get_child(102)->get_child("205")->get_value().to_string().c_str());
		printf("i-103=>i-206=>i-302: %s\n", variableTree.get_child(103)->get_child(206)->get_child(302)->get_value().to_string().c_str());
		printf("i-103=>i-206=>s-303: %s\n", variableTree.get_child(103)->get_child(206)->get_child("303")->get_value().to_string().c_str());
		printf("i-103=>i-206=>i-304: %s\n", variableTree.get_child(103)->get_child(206)->get_child(304)->get_value().to_string().c_str());
		printf("s-103=>s-207=>s-305: %s\n", variableTree.get_child(103)->get_child("207")->get_child("305")->get_value().to_string().c_str());
		printf("s-103=>s-207=>i-306: %s\n", variableTree.get_child(103)->get_child("207")->get_child(306)->get_value().to_string().c_str());
		printf("s-103=>s-207=>s-307: %s\n", variableTree.get_child(103)->get_child("207")->get_child("307")->get_value().to_string().c_str());
		printf("s-104: %s\n", variableTree.get_child("104")->get_value().to_string().c_str());
		printf("i-105: %s\n", variableTree.get_child(105)->get_value().to_string().c_str());

		printf("\nError Test:\n");
		printf("s-abc: %p\n", variableTree.get_child("abc").get());
		printf("s-100: %s\n", variableTree.get_child("100")->get_value().to_string().c_str());
	}

	void run_tree_browser() {
		shared_ptr<VariableTree> variableTree = make_shared<VariableTree>();

		variableTree->create_branch("100");
		variableTree->create_branch("101");
		variableTree->create_branch(102);
		variableTree->create_branch(103);
		variableTree->create_leaf("104", make_shared<RamVariable>());
		variableTree->create_leaf(105, make_shared<RamVariable>());

		auto aBranch = variableTree->get_child("100");
		aBranch->create_leaf(200, make_shared<RamVariable>());
		aBranch = aBranch->create_branch("201");
		aBranch->create_leaf(300, make_shared<RamVariable>());

		aBranch = variableTree->get_child("101");
		aBranch->create_leaf("202", make_shared<RamVariable>());
		aBranch = aBranch->create_branch(203);
		aBranch->create_leaf("301", make_shared<RamVariable>());

		aBranch = variableTree->get_child(102);
		aBranch->create_leaf(204, make_shared<RamVariable>());
		aBranch->create_leaf("205", make_shared<RamVariable>());

		aBranch = variableTree->get_child(103);
		auto anotherBranch = aBranch->create_branch(206);
		anotherBranch->create_leaf(302, make_shared<RamVariable>());
		anotherBranch->create_leaf("303", make_shared<RamVariable>());
		anotherBranch->create_leaf(304, make_shared<RamVariable>());
		anotherBranch = aBranch->create_branch("207");
		anotherBranch->create_leaf("305", make_shared<RamVariable>());

		anotherBranch->create_leaf(306, make_shared<RamVariable>());
		anotherBranch->create_leaf("307", make_shared<RamVariable>());
		anotherBranch = anotherBranch->create_branch(308);
		anotherBranch = anotherBranch->create_branch("400");
		anotherBranch->create_leaf(500, make_shared<RamVariable>());

		TreeBrowser treeBrowser(variableTree);
		variableTree->get_child("104")->set_value(344);
		printf("%s\n", treeBrowser.process_command("{\"Command\":\"Show\"}").c_str());
		printf("%s\n", treeBrowser.process_command("{\"Command\":\"MoveDown\", \"Id\":\"100\"}").c_str());
		printf("%s\n", treeBrowser.process_command("{\"Command\":\"Read\"}").c_str());
		printf("%s\n", treeBrowser.process_command("{\"Command\":\"BackToTop\"}").c_str());
		printf("%s\n", treeBrowser.process_command("{\"Command\":\"MoveDown\", \"Id\":\"101\"}").c_str());
		printf("%s\n", treeBrowser.process_command("{\"Command\":\"Read\"}").c_str());
		printf("%s\n", treeBrowser.process_command("{\"Command\":\"BackToTop\"}").c_str());
		printf("%s\n", treeBrowser.process_command("{\"Command\":\"MoveDown\", \"Id\":\"104\"}").c_str());
		printf("%s\n", treeBrowser.process_command("{\"Command\":\"Read\"}").c_str());
	}

	//This test 1st populate the tree with some data.
	//Then creates a TcpTalker which takes the tree
	//Then runs the TcpTalker
	void run_tcp_talker() {
		printf("tcp talker running.\n");
		shared_ptr<VariableTree> variableTree = make_shared<VariableTree>();

		variableTree->create_branch("100");
		variableTree->create_branch("101");
		variableTree->create_branch(102);
		variableTree->create_branch(103);
		variableTree->create_leaf("104", make_shared<RamVariable>());
		variableTree->create_leaf(105, make_shared<RamVariable>());

		auto aBranch = variableTree->get_child("100");
		aBranch->create_leaf(200, make_shared<RamVariable>());
		aBranch = aBranch->create_branch("201");
		aBranch->create_leaf(300, make_shared<RamVariable>());

		aBranch = variableTree->get_child("101");
		aBranch->create_leaf("202", make_shared<RamVariable>());
		aBranch = aBranch->create_branch(203);
		aBranch->create_leaf("301", make_shared<RamVariable>());

		aBranch = variableTree->get_child(102);
		aBranch->create_leaf(204, make_shared<RamVariable>());
		aBranch->create_leaf("205", make_shared<RamVariable>());

		aBranch = variableTree->get_child(103);
		auto anotherBranch = aBranch->create_branch(206);
		anotherBranch->create_leaf(302, make_shared<RamVariable>());
		anotherBranch->create_leaf("303", make_shared<RamVariable>());
		anotherBranch->create_leaf(304, make_shared<RamVariable>());
		anotherBranch = aBranch->create_branch("207");
		anotherBranch->create_leaf("305", make_shared<RamVariable>());

		anotherBranch->create_leaf(306, make_shared<RamVariable>());
		anotherBranch->create_leaf("307", make_shared<RamVariable>());
		anotherBranch = anotherBranch->create_branch(308);
		anotherBranch = anotherBranch->create_branch("400");
		anotherBranch->create_leaf(500, make_shared<RamVariable>());

		variableTree->get_child("104")->set_value(344);

		printf("Creating tcp talker.\n");
		TcpTalker tcpTalker(56789);
		tcpTalker.set_target(variableTree);
		tcpTalker.start();
		printf("Starting tcp talker.\n");
		//shared_ptr<ValueListener> aValueListener = make_shared<ValueListener>("ValueListener1");
		//variableTree->add_value_change_listener(aValueListener);
		while (1) {
			this_thread::sleep_for(1s);
		}
	}

	void run_tcp_talker_and_modbus() {
		unique_ptr<ModbusIpProcess> modbusIp1 = make_unique<ModbusIpProcess>("127.0.0.1", 502, 1, 16, 64, true, std::chrono::milliseconds(100));
		shared_ptr<Variable> coil1 = modbusIp1->get_coil_status_variable(1);
		shared_ptr<Variable> coil2 = modbusIp1->get_coil_status_variable(2);
		shared_ptr<Variable> coil10 = modbusIp1->get_coil_status_variable(10);
		shared_ptr<Variable> register1 = modbusIp1->get_holding_register_variable(1,ModbusRegisterValue::DataType::INT16);
		shared_ptr<Variable> register10 = modbusIp1->get_holding_register_variable(10,ModbusRegisterValue::DataType::INT32_LM);
		shared_ptr<Variable> register20 = modbusIp1->get_holding_register_variable(20,ModbusRegisterValue::DataType::INT32_ML);

		shared_ptr<VariableTree> variableTree = make_shared<VariableTree>();
		auto device1 = variableTree->create_branch("Device1");
		auto device2 = variableTree->create_branch("Device2");
		device1->create_leaf("coil1", coil1);
		device1->create_leaf("coil2", coil2);
		device1->create_leaf("coil10", coil10);
		device2->create_leaf("register1", register1);
		device2->create_leaf("register10", register10);
		device2->create_leaf("register20", register20);

		modbusIp1->start();
		while(1) {
			this_thread::sleep_for(1s);
		}
	}
}