#pragma once

namespace Sys {

	const unsigned one = 1U;

	inline bool little_endian()
	{
		return reinterpret_cast<const char*>(&one) + sizeof(unsigned) - 1;
	}

	inline bool big_endian()
	{
		return !little_endian();
	}

} // sys