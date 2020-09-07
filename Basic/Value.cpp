#include <ctime>
#include <math.h>
#include "Value.h"

using namespace std;

Value Value::emptyValue;
Value Value::writableEmpty;

Value::Value()
{
}

Value::Value(const Value& theOther)
{
	set_value(theOther);
	//printf("Constructor copied %s\n", this->to_string().c_str());
}

Value::Value(Value&& theOther) noexcept
{
	type = theOther.type;
	data.intValue = theOther.data.intValue;
	theOther.type = Type::None;
	theOther.data.intValue = nullptr;
	//printf("Constructor moved %s\n", this->to_string().c_str());
}

Value::Value(int8_t newValue)
{
	set_integer(newValue);
}

Value::Value(int16_t newValue)
{
	set_integer(newValue);
}

Value::Value(int32_t newValue)
{
	set_integer(newValue);
}

Value::Value(int64_t newValue)
{
	set_integer(newValue);
}

Value::Value(uint8_t newValue)
{
	set_integer((int64_t)newValue);
}

Value::Value(uint16_t newValue)
{
	set_integer((int64_t)newValue);
}

Value::Value(uint32_t newValue)
{
	set_integer((int64_t)newValue);
}

Value::Value(uint64_t newValue)
{
	set_integer((int64_t)newValue);
}

Value::Value(float newValue)
{
	set_float(newValue);
}

Value::Value(double newValue)
{
	set_float(newValue);
}

Value::Value(const std::string& newValue)
{
	set_string(newValue);
}

Value::Value(const char* newValue) {
	set_string(newValue);
}

Value::Value(std::string&& newValue)
{
	set_string(newValue);
}

Value::Value(std::chrono::system_clock::time_point newValue)
{
	set_time(newValue);
}

Value::~Value()
{
	delete_data();
}

void Value::delete_data()
{
	switch (type)
	{
	case Type::Integer:
		delete data.intValue;
		break;
	case Type::Float:
		delete data.floatValue;
		break;
	case Type::String:
		delete data.stringValue;
		break;
	case Type::Time:
		delete data.timeValue;
		break;
	case Type::Object:
		delete data.objectValue;
		break;
	default:
		break;
	}
	type = Type::None;
}

int64_t Value::get_int() const
{
	switch (type)
	{
	case Type::Integer:
		return *(data.intValue);
	case Type::Float:
		return (int64_t)* (data.floatValue);
	default:
		return 0;
	}
}

double Value::get_float() const
{
	switch (type)
	{
	case Type::Integer:
		return (double)* (data.intValue);
	case Type::Float:
		return *(data.floatValue);
	default:
		return numeric_limits<double>::quiet_NaN();
	}
}

std::string Value::get_string() const
{
	switch (type)
	{
	case Type::Integer:
		return ::to_string(*(data.intValue));
	case Type::Float:
		return ::to_string(*(data.floatValue));
	case Type::String:
		return *(data.stringValue);
	case Type::Time:
	{
		time_t aTime = chrono::system_clock::to_time_t(*(data.timeValue));
		char str[64];
#if defined(_WIN32)
		ctime_s(str, sizeof(str), &aTime);
#else
		ctime_r(&aTime, str);
#endif
		return str;
	}
	case Type::Object:
		return "object";
	default:
		return "Undefined";
	}
}

std::chrono::system_clock::time_point Value::get_time() const
{
	if (Type::Time == type)
	{
		return *(data.timeValue);
	}
	return chrono::system_clock::now();
}

bool Value::has_entry(const std::string& name) const
{
	if (Type::Object == type)
	{
		return (bool)data.objectValue->count(name);
	}
	return false;
}

void Value::set_value(const Value& newValue)
{
	if (type != newValue.type)
	{
		delete_data();
	}
	switch (newValue.type)
	{
	case Type::Integer:
		set_integer(*newValue.data.intValue);
		break;
	case Type::Float:
		set_float(*newValue.data.floatValue);
		break;
	case Type::String:
		set_string(*newValue.data.stringValue);
		break;
	case Type::Time:
		set_time(*newValue.data.timeValue);
		break;
	case Type::Object:
		set_object(*newValue.data.objectValue);
		break;
	default:
		break;
	}
}

void Value::set_value(Value&& newValue)
{
	if (type != Type::None)
	{
		delete_data();
	}
	type = newValue.type;
	data.intValue = newValue.data.intValue;
	newValue.type = Type::None;
	newValue.data.intValue = nullptr;
}

void Value::set_integer(int64_t newValue)
{
	if (type != Type::Integer) {
		delete_data();
		type = Type::Integer;
		data.intValue = new int64_t();
	}
	*(data.intValue) = newValue;
}

void Value::set_float(double newValue)
{
	if (type != Type::Float) {
		delete_data();
		type = Type::Float;
		data.floatValue = new double();
	}
	*(data.floatValue) = newValue;
}

void Value::set_string(const std::string& newValue)
{
	if (type != Type::String) {
		delete_data();
		type = Type::String;
		data.stringValue = new string();
	}
	*(data.stringValue) = newValue;
}

void Value::set_time(std::chrono::system_clock::time_point newValue)
{
	if (type != Type::Time)
	{
		delete_data();
		type = Type::Time;
		data.timeValue = new chrono::system_clock::time_point();
	}
	*(data.timeValue) = newValue;
}

void Value::set_object(const std::unordered_map<HashKey::EitherKey, Value, HashKey::EitherKey>& newValue)
{
	if (type != Type::Time)
	{
		delete_data();
		type = Type::Object;
		data.objectValue = new std::unordered_map<HashKey::EitherKey, Value, HashKey::EitherKey>();
	}
	*(data.objectValue) = newValue;
}

void Value::clear_object()
{
	if (type == Type::Object)
	{
		data.objectValue->clear();
	}
}

