/*
 * Console.cpp
 *
 *  Created on: 30 сент. 2016 г.
 *      Author: radiolok
 */

#include "Console.h"

uint8_t InputString[256];
size_t input_string_current_pos = 0;

bool stringIsFree = true;

size_t GetCharLength(uint8_t *str)
{
	size_t length = 0;
	while(*(str + length))
	{
		++length;
	}
	return length;
}

void Out(uint16_t val){
	if (val < 0x20)
	{
		fprintf(stderr, "(%02x)", val);
	}
	else{
		fprintf(stderr, "%c", val);
	}

}


uint8_t In(void){
	uint8_t val = 0;
	if (stringIsFree == true)
	{
		fprintf(stderr, ">");
		scanf("%s", InputString);
		size_t length = GetCharLength(InputString);
		if (length != 0)
		{
			stringIsFree = false;
			input_string_current_pos = 0;
			val = InputString[input_string_current_pos++];
		}
	}
	else
	{
		val = InputString[input_string_current_pos++];
	}
	if (!val){
		stringIsFree = true;
	}
	return val;
}
