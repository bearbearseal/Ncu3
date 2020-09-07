#pragma once
#ifndef _VALUE_H_
#define _VALUE_H_
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <chrono>
#include "HashKey.h"

class Value
{
private:
	enum class Type
	{
		None = 0,
		Integer = 1,
		Unsigned = 2,
		Float = 3,
		String = 4,
		Time = 5,
		Object = 6
	};
	//For operator [] of string and wstring type

public:
	enum class Status {
		Good = 0,
		Initial = 1
	};

	//Constructor & Destructor
	Value();
	Value(const Value& theOther);
	Value(Value&& theOther) noexcept;
	Value(int8_t);
	Value(int16_t);
	Value(int32_t);
	Value(int64_t);
	Value(uint8_t);
	Value(uint16_t);
	Value(uint32_t);
	Value(uint64_t);
	Value(float);
	Value(double);
	Value(const std::string&);
	Value(const char*);
	Value(std::string&&);
	Value(std::chrono::system_clock::time_point);
	~Value();

	//Check Type
	inline bool is_empty() const { return type == Type::None; }
	inline bool is_integer() const { return (type == Type::Integer || type == Type::Unsigned); }
	inline bool is_float() const { return type == Type::Float; }
	inline bool is_string() const { return type == Type::String; }
	inline bool is_time() const { return type == Type::Time; }
	inline bool is_object() const { return type == Type::Object; }
	inline bool is_numeric() const { return (type == Type::Integer || type == Type::Float); }

	//Get Value
	int64_t get_int() const;
	double get_float() const;
	std::string get_string() const;
	std::chrono::system_clock::time_point get_time() const;
	bool has_entry(const std::string& name) const;

	//Set Value
	void set_value(const Value&);
	void set_value(Value&&);
	void set_integer(int64_t);
	void set_float(double);
	void set_string(const std::string&);
	void set_time(std::chrono::system_clock::time_point);
	void set_object(const std::unordered_map<HashKey::EitherKey, Value, HashKey::EitherKey>&);
	void clear_object();

	std::string to_string() const;

	//Operators
	void operator=(const Value&);
	void operator=(Value&&) noexcept;
	Value operator+(const Value&) const;
	Value operator-(const Value&) const;
	Value operator*(const Value&) const;
	Value operator/(const Value&) const;
	Value operator%(const Value&) const;
	bool operator==(const Value&) const;
	bool operator!=(const Value&) const;
	bool operator>(const Value&) const;
	bool operator<(const Value&) const;
	bool operator>=(const Value&) const;
	bool operator<=(const Value&) const;
	Value& operator[](const Value&);

	//Get object entry
	const Value& get_value(const HashKey::EitherKey& mapKey) const;

private:
	void delete_data();
	union Data {
		int64_t* intValue;
		double* floatValue;
		std::string* stringValue;
		std::chrono::system_clock::time_point* timeValue;
		std::unordered_map<HashKey::EitherKey, Value, HashKey::EitherKey>* objectValue;
		char* pointed;
	};
	Data data = { NULL };
	Type type = Type::None;
	static Value emptyValue;
	static Value writableEmpty;
};
#endif
