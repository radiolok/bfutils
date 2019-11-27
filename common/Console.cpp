/* Copyright (c) 2016-2017, Artem Kashkanov
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

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
		fprintf(stdout, "%c", val);
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