string Value::to_string() const
{
	string retVal;
	switch (type)
	{
	case Type::Integer:
		retVal = "I:";
		retVal += ::to_string(*(data.intValue));
		break;
	case Type::Float:
		retVal = "f:";
		retVal += ::to_string(*(data.floatValue));
		break;
	case Type::String:
		retVal = "s:";
		retVal += *(data.stringValue);
		break;
	case Type::Time:
	{
		retVal = "t:";
		time_t aTime = chrono::system_clock::to_time_t(*(data.timeValue));
		char str[64];
#if defined(_WIN32)
		ctime_s(str, sizeof(str), &aTime);
#else
		ctime_r(&aTime, str);
#endif
		retVal += str;
		break;
	}
	case Type::Object:
		return "object";
	default:
		return "Undefined";
	}
	return retVal;
}

const Value& Value::get_value(const HashKey::EitherKey& mapKey) const
{
	if (Type::Object == type)
	{
		if (data.objectValue->count(mapKey))
		{
			return data.objectValue->at(mapKey);
		}
	}
	return emptyValue;
}

void Value::operator=(const Value& right)
{
	//printf("Gonna set value to %s\n", right.to_string().c_str());
	set_value(right);
}

void Value::operator=(Value&& right) noexcept
{
	set_value(right);
}

Value Value::operator+(const Value& right) const
{
	Value retVal;
	switch (type)
	{
	case Type::Integer:
		retVal.set_integer(get_int() + right.get_int());
		break;
	case Type::Float:
		retVal.set_float(get_float() + right.get_float());
		break;
	case Type::String:
		retVal.set_string(get_string() + right.get_string());
		break;
	case Type::None:
		retVal.set_value(right);
		break;
	default:
		break;
	}
	return retVal;
}

Value Value::operator-(const Value& right) const
{
	Value retVal;
	switch (type) {
	case Type::Integer:
		retVal.set_integer((int64_t)(*data.intValue - right.get_int()));
		break;
	case Type::Float:
		retVal.set_float((double)(*data.floatValue - right.get_float()));
		break;
	case Type::Time:
		//retVal.set_time(*data.timeValue - right.get_time());
		break;
	default:
		break;
	}
	return retVal;
}

Value Value::operator*(const Value& right) const
{
	Value retVal;
	switch (type) {
	case Type::Integer:
		retVal.set_integer((int64_t)(*data.intValue * right.get_int()));
		break;
	case Type::Float:
		retVal.set_float((double)(*data.floatValue * right.get_float()));
		break;
	default:
		break;
	}
	return retVal;
}

Value Value::operator/(const Value& right) const
{
	Value retVal;
	switch (type) {
	case Type::Integer:
		retVal.set_integer((int64_t)(*data.intValue / right.get_int()));
		break;
	case Type::Float:
		retVal.set_float((double)(*data.floatValue / right.get_float()));
		break;
	default:
		break;
	}
	return retVal;
}

Value Value::operator%(const Value& right) const
{
	Value retVal;
	switch (type) {
	case Type::Integer:
		retVal.set_integer((int64_t)(*data.intValue % right.get_int()));
		break;
	case Type::Float:
		retVal.set_float(fmod(*data.floatValue, right.get_float()));
		break;
	default:
		break;
	}
	return retVal;
}

bool Value::operator==(const Value& right) const
{
	if (type != right.type) {
		return false;
	}
	switch (type) {
	case Type::None:
		return true;
	case Type::Integer:
		return (get_int() == right.get_int());
	case Type::Float:
	{
		double difference = get_float() - right.get_float();
		difference = fabs(difference);
		return (difference <= numeric_limits<double>::epsilon());
	}
	case Type::String:
		return (get_string().compare(right.get_string()) == 0);
	default:
		break;
	}
	return false;
}

bool Value::operator!=(const Value& right) const
{
	return !operator==(right);
}

bool Value::operator>(const Value& right) const
{
	switch (type) {
	case Type::Integer:
	case Type::Float:
		return get_float() > right.get_float();
	case Type::String:
		if (right.is_string())
		{
			return (get_string().compare(right.get_string()) > 0);
		}
		break;
	default:
		break;
	}
	return false;
}

bool Value::operator<(const Value& right) const
{
	switch (type) {
	case Type::Integer:
	case Type::Float:
		return get_float() < right.get_float();
	case Type::String:
		if (right.is_string())
		{
			return (get_string().compare(right.get_string()) < 0);
		}
		break;
	default:
		break;
	}
	return false;
}

bool Value::operator>=(const Value& right) const
{
	switch (type) {
	case Type::Integer:
	case Type::Float:
		return get_float() >= right.get_float();
	case Type::String:
		if (right.is_string())
		{
			return (get_string().compare(right.get_string()) >= 0);
		}
		break;
	default:
		break;
	}
	return false;
}

bool Value::operator<=(const Value& right) const
{
	switch (type) {
	case Type::Integer:
	case Type::Float:
		return get_float() <= right.get_float();
	case Type::String:
		if (right.is_string())
		{
			return (get_string().compare(right.get_string()) <= 0);
		}
		break;
	default:
		break;
	}
	return false;
}

Value& Value::operator[](const Value& index)
{
	switch (type) {
	case Type::Object:
		if (index.is_string())
		{
			if (data.objectValue->count(index.get_string()))
			{
				return (*data.objectValue)[HashKey::EitherKey(index.get_string())];
			}
		}
		else if (index.is_integer())
		{
			if (data.objectValue->count(index.get_int()))
			{
				return (*data.objectValue)[HashKey::EitherKey(index.get_int())];
			}
		}
		break;
	default:
		break;
	}
	writableEmpty.delete_data();
	return writableEmpty;
}
