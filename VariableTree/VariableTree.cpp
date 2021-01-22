#include "VariableTree.h"
#include <thread>

using namespace std;

const Value VariableTree::empty;
/*
VariableTree::VariableTree(bool _isLeaf) : isLeaf(_isLeaf) {
	toChildren = make_shared<Parent>(*this);
}
*/

VariableTree::VariableTree(bool _isLeaf) : isLeaf(_isLeaf) {
	toChildren = make_shared<Parent>(*this);
	branchData = make_unique<BranchData>();
}

VariableTree::VariableTree(const HashKey::EitherKey& _myId, shared_ptr<Parent> parentProxy, bool _isLeaf) : isLeaf(_isLeaf), myId(_myId) {
	fromParent = parentProxy;
	toChildren = make_shared<Parent>(*this);
	if (!isLeaf) {
		branchData = make_unique<BranchData>();
	}
	else {
		leafData = make_unique<LeafData>();
		variableListener = make_shared<VariableListener>(*this);
	}
}

VariableTree::~VariableTree() {
	weak_ptr<Parent> weak1 = toChildren;
	toChildren.reset();
	weak_ptr<VariableListener> weak2 = variableListener;
	variableListener.reset();
	while (weak1.lock() != nullptr) {
		this_thread::yield();
	}
	while (weak2.lock() != nullptr) {
		this_thread::yield();
	}
}

bool VariableTree::add_child(const HashKey::EitherKey& key, std::shared_ptr<VariableTree> newChild) {
	if (isLeaf) {
		return false;
	}
	{
		lock_guard<mutex> lock(branchData->dataMutex);
		if(branchData->dataMap.count(key)) {
			return false;
		}
		newChild->fromParent = toChildren;
		newChild->myId = key;
		branchData->dataMap[key] = newChild;
	}
	{
		lock_guard<mutex> listenerLock(branchData->addRemoveListenerMutex);
		for (auto i = branchData->addRemoveListeners.begin(); i != branchData->addRemoveListeners.end();) {
			auto shared = i->second.lock();
			if (shared == nullptr) {
				auto temp = i++;
				branchData->addRemoveListeners.erase(temp);
			}
			else {
				shared->catch_add_child_event(vector<HashKey::EitherKey>(), key);
				++i;
			}
		}
	}
	{
		vector<HashKey::EitherKey> branches;
		branches.push_back(myId);
		auto shared = fromParent.lock();
		if (shared != nullptr) {
			shared->catch_child_add_offspring(branches, key);
		}
	}
	return true;
}


shared_ptr<VariableTree> VariableTree::create_branch(const HashKey::EitherKey& key) {
	if (isLeaf) {
		return nullptr;
	}
	shared_ptr<VariableTree> retVal;
	{
		lock_guard<mutex> lock(branchData->dataMutex);
		if (branchData->dataMap.count(key)) {
			return nullptr;
		}
		retVal = make_shared<VariableTree>(key, toChildren, false);
		branchData->dataMap[key] = retVal;
	}
	{
		lock_guard<mutex> listenerLock(branchData->addRemoveListenerMutex);
		for (auto i = branchData->addRemoveListeners.begin(); i != branchData->addRemoveListeners.end();) {
			auto shared = i->second.lock();
			if (shared == nullptr) {
				auto temp = i++;
				branchData->addRemoveListeners.erase(temp);
			}
			else {
				shared->catch_add_child_event(vector<HashKey::EitherKey>(), key);
				++i;
			}
		}
	}
	{
		vector<HashKey::EitherKey> branches;
		branches.push_back(myId);
		auto shared = fromParent.lock();
		if (shared != nullptr) {
			shared->catch_child_add_offspring(branches, key);
		}
	}
	return retVal;
}

