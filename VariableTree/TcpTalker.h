#pragma once
#ifndef _TCPTALKER_H_
#define _TCPTALKER_H_
#include "../../MyLib/TcpSocket/TcpListener.h"
#include "TreeBrowser.h"
#include "VariableTree.h"
#include <memory>
#include <unordered_map>

class TcpTalker : public TcpListener {
	friend class MyShadow;
private:
	class MyShadow {
	public:
		MyShadow(TcpTalker& _master) : master(_master) {}
		void write_message(size_t handle, const std::string& message) { master.write_message(handle, message); }
	private:
		TcpTalker& master;
	};
	std::shared_ptr<MyShadow> myShadow;
public:
	static const char END_CHAR = '\n';
	static const size_t MAX_LENGTH = 1024 * 1024;

	TcpTalker(uint16_t portNumber);
	virtual ~TcpTalker();

	void set_target(std::shared_ptr<VariableTree> _variableTree);

protected:
	virtual void catch_message(std::string& data, size_t handle);
	virtual void catch_connect_event(size_t handle);
	virtual void catch_disconnect_event(size_t handle);

private:
	class MessageCatcher : public TreeBrowser::Listener {
	public:
		MessageCatcher(std::shared_ptr<MyShadow> _master, size_t _myId) { master = _master;  myId = _myId; }
		virtual ~MessageCatcher() {}
		virtual void catch_message(const std::string& message) { auto shared = master.lock(); if (shared != nullptr) { shared->write_message(myId, message); } }
	private:
		std::weak_ptr<MyShadow> master;
		size_t myId;
	};
	struct ConnectionData {
		std::shared_ptr<MessageCatcher> updateCatcher;
		std::shared_ptr<TreeBrowser> treeBrowser;
	};
	std::weak_ptr<VariableTree> variableTree;
	//std::unordered_map<size_t, std::shared_ptr<TreeBrowser>> handle2TreeBrowserMap;
	std::mutex handleMapMutex;
	std::unordered_map<size_t, ConnectionData> handle2ConnectionDataMap;
	std::string partialMessage;
};

#endif