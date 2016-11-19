//============================================================================
// Name        : brainfuck_rpc_emulator.cpp
// Author      : Artem Kashkanov
// Version     :
// Copyright   : 
// Description : Hello World in C++, Ansi-style
//============================================================================

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



uint8_t ExecCmd(Image &image, uint16_t *MemoryPtr, uint16_t &IP, uint16_t &AP){
	uint8_t status = SUCCESS;
	uint16_t cmd = MemoryPtr[IP];
	uint16_t cmd_bin = ((cmd>>13) & 0x0007);
	bool sign = ((cmd >> 12)&0x01)? true: false;
	uint16_t bias = (cmd&0x0FFF);
	switch (cmd_bin){
	case 0:// '<' and '>' commands
		MemoryPtr[AP] = sign? MemoryPtr[AP] - bias: MemoryPtr[AP] + bias;
		break;
	case 1:// '+' and '-' commands
		AP = sign? AP - bias: AP + bias;
		break;
	case 2://Console input cmd
		if (GetWordMode()){
			MemoryPtr[AP] = In();		
		}
		else{
			MemoryPtr[AP] = In() & 0xFF;
		}
		break;
	case 3://console output cmd
		if (GetWordMode()){
			Out(MemoryPtr[AP]);
		}
		else{
			Out(MemoryPtr[AP] & 0xFF);
		}
		break;
	case 4://Jump If Zero
		if (GetWordMode()){
			IP = MemoryPtr[AP]? IP : (sign? IP - bias: IP + bias);
		}
		else{
			IP = (MemoryPtr[AP] & 0xFF)? IP : (sign? IP - bias: IP + bias);
		}
		break;
	case 5://Jump If not zero
		if (GetWordMode()){
			IP = MemoryPtr[AP]? (sign? IP - bias: IP + bias) : IP;
		}
		else{
			IP = (MemoryPtr[AP] & 0xFF)? (sign? IP - bias: IP + bias) : IP;
		}
		break;
	case 6://Set IP
		IP = bias;
		break;
	case 7://Set AP
		AP =  bias;
		break;
	}
	return status;
}

uint8_t ExecCode(Image &image, uint16_t *MemoryPtr){
	uint8_t status = 0;

	ADDRESS_TYPE IP = image.GetIpEntry();
	ADDRESS_TYPE AP = image.GetApEntry();

	size_t i = 0;
	do {
		i++;
		status = ExecCmd(image,MemoryPtr, IP, AP);
		if (status){
			return -1;
		}

		IP++;
	}while (IP < image.GetSection(0).GetMemoryBase().Word + image.GetSection(0).GetMemorySize().Word );
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
	std::fstream File(filePath, std::fstream::binary);
	if (!File.good()){
		return -1;
	}

	Image image(File, Memory);
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
