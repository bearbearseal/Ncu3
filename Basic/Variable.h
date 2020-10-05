#ifndef _Variable_H_
#define _Variable_H_
#include "Value.h"
#include "PrioritizedValue.h"
#include <memory>
#include <vector>
#include <unordered_set>
#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <tuple>

class Variable
{
public:
	class Listener {
	public:
		Listener() {}
		virtual ~Listener() {}
		virtual void catch_value_change(const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment) = 0;
	};
	Variable();
	virtual ~Variable();
	Value read_value() const;
	bool write_value(const Value& newValue, uint8_t priority);
	void trigger_value(const Value& newValue, uint8_t priority);
	void set_listener(std::shared_ptr<Listener> _listener);
protected:
	void update_value_to_cache(const Value& newValue);
	virtual void _write_value(const Value& newValue) = 0;

private:
	PrioritizedValue writeValue;
	mutable std::mutex valueMutex;
	Value value;

	std::weak_ptr<Listener> listener;
};
#endif
