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

//Set 16-bit word mode:
bool Debug = false;

void SetDebug(bool mode) {
	Debug = mode;
}

bool getDebug(void) {
	return Debug;
}

//Set 16-bit word mode:
bool Hex = false;

void SetHex(bool mode) {
	Hex = mode;
}

bool getHex(void) {
	return Hex;
}

bool GetWordMode(void){
     return WordModeEnabled;
}




void debugOutput(size_t instrRetired, const char * cmd, uint16_t ip, uint16_t ap, uint16_t data)
{
	if (getDebug())
	{
		cerr << std::dec << instrRetired << ": IP: 0x" << setw(4) << std::hex << ip << " (" << cmd << ") "
			<< " AP: 0x" << setw(4)  << std::hex << ap
			<< " *AP: 0x"<< std::hex << data << std::endl;
	
	}
	
}

uint8_t ExecCode(Image &image, uint16_t *MemoryPtr){
	uint8_t status = 0;

	ADDRESS_TYPE IP = image.GetIpEntry();
	ADDRESS_TYPE AP = image.GetApEntry();

	ADDRESS_TYPE AP_MAX =  image.GetSection(0).GetMemoryBase() + image.GetSection(0).GetMemorySize() ;
	size_t instrRetired = 0;

	uint16_t JCC_MASK = GetWordMode() ? 0xFFFF : 0xFF;
	uint16_t bias = 0;
	do {
		bias = (MemoryPtr[IP] & 0x1FFF);
		if (MemoryPtr[IP] & (1 << 12))
		{
			bias |= 0xF000;
		}
		switch (MemoryPtr[IP] & 0xF000) {
		case (CMD_NOP):
			break;
		case (CMD_IO):
			if (bias & CMD_INPUT_MASK)
			{

				MemoryPtr[AP] = In() & 0xFF;
				debugOutput(instrRetired, "getc", IP, AP, MemoryPtr[AP]);
			}
			else if (bias & CMD_OUTPUT_MASK)
			{
				Out(MemoryPtr[AP] & 0xFF);
				debugOutput(instrRetired, "putc", IP, AP, MemoryPtr[AP]);
			}
			else if (bias & CMD_CTRLIO_CLR_DATA)
			{
				MemoryPtr[AP] = 0;
				debugOutput(instrRetired, "clr.data", IP, AP, MemoryPtr[AP]);
			}
			break;
		case (CMD_ADD):
		case (CMD_SUB):
			MemoryPtr[AP] += bias;
			debugOutput(instrRetired, "ADD", IP, AP, MemoryPtr[AP]);
			break;
		case (CMD_RIGHT):
		case (CMD_LEFT):
			AP += bias;
			debugOutput(instrRetired, "ADA", IP, AP, MemoryPtr[AP]);
			break;

		case (CMD_JZ):
		case (CMD_JZ_DOWN):
			IP = (MemoryPtr[AP] & JCC_MASK) ? IP : IP + bias;
			debugOutput(instrRetired, "JZ", IP, AP, MemoryPtr[AP]);
			break;
		case (CMD_JNZ):
		case (CMD_JNZ_DOWN):
			IP = (MemoryPtr[AP] & JCC_MASK) ? IP + bias : IP;
			debugOutput(instrRetired, "JNZ", IP, AP, MemoryPtr[AP]);
			break;
		default:
			fprintf(stderr, "IP:0x04lx Unknown Opcode: 0x%04lx", IP, MemoryPtr[IP]);
			break;
		}
		++instrRetired;
		++IP;
	} while (IP < AP_MAX);
	
	if (Statistic)
	{
		cerr << "\r\nIstructions_retired:" << instrRetired << "\r\n";
	}
	return status;
}

