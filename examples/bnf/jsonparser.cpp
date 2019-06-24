

#include <stdio.h>

#include "jsonparser.h"

void jsonparser::initTable() {
	ss.push(0);
	ss.push(NTS_file);
	ss.push(0);
	table[NTS_array]['['] = 1;
	table[NTS_file][FALSE] = 2;
	table[NTS_file][NULL] = 3;
	table[NTS_file][NUM] = 4;
	table[NTS_file][STRING] = 5;
	table[NTS_file][TRUE] = 6;
	table[NTS_file]['['] = 7;
	table[NTS_file]['{'] = 8;
	table[NTS_key_values][FALSE] = 9;
	table[NTS_key_values][NULL] = 10;
	table[NTS_key_values][NUM] = 11;
	table[NTS_key_values][STRING] = 12;
	table[NTS_key_values][TRUE] = 13;
	table[NTS_key_values]['['] = 14;
	table[NTS_key_values]['{'] = 15;
	table[NTS_key_values]['}'] = 16;
	table[NTS_more_values][','] = 17;
	table[NTS_more_values]['}'] = 18;
	table[NTS_object]['{'] = 19;
	table[NTS_value][FALSE] = 20;
	table[NTS_value][NULL] = 21;
	table[NTS_value][NUM] = 22;
	table[NTS_value][STRING] = 23;
	table[NTS_value][TRUE] = 24;
	table[NTS_value]['['] = 25;
	table[NTS_value]['{'] = 26;
}

void jsonparser::yyrule(int rule)
{
	switch (rule)
	{
		case 1:
			ss.pop();
			ss.push(']');
			ss.push(NTS_values);
			ss.push('[');
			break;

		case 2:
			ss.pop();
			ss.push(NTS_value);
			break;

		case 3:
			ss.pop();
			ss.push(NTS_value);
			break;

		case 4:
			ss.pop();
			ss.push(NTS_value);
			break;

		case 5:
			ss.pop();
			ss.push(NTS_value);
			break;

		case 6:
			ss.pop();
			ss.push(NTS_value);
			break;

		case 7:
			ss.pop();
			ss.push(NTS_value);
			break;

		case 8:
			ss.pop();
			ss.push(NTS_value);
			break;

		case 9:
			ss.pop();
			ss.push(NTS_more_values);
			ss.push(NTS_value);
			break;

		case 10:
			ss.pop();
			ss.push(NTS_more_values);
			ss.push(NTS_value);
			break;

		case 11:
			ss.pop();
			ss.push(NTS_more_values);
			ss.push(NTS_value);
			break;

		case 12:
			ss.pop();
			ss.push(NTS_more_values);
			ss.push(NTS_value);
			break;

		case 13:
			ss.pop();
			ss.push(NTS_more_values);
			ss.push(NTS_value);
			break;

		case 14:
			ss.pop();
			ss.push(NTS_more_values);
			ss.push(NTS_value);
			break;

		case 15:
			ss.pop();
			ss.push(NTS_more_values);
			ss.push(NTS_value);
			break;

		case 16:
			ss.pop();
			break;

		case 17:
			ss.pop();
			ss.push(NTS_key_values);
			ss.push(',');
			break;

		case 18:
			ss.pop();
			break;

		case 19:
			ss.pop();
			ss.push('}');
			ss.push(NTS_key_values);
			ss.push('{');
			break;

		case 20:
			ss.pop();
			ss.push(TS_FALSE);
			break;

		case 21:
			ss.pop();
			ss.push(TS_NULL);
			break;

		case 22:
			ss.pop();
			ss.push(TS_NUM);
			break;

		case 23:
			ss.pop();
			ss.push(TS_STRING);
			break;

		case 24:
			ss.pop();
			ss.push(TS_TRUE);
			break;

		case 25:
			ss.pop();
			ss.push(NTS_array);
			break;

		case 26:
			ss.pop();
			ss.push(NTS_object);
			break;

		default:
			yyerror("parsing table defaulted");
			return 0;
			break;
	}
}
//
int yylex()
{
}

//
int main()
{
	json
}