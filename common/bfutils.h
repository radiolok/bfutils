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
#define OPEN_SOURCE_ERROR   -4
#define OPEN_OUTPUT_ERROR   -5

#define BF_MAGIC 0x4246
#define BF_INDENT 0x10

#define MEMORY_PTR_MAX 65535

#define SECTION_CODE 0x01
#define SECTION_DATA 0x02


#define CORE_CMD_ADD_SUB 0x00
#define CORE_CMD_RIGHT_LEFT 0x01
#define CORE_CMD_INPUT 0x02
#define CORE_CMD_OUTPUT 0x03
#define CORE_CMD_JZ_UP_DOWN 0x04
#define CORE_CMD_JNZ_UP_DOWN 0x05


//Commands list:
//
//
#define CMD_ADD (0x0)
#define CMD_SUB (0x1)

#define CMD_RIGHT (0x2)
#define CMD_LEFT (0x03)

#define CMD_INPUT (0x4)

#define CMD_OUTPUT (0x6)
#define CMD_JZ_UP  (0x8)
#define CMD_JZ_DOWN (0x9)

#define CMD_JNZ_UP  (0xA)
#define CMD_JNZ_DOWN (0xB)

#define CMD_IP_SET   (0xC)
#define CMD_AP_SET (0xE)

#include <cstdint>
#include <cstddef>

enum class DataType { ByteData = 0, WordData = 1 };

typedef  uint8_t BF_STATUS;  
typedef  uint16_t  ADDRESS_TYPE;

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