uint8_t ExecCode(uint16_t *CodePtr, uint16_t codeSize, uint16_t *MemoryPtr) {
	uint8_t status = 0;

	ADDRESS_TYPE IP = 0;
	ADDRESS_TYPE AP = 0;

	memcpy(MemoryPtr, CodePtr, codeSize * sizeof(uint16_t));


	ADDRESS_TYPE AP_MAX = codeSize;
	size_t instrRetired = 0;

	uint16_t JCC_MASK = GetWordMode() ? 0xFFFF : 0xFF;
	uint16_t bias = 0;
	do {
		++IP;
		bias = (MemoryPtr[IP] & 0x1FFF);
		if (MemoryPtr[IP] & (1 << 12))
		{
			bias |= 0xF000;
		}
		switch (MemoryPtr[IP] & 0xF000) {
		case (CMD_NOP):
			break;
		case (CMD_IO):
			if (bias & CMD_INPUT_MASK)
			{

				MemoryPtr[AP] = In() & 0xFF;
				debugOutput(instrRetired, "getc", IP, AP, MemoryPtr[AP]);
			}
			else if (bias & CMD_OUTPUT_MASK)
			{
				Out(MemoryPtr[AP] & 0xFF);
				debugOutput(instrRetired, "putc", IP, AP, MemoryPtr[AP]);
			}
			else if (bias & CMD_CTRLIO_CLR_DATA)
			{
				MemoryPtr[AP] = 0;
				debugOutput(instrRetired, "clr.data", IP, AP, MemoryPtr[AP]);
			}
			else if (bias & CMD_CTRLIO_MODE8)
			{
				SetWordMode(false);
				JCC_MASK = GetWordMode() ? 0xFFFF : 0xFF;
				debugOutput(instrRetired, "CTRLIO.BIT8", IP, AP, MemoryPtr[AP]);
			}
			else if (bias & CMD_CTRLIO_MODE16)
			{
				SetWordMode(true);
				JCC_MASK = GetWordMode() ? 0xFFFF : 0xFF;
				debugOutput(instrRetired, "CTRLIO.BIT16", IP, AP, MemoryPtr[AP]);
			}
			else if (bias & CMD_CTRLIO_HALT)
			{
				debugOutput(instrRetired, "CTRLIO.HALT", IP, AP, MemoryPtr[AP]);
				goto stop;
			}
			else if (bias & CMD_CTRLIO_PAUSE)
			{
				debugOutput(instrRetired, "CTRLIO.PAUSE", IP, AP, MemoryPtr[AP]);
			}
			break;
		case (CMD_ADD):
		case (CMD_SUB):
			MemoryPtr[AP] += bias;
			debugOutput(instrRetired, "ADD", IP, AP, MemoryPtr[AP]);
			break;
		case (CMD_RIGHT):
		case (CMD_LEFT):
			AP += bias;
			debugOutput(instrRetired, "ADA", IP, AP, MemoryPtr[AP]);
			break;

		case (CMD_JZ):
		case (CMD_JZ_DOWN):
			IP = (MemoryPtr[AP] & JCC_MASK) ? IP : IP + bias;
			debugOutput(instrRetired, "JZ", IP, AP, MemoryPtr[AP]);
			break;
		case (CMD_JNZ):
		case (CMD_JNZ_DOWN):
			IP = (MemoryPtr[AP] & JCC_MASK) ? IP + bias : IP;
			debugOutput(instrRetired, "JNZ", IP, AP, MemoryPtr[AP]);
			break;
		default:
			fprintf(stderr, "IP:0x04lx Unknown Opcode: 0x%04lx", IP, MemoryPtr[IP]);
			break;
		}
		++instrRetired;

	} while (IP < AP_MAX);
stop:
	if (Statistic)
	{
		cerr << "\r\nIstructions_retired:" << instrRetired << "\r\n";
	}
	return status;
}


