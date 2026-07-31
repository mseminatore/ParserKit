#ifndef __JSONVALUE_H
#define __JSONVALUE_H

#pragma once
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <cstring>

struct JSONArray;
struct JSONObject;

struct JSONValue
{
	enum class ValueType { None, Null, String, Boolean, Number, Object, Array };
	ValueType value_type;

	union
	{
		std::string s;
		bool b;
		float n;
		std::unique_ptr<JSONArray> a;
		std::unique_ptr<JSONObject> o;
	};

	JSONValue() : value_type(ValueType::None) {
		// Zero-initialize union storage.
		// A zero-bit std::string is a valid empty SSO string in libc++.
		// A zero-bit unique_ptr is a valid null unique_ptr.
		std::memset((void*)&s, 0, sizeof(s));
	}

	virtual ~JSONValue() {
		switch (value_type) {
			case ValueType::String:  s.~basic_string<char>(); break;
			case ValueType::Object:  o.~unique_ptr<JSONObject>(); break;
			case ValueType::Array:   a.~unique_ptr<JSONArray>(); break;
			default: break;
		}
	}

	// not copyable
	JSONValue(const JSONValue&) = delete;
	JSONValue &operator=(const JSONValue&) = delete;

	// movable
	JSONValue(JSONValue&& other) noexcept : value_type(other.value_type) {
		switch (value_type) {
			case ValueType::String:
				new (&s) std::string(std::move(other.s));
				other.s.~basic_string<char>();
				break;
			case ValueType::Object:
				new (&o) std::unique_ptr<JSONObject>(std::move(other.o));
				other.o.~unique_ptr<JSONObject>();
				break;
			case ValueType::Array:
				new (&a) std::unique_ptr<JSONArray>(std::move(other.a));
				other.a.~unique_ptr<JSONArray>();
				break;
			case ValueType::Boolean: b = other.b; break;
			case ValueType::Number:  n = other.n; break;
			default: break;
		}
		other.value_type = ValueType::None;
		std::memset((void*)&other.s, 0, sizeof(other.s));
	}

	JSONValue &operator=(JSONValue&& other) noexcept {
		if (this != &other) {
			this->~JSONValue();
			new (this) JSONValue(std::move(other));
		}
		return *this;
	}

	void dumpAll();
	void dump();
};


struct JSONArray
{
	std::vector<std::unique_ptr<JSONValue>> elements;

	JSONArray() = default;
	virtual ~JSONArray() = default;

	// not copyable
	JSONArray(JSONArray&) = delete;
	JSONArray &operator=(JSONArray&) = delete;

	// only movable
	JSONArray(JSONArray&&) = default;
	JSONArray &operator=(JSONArray&&) = default;

	void dump();
};

struct JSONObject
{
	std::map<std::string, std::unique_ptr<JSONValue>> key_values;
	
	JSONObject() = default;
	virtual ~JSONObject() = default;

	// not copyable
	JSONObject(JSONObject&) = delete;
	JSONObject &operator=(JSONObject&) = delete;

	// only movable
	JSONObject(JSONObject&&) = default;
	JSONObject &operator=(JSONObject&&) = default;

	void dump();
};

#endif //__JSONVALUE_H