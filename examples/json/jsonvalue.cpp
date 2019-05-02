#include "jsonvalue.h"

//
void JSONValue::dump()
{
	switch (value_type)
	{
	default:
	case ValueType::None:
		break;

	case ValueType::Null:
		printf("null");
		break;

	case ValueType::Boolean:
		if (b)
			printf("true");
		else
			printf("false");
		break;

	case ValueType::Number:
		printf("%f", n);
		break;

	case ValueType::String:
		printf("\"%s\"", s.c_str());
		break;

	case ValueType::Array:
		a->dump();
		break;

	case ValueType::Object:
		o->dump();
		break;
	};
}

//
void JSONObject::dump()
{
	puts("{");

	auto iter = key_values.begin();
	for (; iter != key_values.end(); iter++)
	{
		printf("\"%s\": ", iter->first.c_str());
		iter->second->dump();
		puts(",");
	}

	puts("}");
}

//
void JSONArray::dump()
{
	puts("[");

	auto iter = elements.begin();
	for (; iter != elements.end(); iter++)
	{
		(*iter)->dump();
		puts(",");
	}

	puts("]");
}
