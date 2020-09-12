#pragma once
#ifndef _RamVariable_H_
#define _RamVariable_H_
#include "Variable.h"

class RamVariable : public Variable {
public:
	RamVariable() {}
	~RamVariable() {}
	void _write_value(const Value& newValue) { this->update_value_to_cache(newValue); }
};

#endif