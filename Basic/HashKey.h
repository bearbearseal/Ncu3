#ifndef _HASHKEY_H_
#define _HASHKEY_H_
#include <string>
#include <vector>

namespace HashKey
{
	class EitherKey
	{
	public:
		enum class KeyType
		{
			None = 0,
			Integer,
			String
		};
		EitherKey();
		EitherKey(const EitherKey& other);
		EitherKey(int64_t key);
		EitherKey(const std::string& key);
		EitherKey(const char* key);
		~EitherKey();

		bool operator==(const EitherKey& theOther) const;
		size_t operator()(const EitherKey& me) const;
		EitherKey& operator=(const EitherKey& theOther);

		std::string to_string() const;
		bool is_integer() const;
		int64_t get_integer() const;
		bool is_string() const;
		const std::string& get_string() const;
		bool is_empty() const;

	private:
		int64_t intValue;
		std::string stringValue;
		KeyType keyType;
	};
}

inline HashKey::EitherKey::EitherKey()
{
	keyType = KeyType::Integer;
	intValue = 0;
}

inline HashKey::EitherKey::EitherKey(const EitherKey& other)
{
	keyType = other.keyType;
	if (keyType == KeyType::Integer)
	{
		intValue = other.intValue;
	}
	else
	{
		stringValue = other.stringValue;
	}
}

inline HashKey::EitherKey::EitherKey(int64_t newValue)
{
	keyType = KeyType::Integer;
	intValue = newValue;
}

inline HashKey::EitherKey::EitherKey(const std::string& key)
{
	keyType = KeyType::String;
	stringValue = key;
}

inline HashKey::EitherKey::EitherKey(const char* key)
{
	keyType = KeyType::String;
	stringValue = key;
}

inline HashKey::EitherKey::~EitherKey()
{
}

inline bool HashKey::EitherKey::operator==(const EitherKey& theOther) const
{
	if (keyType != theOther.keyType)
	{
		return false;
	}
	switch (keyType)
	{
	case KeyType::Integer:
		return intValue == theOther.intValue;
	case KeyType::String:
		return !((stringValue).compare(theOther.stringValue));
	default:
		break;
	}
	return false;
}

inline size_t HashKey::EitherKey::operator()(const EitherKey& me)const
{
	if (KeyType::String == me.keyType)
	{
		std::hash<std::string> hasher;
		return hasher(me.stringValue);
	}
	else
	{
		std::hash<int64_t> hasher;
		return hasher(me.intValue);
	}
}

inline HashKey::EitherKey& HashKey::EitherKey::operator=(const EitherKey& theOther)
{
	keyType = theOther.keyType;
	if (keyType == KeyType::String)
	{
		stringValue = theOther.stringValue;
	}
	else
	{
		intValue = theOther.intValue;
	}
	return *this;
}

inline std::string HashKey::EitherKey::to_string() const
{
	std::string retVal;
	switch (keyType)
	{
	case KeyType::Integer:
		retVal = "I:";
		retVal += std::to_string(intValue);
		break;
	case KeyType::String:
		retVal = "s:";
		retVal += stringValue;
		break;
	default:
		return "Unknown";
	}
	return retVal;
}

inline bool HashKey::EitherKey::is_integer() const
{
	return keyType == KeyType::Integer;
}

inline int64_t HashKey::EitherKey::get_integer() const
{
	return intValue;
}

inline bool HashKey::EitherKey::is_string() const
{
	return keyType == KeyType::String;
}

inline const std::string& HashKey::EitherKey::get_string() const
{
	return stringValue;
}

inline bool HashKey::EitherKey::is_empty() const {
	return keyType == KeyType::None;
}

#endif