void help(int argc, char* argv[])
{
	cerr << "Usage:" << std::endl;
	cerr << argv[0] << " -f binary file [-p -x -s]" << std::endl;
	cerr << "\t-p - Enable protection mode [UNSUPPORTED]" << std::endl;
	cerr << "\t-x - set 16-bit mode" << std::endl;
	cerr << "\t-s - Enable instruction retired count" << std::endl;
	cerr << "\t-d - enable debug output" << std::endl;
	return;
}

uint16_t calcpi[] = {
	0x0000, 0x0000, 0x4400, 0x2016, 0x4001, 0x2007, 0x6008, 0x4001, 0x2001, 0x5fff, 0x3fff,
	0x4002, 0x2001, 0x5ffe, 0x9ff8, 0x6002, 0x3fff, 0x9ffe, 0x5fff, 0x6022, 0x4002,
	0x6005, 0x4004, 0x2001, 0x5ffc, 0x3fff, 0x9ffb, 0x4004, 0x6008, 0x3fff, 0x5ffc,
	0x2001, 0x5ffe, 0x3fff, 0x6004, 0x4006, 0x9ff8, 0x5ffa, 0x0000, 0x4006, 0x6009,
	0x6005, 0x5ffa, 0x2001, 0x4006, 0x3fff, 0x9ffb, 0x5ffa, 0x8006, 0x0000, 0x5ffb,
	0x2001, 0x5fff, 0x9fde, 0x0000, 0x400a, 0x2050, 0x1002, 0x2019, 0x1002, 0x3fd4,
	0x1002, 0x5ff7, 0x2030, 0x1002, 0x4009, 0x3ff1, 0x1002,
	0x5ff6, 0x3ffc, 0x1100, 0x6056, 0x3fff, 0x4001, 0x6002,
	0x3fff, 0x9ffe, 0x4001, 0x6005, 0x3fff, 0x5fff, 0x200a, 0x4001, 0x9ffb, 0x4001,
	0x6008, 0x4001, 0x2001, 0x5fff, 0x3fff, 0x4002, 0x2001, 0x5ffe, 0x9ff8, 0x4003,
	0x6002, 0x3fff, 0x9ffe, 0x5ffb, 0x6022, 0x4004, 0x6005, 0x4006, 0x2001, 0x5ffa,
	0x3fff, 0x9ffb, 0x4006, 0x6008, 0x3fff, 0x5ffa, 0x2001, 0x5ffc, 0x3fff, 0x6004,
	0x400a, 0x9ff8, 0x5ff6, 0x0000, 0x400a, 0x6009, 0x6005, 0x5ff6, 0x2001, 0x400a,
	0x3fff, 0x9ffb, 0x5ff6, 0x8006, 0x0000, 0x5ffb, 0x2001, 0x5ffb, 0x9fde, 0x0000,
	0x4004, 0x6005, 0x5ffd, 0x2001, 0x4003, 0x3fff, 0x9ffb, 0x4001, 0x2030, 0x1002,
	0x4001, 0x6002, 0x3fff, 0x9ffe, 0x5ffd, 0x6005, 0x5fff, 0x2001, 0x4001, 0x3fff,
	0x9ffb, 0x5ffc, 0x9faa, 0x200a, 0x1002, 0x2003, 0x1002, 0x1800 };

int main(int argc, char *argv[]) {
	int status = -1;
	int c = 0;
	char *filePath = NULL;
	while((c = getopt(argc, argv, "f:pxsd?hH")) != -1){
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
		case 'd':
			SetDebug(true);
			break;
		case 'H':
			SetHex(true);
			break;
		case '?':
		case 'h':
			help(argc, argv);
			return 0;
			break;
		}
	}
	/*if (filePath == NULL){
		cout << "Fatal error: add input file"<< endl;
		return -1;
	}*/
	uint16_t Memory[65536];
	memset(&Memory,0,sizeof(Memory));

	std::fstream File(filePath, std::fstream::in | std::fstream::binary);
	if (!File.good()){
		cerr << "Input file error, exiting"<<endl;
		return -1;
	}

	Image image(File, getHex());

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
