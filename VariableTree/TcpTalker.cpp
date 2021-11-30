#include "TcpTalker.h"

using namespace std;

TcpTalker::TcpTalker(uint16_t portNumber) : TcpListener(portNumber) {
	myShadow = make_shared<MyShadow>(*this);
}

TcpTalker::~TcpTalker() {
	weak_ptr<MyShadow> _myShadow = myShadow;
	myShadow.reset();
	while (_myShadow.lock() != nullptr) {
		this_thread::yield();
	}
}

void TcpTalker::set_target(shared_ptr<VariableTree> _variableTree) {
	variableTree = _variableTree;
}

void TcpTalker::catch_message(string& message, size_t handle) {
	//printf("Caught message.\n");
next_while:
	while (message.size()) {
		for (unsigned i = 0; i < message.size(); ++i) {
			if (message[i] == END_CHAR) {
				partialMessage += message.substr(0, i);
				message = message.substr(i + 1);
				if (partialMessage.size() >= MAX_LENGTH) {
					partialMessage.clear();
					goto next_while;
				}
				auto j = handle2ConnectionDataMap.find(handle);
				if (j!= handle2ConnectionDataMap.end()) {
					string reply = j->second.treeBrowser->process_command(partialMessage);
					write_message(handle, reply);
					partialMessage.clear();
					goto next_while;
				}
				else{
					printf("Cannot find handle: %zu\n", handle);
				}
			}
		}
		partialMessage += message;
		message.clear();
	}
}

void TcpTalker::catch_connect_event(size_t handle) {
	printf("Caught connect event.\n");
	//handle2TreeBrowserMap.insert({handle, make_shared<TreeBrowser>(shared)});
	auto shared = variableTree.lock();
	if (shared != nullptr) {
		ConnectionData connectionData;
		connectionData.updateCatcher = make_shared<MessageCatcher>(myShadow, handle);
		connectionData.treeBrowser = make_shared<TreeBrowser>(shared, connectionData.updateCatcher);
		lock_guard<mutex> lock(handleMapMutex);
		handle2ConnectionDataMap[handle] = connectionData;
	}
	//handle2ConnectionDataMap.insert({handle});
}

void TcpTalker::catch_disconnect_event(size_t handle) {
	printf("Caught disconnect event.\n");
	handle2ConnectionDataMap.erase(handle);
}

