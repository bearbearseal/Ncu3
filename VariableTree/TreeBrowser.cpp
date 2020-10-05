#include "TreeBrowser.h"
#include "../Basic/RamVariable.h"
#include <thread>

using namespace std;

TreeBrowser::TreeBrowser(std::shared_ptr<VariableTree> _variableTree, std::shared_ptr<Listener> listener) {
	treeRoot = _variableTree;
	cursor = treeRoot;
	myListener = listener;
	myShadow = make_shared<MyShadow>(*this);
}

TreeBrowser::~TreeBrowser() {
	lock_guard<mutex> lock(valueListeners.theMutex);
	valueListeners.data.clear();
	weak_ptr<MyShadow> _myShadow = myShadow;
	myShadow.reset();
	while (_myShadow.lock() != nullptr) {
		this_thread::yield();
	}
}

void TreeBrowser::forward_to_listener(const std::string& message) {
	auto shared = myListener.lock();
	if (shared != nullptr) {
		shared->catch_message(message);
	}
}

string TreeBrowser::process_command(const string& input) {
	nlohmann::json theReply;
	nlohmann::json theJson;
	printf("Parsing Message: %s\n", input.c_str());
	try
	{
		theJson = nlohmann::json::parse(input);
	}
	catch (nlohmann::json::parse_error)
	{
		theReply["Status"] = "Bad";
		theReply["Message"] = "Parse error";
		return theReply.dump() + '\n';
	}
	if (!theJson.is_object()) {
		theReply["Status"] = "Bad";
		theReply["Message"] = "Frame is not a json object";
		return theReply.dump() + '\n';
	}
	if (!theJson.contains("Command")) {
		theReply["Status"] = "Bad";
		theReply["Message"] = "No command";
		return theReply.dump() + '\n';
	}
	nlohmann::json& jCommand = theJson["Command"];
	if (!jCommand.is_string()) {
		theReply["Status"] = "Bad";
		theReply["Message"] = "Unknown command type";
		return theReply.dump() + '\n';
	}
	std::string command = jCommand.get<string>();
	if (!command.compare(COMMAND_BackToTop)) {
		return process_command_back_to_top();
	}
	else if (!command.compare(COMMAND_ShowBranches)) {
		return process_command_show_branches();
	}
	else if (!command.compare(COMMAND_MoveToBranch)) {
		return process_command_move_to_branch(theJson);
	}
	else if (!command.compare(COMMAND_ListenBranch)) {
		return process_command_subsribe_to_branch_event(theJson);
	}
	else if (!command.compare(COMMAND_ListenToValue)) {
		return process_command_subscribe_to_value_event(theJson);
	}
	else if (!command.compare(COMMAND_ReadValue)) {
		return process_command_read_value(theJson);
	}
	else if (!command.compare(COMMAND_WriteValue)) {
		return process_command_write_value(theJson);
	}
	else if (!command.compare(COMMAND_SetValue)) {
		return process_command_set_value(theJson);
	}
	else if (!command.compare(COMMAND_CreateBranch)) {
		return process_command_create_branch(theJson);
	}
	else if (!command.compare(COMMAND_CreateLeaf)) {
		return process_command_create_leaf(theJson);
	}
	else if (!command.compare(COMMAND_DeleteChild)) {
		return process_command_delete_child(theJson);
	}
	theReply["Status"] = "Bad";
	theReply["Message"] = "Unknown command";
	return theReply.dump() + '\n';
}

string TreeBrowser::process_command_back_to_top() {
	nlohmann::json retVal;
	cursor = treeRoot;
	retVal["Status"] = "Good";
	return retVal.dump() + '\n';
}

shared_ptr<VariableTree> recursive_move_to_branch(const nlohmann::json& jData, shared_ptr<VariableTree> cursor) {
	if (!jData.contains("Id") && !jData.contains("Branch")) {
		return cursor;
	}
	if (jData.contains("Id")) {
		const nlohmann::json& id = jData["Id"];
		HashKey::EitherKey key;
		if (id.is_number_integer()) {
			key = id.get<int>();
		}
		else if (id.is_string()) {
			key = id.get<string>();
		}
		else {
			return nullptr;
		}
		cursor = cursor->get_child(key);
	}
	if (cursor == nullptr) {
		return nullptr;
	}
	if (!jData.contains("Branch")) {
		return cursor;
	}
	else {
		const nlohmann::json& jSubData = jData["Branch"];
		if (!jSubData.is_object()) {
			printf("Branch is not object.\n");
			return nullptr;
		}
		return recursive_move_to_branch(jSubData, cursor);
	}
}

