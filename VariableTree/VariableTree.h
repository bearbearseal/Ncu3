#ifndef _VARIABLETREE_H_
#define _VARIABLETREE_H_
#include <unordered_map>
#include <mutex>
#include <vector>
#include <chrono>
#include "../../MyLib/Basic/Variable.h"
#include "../../MyLib/Basic/HashKey.h"

class VariableTree
{
	friend class Parent;
protected:
	class Parent {
	public:
		Parent(VariableTree& _parent);
		virtual ~Parent();
		void catch_child_add_offspring(std::vector<HashKey::EitherKey>& branches, const HashKey::EitherKey& key);
		void catch_child_lost_offspring(std::vector<HashKey::EitherKey>& branches, const HashKey::EitherKey& key);
		void catch_child_value_change(std::vector<HashKey::EitherKey>& branches, const Value& value, std::chrono::time_point<std::chrono::system_clock> theMoment);

	private:
		VariableTree& parent;
	};
public:
	class AddRemoveListener {
	public:
		AddRemoveListener() {}
		virtual ~AddRemoveListener() {}
		virtual void catch_add_child_event(const std::vector<HashKey::EitherKey>& branch, const HashKey::EitherKey& newChild) {};
		virtual void catch_remove_child_event(const std::vector<HashKey::EitherKey>& branch, const HashKey::EitherKey& lostChild) {};
	};

	class ValueChangeListener {
	public:
		ValueChangeListener() {}
		virtual ~ValueChangeListener() {}
		virtual void catch_value_change_event(const std::vector<HashKey::EitherKey>& branch, const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment) {};
	};

public:
	VariableTree(const HashKey::EitherKey& _myId, std::shared_ptr<Parent> parentProxy, bool isLeaf);
	VariableTree(bool isLeaf=false);
	virtual ~VariableTree();

	virtual bool add_child(const HashKey::EitherKey& key, std::shared_ptr<VariableTree> newChild);
	virtual std::shared_ptr<VariableTree> create_branch(const HashKey::EitherKey& key);
	virtual std::shared_ptr<VariableTree> create_leaf(const HashKey::EitherKey& key, std::shared_ptr<Variable> _variable);
	virtual std::shared_ptr<VariableTree> force_create_branch(const HashKey::EitherKey& key);
	virtual std::shared_ptr<VariableTree> force_create_leaf(const HashKey::EitherKey& key, std::shared_ptr<Variable> _variable);
	std::vector<std::pair<HashKey::EitherKey, bool>> list_all_children() const;
	bool remove_child(const HashKey::EitherKey& key);

	std::shared_ptr<VariableTree> get_child(const HashKey::EitherKey& key) const;

	virtual bool write_value(const Value& value, uint8_t priority = 0);
	virtual bool set_value(const Value& value, uint8_t priority = 0);
	virtual Value get_value() const;

	void add_add_remove_listener(std::shared_ptr<AddRemoveListener> listener);
	void remove_add_remove_listener(std::shared_ptr<AddRemoveListener> listener);

	void add_value_change_listener(std::shared_ptr<ValueChangeListener> listener);
	void remove_value_change_listener(std::shared_ptr<ValueChangeListener> listener);

	const bool isLeaf;

protected:
	virtual void catch_value_change(const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment);
	void catch_child_add_offspring(std::vector<HashKey::EitherKey>& branches, const HashKey::EitherKey& key);
	void catch_child_lost_offspring(std::vector<HashKey::EitherKey>& branches, const HashKey::EitherKey& key);
	void catch_child_value_change(std::vector<HashKey::EitherKey>& branches, const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment);

public:
	class VariableListener : public Variable::Listener {
	public:
		VariableListener(VariableTree& _master) : master(_master) {}
		virtual ~VariableListener() {}
		virtual void catch_value_change(const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment) { master.catch_value_change(newValue, theMoment); }
	private:
		VariableTree& master;
	};
	std::shared_ptr<VariableListener> variableListener;

protected:
	static const Value empty;
	HashKey::EitherKey myId;
	std::shared_ptr<Parent> toChildren;
	std::weak_ptr<Parent> fromParent;

private:
	struct BranchData {
		mutable std::mutex dataMutex;
		std::unordered_map<HashKey::EitherKey, std::shared_ptr<VariableTree>, HashKey::EitherKeyHash> dataMap;
		std::mutex addRemoveListenerMutex;
		std::unordered_map<void*, std::weak_ptr<AddRemoveListener>> addRemoveListeners;
		std::mutex valueChangeListenerMutex;
		std::unordered_map<void*, std::weak_ptr<ValueChangeListener>> valueChangeListeners;
	};
	std::unique_ptr<BranchData> branchData;

	struct LeafData {
		//mutable std::mutex dataMutex;
		std::shared_ptr<Variable> variable;
		std::mutex listenerMutex;
		std::unordered_map<void*, std::weak_ptr<ValueChangeListener>> valueChangeListeners;
	};
	std::unique_ptr<LeafData> leafData;

};
#endif