shared_ptr<VariableTree> VariableTree::create_leaf(const HashKey::EitherKey& key, std::shared_ptr<Variable> _variable) {
	if (isLeaf) {
		return nullptr;
	}
	shared_ptr<VariableTree> newLeaf;
	{
		lock_guard<mutex> lock(branchData->dataMutex);
		if (branchData->dataMap.count(key)) {
			return nullptr;
		}
		newLeaf = make_shared<VariableTree>(key, toChildren, true);
		newLeaf->leafData->variable = _variable;
		_variable->set_listener(newLeaf->variableListener);
		branchData->dataMap[key] = newLeaf;
	}
	{
		lock_guard<mutex> listenerLock(branchData->addRemoveListenerMutex);
		for (auto i = branchData->addRemoveListeners.begin(); i != branchData->addRemoveListeners.end();) {
			auto shared = i->second.lock();
			if (shared == nullptr) {
				auto temp = i++;
				branchData->addRemoveListeners.erase(temp);
			}
			else {
				shared->catch_add_child_event(vector<HashKey::EitherKey>(), key);
				++i;
			}
		}
	}
	{
		vector<HashKey::EitherKey> branches;
		branches.push_back(myId);
		auto shared = fromParent.lock();
		if (shared != nullptr) {
			shared->catch_child_add_offspring(branches, key);
		}
	}
	return newLeaf;
}

shared_ptr<VariableTree> VariableTree::force_create_branch(const HashKey::EitherKey& key) {
	if (isLeaf) {
		return nullptr;
	}
	shared_ptr<VariableTree> newBranch;
	{
		lock_guard<mutex> lock(branchData->dataMutex);
		newBranch = make_shared<VariableTree>(key, toChildren, false);
		branchData->dataMap[key] = newBranch;
	}
	{
		lock_guard<mutex> listenerLock(branchData->addRemoveListenerMutex);
		for (auto i = branchData->addRemoveListeners.begin(); i != branchData->addRemoveListeners.end();) {
			auto shared = i->second.lock();
			if (shared == nullptr) {
				auto temp = i++;
				branchData->addRemoveListeners.erase(temp);
			}
			else {
				shared->catch_add_child_event(vector<HashKey::EitherKey>(), key);
				++i;
			}
		}
	}
	{
		vector<HashKey::EitherKey> branches;
		branches.push_back(myId);
		auto shared = fromParent.lock();
		if (shared != nullptr) {
			shared->catch_child_add_offspring(branches, key);
		}
	}
	return newBranch;
}

bool VariableTree::remove_child(const HashKey::EitherKey& key) {
	if (isLeaf) {
		return false;
	}
	{
		lock_guard<mutex> lock(branchData->dataMutex);
		auto i = branchData->dataMap.find(key);
		if (i == branchData->dataMap.end()) {
			return false;
		}
		branchData->dataMap.erase(i);
	}
	{
		lock_guard<mutex> listenerLock(branchData->addRemoveListenerMutex);
		for (auto j = branchData->addRemoveListeners.begin(); j != branchData->addRemoveListeners.end();) {
			auto shared = j->second.lock();
			if (shared == nullptr) {
				auto temp = j++;
				branchData->addRemoveListeners.erase(temp);
			}
			else {
				shared->catch_remove_child_event(vector<HashKey::EitherKey>(), key);
				++j;
			}
		}
	}
	{
		vector<HashKey::EitherKey> branches;
		branches.push_back(myId);
		auto shared = fromParent.lock();
		if (shared != nullptr) {
			shared->catch_child_lost_offspring(branches, key);
		}
	}
	return true;
}

shared_ptr<VariableTree> VariableTree::force_create_leaf(const HashKey::EitherKey& key, shared_ptr<Variable> _variable) {
	if (isLeaf) {
		return nullptr;
	}
	shared_ptr<VariableTree> newLeaf;
	{
		lock_guard<mutex> lock(branchData->dataMutex);
		auto newLeaf = make_shared<VariableTree>(key, toChildren, true);
		newLeaf->leafData->variable = _variable;
		branchData->dataMap[key] = newLeaf;
	}
	{
		lock_guard<mutex> listenerLock(branchData->addRemoveListenerMutex);
		for (auto i = branchData->addRemoveListeners.begin(); i != branchData->addRemoveListeners.end();) {
			auto shared = i->second.lock();
			if (shared == nullptr) {
				auto temp = i++;
				branchData->addRemoveListeners.erase(temp);
			}
			else {
				shared->catch_add_child_event(vector<HashKey::EitherKey>(), key);
				++i;
			}
		}
	}
	{
		vector<HashKey::EitherKey> branches;
		branches.push_back(myId);
		auto shared = fromParent.lock();
		if (shared != nullptr) {
			shared->catch_child_add_offspring(branches, key);
		}
	}
	return newLeaf;
}

