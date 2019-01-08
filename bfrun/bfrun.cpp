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

#include <iostream>
#include <iomanip>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <cstring>

#include "Console.h"

#include "Image.h"
#include <fstream>

#include <vector>

#include "bfutils.h"

#include "Command.h"

#include <getopt.h>

using namespace std;

bool ProtectedModeEnabled = false;

void SetProtectedMode(bool mode){
     ProtectedModeEnabled = mode;
}

bool GetProtectedMode(void){
     return ProtectedModeEnabled;
}
//Set 16-bit word mode:
bool WordModeEnabled = false;

void SetWordMode(bool mode){
     WordModeEnabled = mode;
}

//Set 16-bit word mode:
bool Statistic = false;

void SetStatistic(bool mode){
     Statistic = mode;
}

bool GetWordMode(void){
     return WordModeEnabled;
}


uint8_t ExecCode(Image &image, uint16_t *MemoryPtr){
	uint8_t status = 0;

	ADDRESS_TYPE IP = image.GetIpEntry();
	ADDRESS_TYPE AP = image.GetApEntry();

	ADDRESS_TYPE AP_MAX =  image.GetSection(0).GetMemoryBase() + image.GetSection(0).GetMemorySize() ;
	size_t i = 0;

	uint16_t JCC_MASK = GetWordMode() ? 0xFFFF : 0xFF;
	uint16_t bias = 0;
	do {
		bias = (MemoryPtr[IP] & 0x1FFF);
		if (MemoryPtr[IP] & (1 << 12))
		{
			bias |= 0xF000;
		}
		switch (MemoryPtr[IP] & 0xF000) {
		case (CMD_IO):
			if (bias & CMD_INPUT_MASK)
			{
				MemoryPtr[AP] = In() & 0xFF;
			}
			else if (bias & CMD_OUTPUT_MASK)
			{
				Out(MemoryPtr[AP] & 0xFF);
			}
			break;
		case (CMD_ADD):
		case (CMD_SUB):
			MemoryPtr[AP] += bias;
			break;
		case (CMD_RIGHT):
		case (CMD_LEFT):
			AP += bias;
			break;

		case (CMD_JZ):
		case (CMD_JZ_DOWN):
			IP = (MemoryPtr[AP] & JCC_MASK) ? IP : IP + bias;
			break;
		case (CMD_JNZ):
		case (CMD_JNZ_DOWN):
			IP = (MemoryPtr[AP] & JCC_MASK) ? IP + bias : IP;
			break;
		default:
			fprintf(stderr, "IP:0x%04x Invalid opcode: 0x%04x\n", IP, MemoryPtr[IP]);
			break;
		}
		++i;
		++IP;
	} while (IP < AP_MAX);
	
	if (Statistic)
	{
		cerr << "\r\nIstructions_retired:" << i << "\r\n";
	}
	return status;
}


int main(int argc, char *argv[]) {
	int status = -1;
	int c = 0;
	char *filePath = NULL;
	while((c = getopt(argc, argv, "f:pxs")) != -1){
		switch(c)
		{
		case 'f':
			filePath = optarg;
			break;
		case 'p':
			SetProtectedMode(true);
		break;
		case 'x':
			SetWordMode(true);
		break;
		case 's':
			SetStatistic(true);
		break;
		case '?':
			return -1;
			break;
		}
	}
	if (filePath == NULL){
		cout << "Fatal error: add input file"<< endl;
		return -1;
	}
	uint16_t Memory[65536];
	memset(&Memory,0,sizeof(Memory));
	std::fstream File(filePath, std::fstream::in | std::fstream::binary);
	if (!File.good()){
		cerr << "Input file error, exiting"<<endl;
		return -1;
	}

	Image image(File);

	LoadShared(image, Memory);

	if (image.Error()){
		cerr << "Load BF buffer Error, Status =" << status << endl;
		return -1;
	}
	status = ExecCode(image, Memory);
	if (status){
		cerr << "Code Execution Error, Status =" << status << endl;
		return -1;
	}

	return 0;
}