string TreeBrowser::process_command_move_to_branch(const nlohmann::json& jData) {
	nlohmann::json retVal;
	auto shared = cursor.lock();
	if (shared == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch deleted";
		return retVal.dump() + '\n';
	}
	auto target = recursive_move_to_branch(jData, shared);
	if (target == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch not found";
		return retVal.dump() + '\n';
	}
	cursor = target;
	retVal["Status"] = "Good";
	return retVal.dump() + '\n';
}

string TreeBrowser::process_command_show_branches() {
	nlohmann::json retVal;
	auto shared = cursor.lock();
	if (shared == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch deleted";
		return retVal.dump() + '\n';
	}
	if (shared->isLeaf) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "This is a leaf";
		return retVal.dump() + '\n';
	}
	retVal["Status"] = "Good";
	auto result = shared->list_all_children();
	nlohmann::json& jBranches = retVal["Branches"];
	jBranches.array();
	for (auto i : result) {
		nlohmann::json entry;
		if (i.first.is_integer()) {
			entry["Id"] = i.first.get_integer();
		}
		else {
			entry["Id"] = i.first.get_string();
		}
		if (i.second) {
			entry["Type"] = "Leaf";
		}
		else {
			entry["Type"] = "Branch";
		}
		jBranches.push_back(entry);
	}
	return retVal.dump() + '\n';
}

string TreeBrowser::process_command_subsribe_to_branch_event(const nlohmann::json& jData) {
	nlohmann::json retVal;
	auto shared = cursor.lock();
	if (shared == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch deleted";
		return retVal.dump() + '\n';
	}
	if (!jData.contains("Id")) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Subscription needs an Id";
		return retVal.dump() + '\n';
	}
	HashKey::EitherKey id;
	const nlohmann::json& jId = jData["Id"];
	if (jId.is_string()) {
		id = HashKey::EitherKey(jId.get<string>());
	}
	else if (jId.is_number_integer()) {
		id = HashKey::EitherKey(jId.get<int>());
	}
	else {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Invalid id type";
		return retVal.dump() + '\n';
	}
	auto target = recursive_move_to_branch(jData, shared);
	if (target == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch not found";
		return retVal.dump() + '\n';
	}
	{
		lock_guard<mutex> lock(branchListeners.theMutex);
		//if the node is already subscribed, only change the id
		auto i = branchListeners.data.find(target.get());
		if (i != branchListeners.data.end()) {
			i->second->change_id(id);
		}
		auto newListener = make_shared<BranchListener>(id);
		branchListeners.data.insert({ target.get(), newListener });
		target->add_add_remove_listener(newListener);
	}
	retVal["Status"] = "Good";
	return retVal.dump() + '\n';
}

string TreeBrowser::process_command_subscribe_to_value_event(const nlohmann::json& jData) {
	nlohmann::json retVal;

	auto shared = cursor.lock();
	if (shared == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch deleted";
		return retVal.dump() + '\n';
	}
	if (!jData.contains("Tag")) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Subscription needs a Tag";
		return retVal.dump() + '\n';
	}
	HashKey::EitherKey tag;
	const nlohmann::json& jTag = jData["Tag"];
	if (jTag.is_string()) {
		tag = HashKey::EitherKey(jTag.get<string>());
	}
	else if (jTag.is_number_integer()) {
		tag = HashKey::EitherKey(jTag.get<int>());
	}
	else {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Invalid Tag type";
		return retVal.dump() + '\n';
	}
	auto target = recursive_move_to_branch(jData, shared);
	if (target == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch not found";
		return retVal.dump() + '\n';
	}
	{
		lock_guard<mutex> lock(valueListeners.theMutex);
		//if the node is already subscribed, only change the id
		auto i = valueListeners.data.find(target.get());
		if (i != valueListeners.data.end()) {
			i->second->change_id(tag);
		}
		else {
			auto newListener = make_shared<ValueListener>(myShadow, tag);
			valueListeners.data.insert({ target.get(), newListener });
			target->add_value_change_listener(newListener);
		}
	}
	retVal["Status"] = "Good";
	return retVal.dump() + '\n';
}

string TreeBrowser::process_command_read_value(const nlohmann::json& jData) {
	nlohmann::json retVal;
	auto shared = cursor.lock();
	if (shared == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch deleted";
		return retVal.dump() + '\n';
	}
	auto target = recursive_move_to_branch(jData, shared);
	if (target == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch not found";
		return retVal.dump() + '\n';
	}
	if (!target->isLeaf) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "This is not a leaf";
		return retVal.dump() + '\n';
	}
	Value value = target->get_value();
	retVal["Status"] = "Good";
	if (value.is_numeric()) {
		retVal["Value"] = value.get_float();
	}
	else {
		retVal["Value"] = value.get_string();
	}
	return retVal.dump() + '\n';
}