vector<pair<HashKey::EitherKey, bool>> VariableTree::list_all_children() const {
	vector<pair<HashKey::EitherKey, bool>> retVal;
	if (!isLeaf) {
		lock_guard<mutex> lock(branchData->dataMutex);
		for (auto i : branchData->dataMap) {
			retVal.push_back({ i.first , i.second->isLeaf});
		}
	}
	return retVal;
}

shared_ptr<VariableTree> VariableTree::get_child(const HashKey::EitherKey& key) const {
	if (isLeaf) {
		return nullptr;
	}
	lock_guard<mutex> lock(branchData->dataMutex);
	auto i = branchData->dataMap.find(key);
	if (i == branchData->dataMap.end()) {
		return nullptr;
	}
	return i->second;
}

bool VariableTree::write_value(const Value& value, uint8_t priority) {
	if(!isLeaf) {
		return false;
	}
	bool result = leafData->variable->write_value(value, priority);
	return result;
}

bool VariableTree::set_value(const Value& value, uint8_t priority) {
	if (!isLeaf) {
		return false;
	}
	//lock_guard<mutex> lock(dataMutex);
	bool result = leafData->variable->set_value(value, priority);
	/*
	if (!result) {
		return false;
	}
	auto theMoment = chrono::system_clock::now();
	{
		lock_guard<mutex> lock(leafData->listenerMutex);
		for (auto i = leafData->valueChangeListeners.begin(); i != leafData->valueChangeListeners.end();) {
			auto shared = i->second.lock();
			if (shared == nullptr) {
				auto temp = i++;
				leafData->valueChangeListeners.erase(temp);
			}
			else {
				shared->catch_value_change_event({ }, value, theMoment);
				++i;
			}
		}
	}
	{
		vector<HashKey::EitherKey> branches({ myId });
		auto shared = fromParent.lock();
		if (shared != nullptr) {
			shared->catch_child_value_change(branches, value, theMoment);
		}
	}
	*/
	return result;
}

Value VariableTree::get_value() const {
	if (!isLeaf) {
		return empty;
	}
	//lock_guard<mutex> lock(dataMutex);
	return leafData->variable->read_value();
}

void VariableTree::add_add_remove_listener(shared_ptr<AddRemoveListener> listener) {
	if (isLeaf) {
		return;
	}
	lock_guard<mutex> lock(branchData->addRemoveListenerMutex);
	branchData->addRemoveListeners[listener.get()] = listener;
}

void VariableTree::remove_add_remove_listener(shared_ptr<AddRemoveListener> listener) {
	if (isLeaf) {
		return;
	}
	lock_guard<mutex> lock(branchData->addRemoveListenerMutex);
	auto i = branchData->addRemoveListeners.find(listener.get());
	if (i != branchData->addRemoveListeners.end()) {
		branchData->addRemoveListeners.erase(i);
	}
}

void VariableTree::add_value_change_listener(shared_ptr<ValueChangeListener> listener) {
	if (isLeaf) {
		lock_guard<mutex> lock(leafData->listenerMutex);
		leafData->valueChangeListeners[listener.get()] = listener;
	}
	else {
		lock_guard<mutex> lock(branchData->valueChangeListenerMutex);
		branchData->valueChangeListeners[listener.get()] = listener;
	}
}

void VariableTree::remove_value_change_listener(shared_ptr<ValueChangeListener> listener) {
	if (isLeaf) {
		lock_guard<mutex> lock(leafData->listenerMutex);
		auto i = leafData->valueChangeListeners.find(listener.get());
		if (i != leafData->valueChangeListeners.end()) {
			leafData->valueChangeListeners.erase(i);
		}
	}
	else {
		lock_guard<mutex> lock(branchData->valueChangeListenerMutex);
		auto i = branchData->valueChangeListeners.find(listener.get());
		if (i != branchData->valueChangeListeners.end()) {
			branchData->valueChangeListeners.erase(i);
		}
	}
}

