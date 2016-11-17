//============================================================================
// Name        : brainfuck_rpc_emulator.cpp
// Author      : Artem Kashkanov
// Version     :
// Copyright   : 
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <cstring>

#include "Console.h"


#include <fstream>

#include <vector>

#include "bfutils.h"

using namespace std;

uint8_t  LoadImage(std::ifstream &File, BfHeader_t *Header, uint16_t *MemoryPtr){
	uint8_t status = 0;

	File.read(reinterpret_cast<char *>(Header), sizeof(BfHeader_t));

	BfSection_t Code;
	Code.Position.Byte.high = Header->Code.Position.Byte.high;
	Code.Position.Byte.low = Header->Code.Position.Byte.low;

	Code.Length.Byte.high = Header->Code.Length.Byte.high;
	Code.Length.Byte.low = Header->Code.Length.Byte.low;

	BfSection_t Data;
	Data.Position.Byte.high = Header->Data.Position.Byte.high;
	Data.Position.Byte.low = Header->Data.Position.Byte.low;

	Data.Length.Byte.high = Header->Data.Length.Byte.high;
	Data.Length.Byte.low = Header->Data.Length.Byte.low;

	File.read(reinterpret_cast<char *>(MemoryPtr+Code.Position.Word), sizeof(uint16_t)*Code.Length.Word);
	if (Data.Length.Word != 0){
		File.read(reinterpret_cast<char *>(MemoryPtr+Data.Position.Word), sizeof(uint16_t)*Data.Length.Word);
	}

	return status;
}

uint8_t LoadBF(char *fName, BfHeader_t *Header, uint16_t *MemoryPtr){
	uint8_t status = 0;
	std::ifstream File(fName, std::ifstream::binary);
	if (!File.good()){
		return -1;
	}
	LoadImage(File, Header, MemoryPtr);
	return status;
}

bool SegFault(const BfSection_t &section, uint16_t Ptr){
	bool result = false;
	if ((Ptr < section.Position.Word) ||(Ptr > section.Position.Word + section.Length.Word)){
		result = true;
	}
	return result;
}

uint8_t ExecCmd(BfHeader_t *Header, uint16_t *MemoryPtr, uint16_t &IP, uint16_t &AP){
	uint8_t status = SUCCESS;
	uint16_t cmd = MemoryPtr[IP];
	uint16_t cmd_bin = ((cmd>>13) & 0x0007);
	bool sign = ((cmd >> 12)&0x01)? true: false;
	uint16_t bias = (cmd&0x0FFF);

	switch (cmd_bin){
	case 0:
		MemoryPtr[AP] = sign? MemoryPtr[AP] - bias: MemoryPtr[AP] + bias;
		break;
	case 1:
		AP = sign? AP - bias: AP + bias;
		if (SegFault(Header->Data, AP)){
			return -1;
		}
		break;
	case 2:
		MemoryPtr[AP] = In();
		break;
	case 3:
		Out(MemoryPtr[AP]);
		break;
	case 4:
		IP = MemoryPtr[AP]? IP : (sign? IP - bias: IP + bias);
		if (SegFault(Header->Data, IP)){
			return -1;
		}
		break;
	case 5:
		IP = MemoryPtr[AP]? (sign? IP - bias: IP + bias) : IP;
		if (SegFault(Header->Data, IP)){
			return -1;
		}
		break;
	case 6:
		IP = bias;
		if (SegFault(Header->Data, IP)){
			return -1;
		}
		break;
	case 7:
		AP = Header->Data.Position.Word + bias;
		if (SegFault(Header->Data, AP)){
			return -1;
		}
		break;
	}
	return status;
}

uint8_t ExecCode(BfHeader_t *Header, uint16_t *MemoryPtr){
	uint8_t status = 0;

	uint16_t IP = Header->Code.Position.Word;
	uint16_t AP = Header->Data.Position.Word;

	do {

		status = ExecCmd(Header,MemoryPtr, IP, AP);
		if (status){
			return -1;
		}

		IP++;
	}while (IP < Header->Code.Position.Word + Header->Code.Length.Word);

	return status;
}


int main(int argc, char *argv[]) {
	int status = -1;
	int c = 0;
	char filePath[4096] = {0x00};
	while((c = getopt(argc, argv, "f:c:di")) != -1){
		switch(c)
		{
		case 'f':
			strcpy(filePath,optarg);
			break;
		}
	}
	if (*filePath == 0){
		cout << "Fatal error: add input file"<< endl;
		return -1;
	}
	BfHeader_t *Header = new BfHeader_t;
	uint16_t Memory[65536];
	memset(&Memory,0,sizeof(Memory));
	status = LoadBF(filePath,Header,Memory);
	if (status){
		return -1;
	}
	ExecCode(Header, Memory);

	return 0;
}