std::string TreeBrowser::process_command_set_value(const nlohmann::json& jData) {
	nlohmann::json retVal;
	auto shared = cursor.lock();
	if (shared == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch deleted";
		return retVal.dump() + '\n';
	}
	if (!jData.contains("Value")) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Write needs value";
		return retVal.dump() + '\n';
	}
	const nlohmann::json& jValue = jData["Value"];
	Value value;
	if (jValue.is_number_float()) {
		value = jValue.get<double>();
	}
	else if (jValue.is_number_integer()) {
		value = jValue.get<int64_t>();
	}
	else if (jValue.is_number_unsigned()) {
		value = jValue.get<uint64_t>();
	}
	else if (jValue.is_string()) {
		value = jValue.get<string>();
	}
	else {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Invalid value type";
		return retVal.dump() + '\n';
	}
	auto target = recursive_move_to_branch(jData, shared);
	if (target == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch not found";
		return retVal.dump() + '\n';
	}
	if (!target->isLeaf) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Target is not a leaf";
		return retVal.dump() + '\n';
	}
	if (target->set_value(value)) {
		retVal["Status"] = "Good";
	}
	else {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Set value rejected";
	}
	retVal["Status"] = "Good";
	return retVal.dump() + '\n';
}

string TreeBrowser::process_command_write_value(const nlohmann::json& jData) {
	nlohmann::json retVal;
	auto shared = cursor.lock();
	if (shared == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch deleted";
		return retVal.dump() + '\n';
	}
	if (!jData.contains("Value")) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Write needs value";
		return retVal.dump() + '\n';
	}
	const nlohmann::json& jValue = jData["Value"];
	Value value;
	if (jValue.is_number_float()) {
		value = jValue.get<double>();
	}
	else if (jValue.is_number_integer()) {
		value = jValue.get<int64_t>();
	}
	else if (jValue.is_number_unsigned()) {
		value = jValue.get<uint64_t>();
	}
	else if (jValue.is_string()) {
		value = jValue.get<string>();
	}
	else {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Invalid value type";
		return retVal.dump() + '\n';
	}
	auto target = recursive_move_to_branch(jData, shared);
	if (target == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch not found";
		return retVal.dump() + '\n';
	}
	if (!target->isLeaf) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Target is not a leaf";
		return retVal.dump() + '\n';
	}
	if (target->write_value(value)) {
		retVal["Status"] = "Good";
	}
	else {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Set value rejected";
	}
	retVal["Status"] = "Good";
	return retVal.dump() + '\n';
}

std::string TreeBrowser::process_command_create_branch(const nlohmann::json& jData) {
	nlohmann::json retVal;
	auto shared = cursor.lock();
	if (shared == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch deleted";
		return retVal.dump() + '\n';
	}
	auto target = recursive_move_to_branch(jData, shared);
	if (target == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch not found";
		return retVal.dump() + '\n';
	}
	/*
	if (!target->isLeaf) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Target is a leaf";
		return retVal.dump() + '\n';
	}
	*/
	if (!jData.contains("NewId")) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "CreateBranch needs NewId";
		return retVal.dump() + '\n';
	}
	const nlohmann::json& newId = jData["NewId"];
	HashKey::EitherKey key;
	if (newId.is_number_integer()) {
		key = newId.get<int>();
	}
	else if (newId.is_string()) {
		key = newId.get<string>();
	}
	auto result = target->create_branch(key);
	if (result == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Failed during creation";
		return retVal.dump() + '\n';
	}
	retVal["Status"] = "Good";
	return retVal.dump() + '\n';
}

std::string TreeBrowser::process_command_create_leaf(const nlohmann::json& jData) {
	nlohmann::json retVal;
	auto shared = cursor.lock();
	if (shared == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch deleted";
		return retVal.dump() + '\n';
	}
	auto target = recursive_move_to_branch(jData, shared);
	if (target == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch not found";
		return retVal.dump() + '\n';
	}
	/*
	if (!target->isLeaf) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Target is a leaf";
		return retVal.dump() + '\n';
	}
	*/
	if (!jData.contains("NewId")) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "CreateLeaf needs NewId";
		return retVal.dump() + '\n';
	}
	const nlohmann::json& newId = jData["NewId"];
	HashKey::EitherKey key;
	if (newId.is_number_integer()) {
		key = newId.get<int>();
	}
	else if (newId.is_string()) {
		key = newId.get<string>();
	}
	auto result = target->create_leaf(key, make_shared<RamVariable>());
	if (result == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Failed during creation";
		return retVal.dump() + '\n';
	}
	retVal["Status"] = "Good";
	return retVal.dump() + '\n';
}

