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
#include <unistd.h>
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

bool GetWordMode(void){
     return WordModeEnabled;
}


uint8_t ExecCode(Image &image, uint16_t *MemoryPtr){
	uint8_t status = 0;

	ADDRESS_TYPE IP = image.GetIpEntry();
	ADDRESS_TYPE AP = image.GetApEntry();

	ADDRESS_TYPE AP_MAX =  image.GetSection(0).GetMemoryBase() + image.GetSection(0).GetMemorySize() ;
	size_t i = 0;
	if (!GetWordMode()){
		uint16_t bias = 0;	
		do {
			bias = ( MemoryPtr[IP] & 0x0FFF);
			switch(MemoryPtr[IP] & 0xF000){
				case (CMD_ADD << 12):
					MemoryPtr[AP] += bias;
					break;
				case (CMD_SUB << 12):
					MemoryPtr[AP] -= bias;
					break;
				case (CMD_RIGHT << 12):
					AP +=bias;
					break;
				case (CMD_LEFT << 12):
					AP -=bias;
					break;
				case (CMD_INPUT << 12):
				       MemoryPtr[AP] = In() & 0xFF;
					break;
				case (CMD_OUTPUT << 12):
					Out(MemoryPtr[AP] & 0xFF);
					break;
				case (CMD_JZ_UP << 12):
					IP = (MemoryPtr[AP] & 0xFF)? IP : IP + bias;
					break;
				case (CMD_JZ_DOWN << 12):	
					IP = (MemoryPtr[AP] & 0xFF)? IP : IP - bias;
					break;
				case (CMD_JNZ_UP << 12):
					IP = (MemoryPtr[AP] & 0xFF)? IP + bias : IP;
					break;
				case (CMD_JNZ_DOWN << 12):	
					IP = (MemoryPtr[AP] & 0xFF)? IP - bias : IP;
					break;
				case (CMD_IP_SET << 12):
					IP = bias;
					break;
				case (CMD_AP_SET << 12):
					AP = bias;
					break;
			}
			++i;
			++IP;
		}while (IP < AP_MAX);
	
	}
	else{
	
		uint16_t bias = 0;	
		do {
			bias = ( MemoryPtr[IP] & 0x0FFF);
			switch(MemoryPtr[IP] & 0xF000){
				case (CMD_ADD << 12):
					MemoryPtr[AP] += bias;
					break;
				case (CMD_SUB << 12):
					MemoryPtr[AP] -= bias;
					break;
				case (CMD_RIGHT << 12):
					AP +=bias;
					break;
				case (CMD_LEFT << 12):
					AP -=bias;
					break;
				case (CMD_INPUT << 12):
				       MemoryPtr[AP] = In() & 0xFFFF;
					break;
				case (CMD_OUTPUT << 12):
					Out(MemoryPtr[AP] & 0xFFFF);
					break;
				case (CMD_JZ_UP << 12):
					IP = (MemoryPtr[AP] & 0xFFFF)? IP : IP + bias;
					break;
				case (CMD_JZ_DOWN << 12):	
					IP = (MemoryPtr[AP] & 0xFFFF)? IP : IP - bias;
					break;
				case (CMD_JNZ_UP << 12):
					IP = (MemoryPtr[AP] & 0xFFFF)? IP + bias : IP;
					break;
				case (CMD_JNZ_DOWN << 12):	
					IP = (MemoryPtr[AP] & 0xFFFF)? IP - bias : IP;
					break;
				case (CMD_IP_SET << 12):
					IP = bias;
					break;
				case (CMD_AP_SET << 12):
					AP = bias;
					break;
			}
			++i;
			++IP;
		}while (IP < AP_MAX);

	}
	cerr << "\r\nIstructions_retired:" << i << "\r\n";
	return status;
}


int main(int argc, char *argv[]) {
	int status = -1;
	int c = 0;
	char *filePath = NULL;
	while((c = getopt(argc, argv, "f:px")) != -1){
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
