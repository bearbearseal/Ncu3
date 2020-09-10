#include "Variable.h"

using namespace std;

Variable::Variable() {

}

Variable::~Variable() {

}

Value Variable::read_value() const {
	lock_guard<mutex> lock(valueMutex);
	return value;
}

bool Variable::write_value(const Value& newValue, uint8_t priority) {
	if(writeValue.set_value(priority, newValue)) {
		_write_value(newValue);
		return true;
	}
	return false;
}

void Variable::trigger_value(const Value& newValue) {
	writeValue.clear_value();
	_write_value(newValue);
}

void Variable::update_value_to_cache(const Value& newValue) {
	{
		lock_guard<mutex> lock(valueMutex);
		if(value == newValue) {
			return;
		}
		value = newValue;
	}
	{
		auto shared = listener.lock();
		if (shared != nullptr) {
			shared->catch_value_change(newValue, chrono::system_clock::now());
		}
	}
}

void Variable::set_listener(std::shared_ptr<Listener> _listener) {
	listener = _listener;
	printf("Listener now is %p\n", _listener.get());
}
