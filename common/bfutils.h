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

#ifndef DF_DEFINITIONS_H_
#define DF_DEFINITIONS_H_

#define SUCCESS              0
#define ERROR               -1
#define TRANSLATION_ERROR   -2
#define LINKING_ERROR 		-3
#define OPTIMIZATION_ERROR  -4
#define OPEN_SOURCE_ERROR   -5
#define OPEN_OUTPUT_ERROR   -6

#define BF_MAGIC 0x4246
#define BF_INDENT 0x10

#define MEMORY_PTR_MAX 65535

#define SECTION_CODE 0x01
#define SECTION_DATA 0x02

//Commands list:
//
//
#define CMD_NOP (0x0)

#define CMD_IO (0x1000) //0x0C01
#define CMD_INPUT_MASK (0x0001)
#define CMD_OUTPUT_MASK (0x0002)
#define CMD_CTRLIO_SYNC (0x0004)
#define CMD_CTRLIO_CLR_DATA (0x0010)
#define CMD_CTRLIO_CLR_AP (0x0020)
#define CMD_CTRLIO_MODE8 (0x0200)
#define CMD_CTRLIO_MODE16 (0x0400)
#define CMD_CTRLIO_HALT (0x0800)


#define CMD_ADD (0x2000)
#define CMD_SUB (0x3000)

#define CMD_RIGHT (0x4000)
#define CMD_LEFT (0x5000)

#define CMD_JZ  (0x6000)
#define CMD_JZ_DOWN (0x7000)

#define CMD_JNZ  (0x8000)
#define CMD_JNZ_DOWN (0x9000)

#define CMD_XOR (0xA000)
//#define CMD_XOR_NEG (0x0B000)

//#define FREE_CODE (0xC000)
//#define FREE_CODE (0xD000)
//#define FREE_CODE (0xE000)

#define CMD_HALT (0xF000)

#include <cstdint>
#include <cstddef>

enum class DataType { ByteData = 0, WordData = 1 };

typedef  uint8_t BF_STATUS;  
typedef  uint16_t  ADDRESS_TYPE;

typedef struct __compiler_options_t
{
	char *InputPath;
	char *OutputPath;

	/*
	-O1 - adding operations join together
	*/
	int OptimizationLevel;
	/*
	-e
	if true - full instruction set is supported
	if false - only basic insturction set
	*/
	bool SetCompilerMode;
	/*
	-s
	save in ascii format
	*/
	bool SaveBinaryAsText;

	/*
	-H
	save binary as HEX
	*/
	bool SaveBinaryAsHex;
	/*
	-d
	print debug assembly
	*/
	bool DebugSymbols;	

	/*
	-l
	if true, debug output would be aligned with spaces, depending on current loop stack*/
	bool DebugLoopShifting;
}compiler_options_t;

typedef union  _WordToBigEndian_t{
	uint16_t Word;
	struct {
		uint8_t low;
		uint8_t high;
	}Byte;
}WordToBigEndian_t;

typedef struct _BfSection_t{
	uint16_t FileBase;//Relative shift from the beginning of file
	uint16_t MemoryBase;//Physical address to load
	uint16_t FileSize;//How much Bytes of data can be loaded to memory
	uint16_t MemorySize;//How much Words in Memory should be allocated
	uint8_t type;
	uint8_t pad;
}BfSection_t;

typedef struct _BfHeader_t{
	uint16_t Magic;//BF should contain "BF" symbols
	uint8_t Machine;//8bit of 16bit mode
	uint8_t HeaderSize;
	uint8_t SectionNum;
	uint8_t flags;
	uint16_t IpEntry;//IP start address
	uint16_t ApEntry;//AP start address 
} BfHeader_t;



void swapLEtoBE(void *array, size_t size);
void swapLEtoBE(void *word);

#endif /* DF_DEFINITIONS_H_ */
