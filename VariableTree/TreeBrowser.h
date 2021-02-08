#pragma once
#ifndef _TREEBROWSER_H_
#define _TREEBROWSER_H_
#include "VariableTree.h"
#include "../../MyLib/Basic/HashKey.h"
#include <vector>
#include <list>
#include <functional>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include "../../OtherLib/nlohmann/json.hpp"

class TreeBrowser {
	friend class BranchListener;
	friend class ValueListener;
private:
	static constexpr char COMMAND_BackToTop[] = "MoveToTop";
	static constexpr char COMMAND_ShowBranches[] = "Show";
	static constexpr char COMMAND_MoveToBranch[] = "MoveDown";
	static constexpr char COMMAND_ListenToValue[] = "ListenValue";
	static constexpr char COMMAND_ListenBranch[] = "ListenBranch";
	static constexpr char COMMAND_ReadValue[] = "Read";
	static constexpr char COMMAND_WriteValue[] = "Write";
	static constexpr char COMMAND_SetValue[] = "Set"; 
	static constexpr char COMMAND_UnsetValue[] = "Unset";
	static constexpr char COMMAND_GetPriority[] = "GetPriority";
	static constexpr char COMMAND_CreateBranch[] = "CreateBranch";
	static constexpr char COMMAND_CreateLeaf[] = "CreateLeaf";
	static constexpr char COMMAND_DeleteChild[] = "DeleteChild";

	class MyShadow{
	public:
		MyShadow(TreeBrowser& _master) : master(_master) {}
		~MyShadow() {}
		void forward_to_listener(const std::string& message) { master.forward_to_listener(message); }
	private:
		TreeBrowser& master;
	};

public:
	class Listener {
	public:
		virtual ~Listener() {}
		virtual void catch_message(const std::string& message){}
	};
	TreeBrowser(std::shared_ptr<VariableTree> _variableTree, std::shared_ptr<Listener> listener = nullptr);
	virtual ~TreeBrowser();

	std::string process_command(const std::string& input);

private:
	void forward_to_listener(const std::string& message);

	class BranchListener : public VariableTree::AddRemoveListener {
	public:
		BranchListener(const HashKey::EitherKey& newId);
		virtual ~BranchListener();

		void change_id(const HashKey::EitherKey& newId);
		HashKey::EitherKey read_id();

	private:
		HashKey::EitherKey id;
	};

	class ValueListener : public VariableTree::ValueChangeListener {
	public:
		ValueListener(std::shared_ptr<MyShadow> _master, const HashKey::EitherKey& _tagId);
		virtual ~ValueListener();

		void change_id(const HashKey::EitherKey& tagId);
		HashKey::EitherKey read_id();
		virtual void catch_value_change_event(const std::vector<HashKey::EitherKey>& branch, const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment);

	private:
		std::weak_ptr<MyShadow> master;
		HashKey::EitherKey tagId;
	};

private:
	std::string process_command_back_to_top();
	std::string process_command_move_to_branch(const nlohmann::json& jData);
	std::string process_command_show_branches();
	std::string process_command_subsribe_to_branch_event(const nlohmann::json& jData);
	std::string process_command_subscribe_to_value_event(const nlohmann::json& jData);
	std::string process_command_read_value(const nlohmann::json& jData);
	std::string process_command_write_value(const nlohmann::json& jData);
	std::string process_command_set_value(const nlohmann::json& jData);
	std::string process_command_unset_value(const nlohmann::json& jData);
	std::string process_command_get_priority(const nlohmann::json& jData);
	std::string process_command_create_branch(const nlohmann::json& jData);
	std::string process_command_create_leaf(const nlohmann::json& jData);
	std::string process_command_delete_child(const nlohmann::json& jData);

private:
	std::weak_ptr<VariableTree> treeRoot;
	std::weak_ptr<VariableTree> cursor;

	struct SubscribeData {
		std::weak_ptr<VariableTree> subscribed;
		std::shared_ptr<BranchListener> subscriber;
	};

	struct {
		std::mutex theMutex;
		std::unordered_map<void*, std::shared_ptr<BranchListener>> data;
	}branchListeners;

	struct {
		std::mutex theMutex;
		std::unordered_map<void*, std::shared_ptr<ValueListener>> data;
	}valueListeners;

	std::weak_ptr<Listener> myListener;
	std::shared_ptr<MyShadow> myShadow;
};
#endif