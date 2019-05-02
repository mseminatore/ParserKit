#ifndef __JSON_H
#define __JSON_H

#pragma once
#include <vector>
#include <map>

struct JSONValue;

struct JSONArray
{
	std::vector<std::unique_ptr<JSONValue>> elements;
};

struct JSONObject
{
	std::map<std::string, std::unique_ptr<JSONValue>> key_values;
};

struct JSONValue
{
	enum class ValueType {None, Null, String, Boolean, Number, Object, Array};
	ValueType value_type;

	union
	{
		std::string s;
		bool b;
		float n;
		std::unique_ptr<JSONArray> a;
		std::unique_ptr<JSONObject> o;
	};

	JSONValue() { value_type = ValueType::None; }
	virtual ~JSONValue() {};
};

#endif //__JSON_H