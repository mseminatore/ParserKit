#ifndef __JSONVALUE_H
#define __JSONVALUE_H

#pragma once
#include <vector>
#include <map>
#include <memory>

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

	JSONValue() : s(), o(), a() {
		value_type = ValueType::None;
	}

	virtual ~JSONValue() {};

	// not copyable
	JSONValue(JSONValue&) = delete;
	JSONValue &operator=(JSONValue&) = delete;

	// only movable
	JSONValue(JSONValue&&) = default;
	JSONValue &operator=(JSONValue&&) = default;

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