void VariableTree::catch_value_change(const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment) {
	//printf("Caught value change.\n");
	//Inform listener
	if (isLeaf) {
		lock_guard<mutex> lock(leafData->listenerMutex);
		for (auto i = leafData->valueChangeListeners.begin(); i != leafData->valueChangeListeners.end();) {
			auto shared = i->second.lock();
			if (shared != nullptr) {
				shared->catch_value_change_event(vector<HashKey::EitherKey>(), newValue, theMoment);
				++i;
			}
			else {
				auto temp = i;
				++i;
				leafData->valueChangeListeners.erase(temp);
			}
		}
	}
	//Inform parent
	auto shared = fromParent.lock();
	if (shared != nullptr) {
		vector<HashKey::EitherKey> keys;
		keys.push_back(myId);
		shared->catch_child_value_change(keys, newValue, theMoment);
	}
}

void VariableTree::catch_child_add_offspring(vector<HashKey::EitherKey>& branches, const HashKey::EitherKey& key) {
	{
		lock_guard<mutex> listenerLock(branchData->addRemoveListenerMutex);
		for (auto i = branchData->addRemoveListeners.begin(); i != branchData->addRemoveListeners.end();) {
			auto shared = i->second.lock();
			if (shared == nullptr) {
				auto temp = i++;
				branchData->addRemoveListeners.erase(temp);
			}
			else {
				shared->catch_add_child_event(branches, key);
				++i;
			}
		}
	}
	branches.push_back(myId);
	auto shared = fromParent.lock();
	if (shared != nullptr) {
		shared->catch_child_add_offspring(branches, key);
	}
}

void VariableTree::catch_child_lost_offspring(vector<HashKey::EitherKey>& branches, const HashKey::EitherKey& key) {
	{
		lock_guard<mutex> listenerLock(branchData->addRemoveListenerMutex);
		for (auto i = branchData->addRemoveListeners.begin(); i != branchData->addRemoveListeners.end();) {
			auto shared = i->second.lock();
			if (shared == nullptr) {
				auto temp = i++;
				branchData->addRemoveListeners.erase(temp);
			}
			else {
				shared->catch_remove_child_event(branches, key);
				++i;
			}
		}
	}
	branches.push_back(myId);
	auto shared = fromParent.lock();
	if (shared != nullptr) {
		shared->catch_child_lost_offspring(branches, key);
	}
}

void VariableTree::catch_child_value_change(vector<HashKey::EitherKey>& branches, const Value& newValue, chrono::time_point<chrono::system_clock> theMoment) {
	{
		lock_guard<mutex> listenerLock(branchData->valueChangeListenerMutex);
		for (auto i = branchData->valueChangeListeners.begin(); i != branchData->valueChangeListeners.end();) {
			auto shared = i->second.lock();
			if (shared == nullptr) {
				auto temp = i++;
				branchData->valueChangeListeners.erase(temp);
			}
			else {
				shared->catch_value_change_event(branches, newValue, theMoment);
				++i;
			}
		}
	}
	branches.push_back(myId);
	auto shared = fromParent.lock();
	if (shared != nullptr) {
		shared->catch_child_value_change(branches, newValue, theMoment);
	}
}

VariableTree::Parent::Parent(VariableTree& _parent) : parent(_parent) {

}

VariableTree::Parent::~Parent() {
}

void VariableTree::Parent::catch_child_add_offspring(std::vector<HashKey::EitherKey>& branches, const HashKey::EitherKey& key) {
	parent.catch_child_add_offspring(branches, key);
}

void VariableTree::Parent::catch_child_lost_offspring(std::vector<HashKey::EitherKey>& branches, const HashKey::EitherKey& key) {
	parent.catch_child_lost_offspring(branches, key);
}

void VariableTree::Parent::catch_child_value_change(std::vector<HashKey::EitherKey>& branches, const Value& value, std::chrono::time_point<std::chrono::system_clock> theMoment) {
	parent.catch_child_value_change(branches, value, theMoment);
}