std::string TreeBrowser::process_command_delete_child(const nlohmann::json& jData) {
	nlohmann::json retVal;
	auto shared = cursor.lock();
	if (shared == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch deleted";
		return retVal.dump() + '\n';
	}
	auto target = recursive_move_to_branch(jData, shared);
	if (target == nullptr) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Branch not found";
		return retVal.dump() + '\n';
	}
	if (!target->isLeaf) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "Target is a leaf";
		return retVal.dump() + '\n';
	}
	if (!jData.contains("ChildId")) {
		retVal["Status"] = "Bad";
		retVal["Message"] = "DeleteChild needs ChildId";
		return retVal.dump() + '\n';
	}
	const nlohmann::json& newId = jData["ChildId"];
	HashKey::EitherKey key;
	if (newId.is_number_integer()) {
		key = newId.get<int>();
	}
	else if (newId.is_string()) {
		key = newId.get<string>();
	}
	if (!target->remove_child(key)) {
		retVal["Status"] = "Good";
		retVal["Message"] = "Error while removing child";
		return retVal.dump();
	}
	retVal["Status"] = "Good";
	return retVal.dump() + '\n';
}

TreeBrowser::BranchListener::BranchListener(const HashKey::EitherKey& newId) {
	id = newId;
}

TreeBrowser::BranchListener::~BranchListener() {

}

void TreeBrowser::BranchListener::change_id(const HashKey::EitherKey& newId) {
	id = newId;
}

HashKey::EitherKey TreeBrowser::BranchListener::read_id() {
	return id;
}

TreeBrowser::ValueListener::ValueListener(std::shared_ptr<MyShadow> _master, const HashKey::EitherKey& _tagId) {
	master = _master;
	tagId = _tagId;
}

TreeBrowser::ValueListener::~ValueListener() {

}

void TreeBrowser::ValueListener::change_id(const HashKey::EitherKey& newId) {
	tagId = newId;
}

HashKey::EitherKey TreeBrowser::ValueListener::read_id() {
	return tagId;
}

void print_branch(const vector<HashKey::EitherKey>& theVector) {
	for (const HashKey::EitherKey& key : theVector) {
		printf("=>{%s}", key.to_string().c_str());
	}
	printf("\n");
}

void print_branch(vector<HashKey::EitherKey>::const_iterator i, vector<HashKey::EitherKey>::const_iterator end) {
	if (i != end) {
		printf("{%s}=>", i->to_string().c_str());
		print_branch(++i, end);
	}
	else {
		printf("{}\n");
	}
}

nlohmann::json make_json_branch(vector<HashKey::EitherKey>::const_reverse_iterator i, vector<HashKey::EitherKey>::const_reverse_iterator end) {
	nlohmann::json retVal;
	if (i->is_integer()) {
		retVal["Id"] = i->get_integer();
	}
	else {
		retVal["Id"] = i->get_string();
	}
	++i;
	if (i != end) {
		retVal["Branch"] = make_json_branch(i, end);
	}
	return retVal;
}

void TreeBrowser::ValueListener::catch_value_change_event(const vector<HashKey::EitherKey>& branch, const Value& newValue, chrono::time_point<chrono::system_clock> theMoment) {
	//printf("Caught value change, tag Id: %s, newValue: %s.\n", tagId.to_string().c_str(), newValue.to_string().c_str());
	//print_branch(branch);
	auto shared = master.lock();
	if (shared != nullptr) {
		nlohmann::json retVal;
		retVal["Command"] = "ValueUpdate";
		if (newValue.is_float()) {
			retVal["Value"] = newValue.get_float();
		}
		else if (newValue.is_integer()) {
			retVal["Value"] = newValue.get_int();
		}
		else if (newValue.is_string()) {
			retVal["Value"] = newValue.get_string();
		}
		if (branch.size()) {
			retVal["Branch"] = make_json_branch(branch.rbegin(), branch.rend());
		}
		if (tagId.is_string()) {
			retVal["Tag"] = tagId.get_string();
		}
		else {
			retVal["Tag"] = tagId.get_integer();
		}
		shared->forward_to_listener(retVal.dump() + '\n');
	}
}
