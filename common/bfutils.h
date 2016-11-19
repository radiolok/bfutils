/*
 * df_definitions.h
 *
 *  Created on: 14 нояб. 2016 г.
 *      Author: radiolok
 */

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
	WordToBigEndian_t FileBase;//Relative shift from the beginning of file
	WordToBigEndian_t MemoryBase;//Physical address to load
	WordToBigEndian_t FileSize;//How much Bytes of data can be loaded to memory
	WordToBigEndian_t MemorySize;//How much Words in Memory should be allocated
}BfSection_t;

typedef struct _BfHeader_t{
	WordToBigEndian_t Magic;//BF should contain "BF" symbols
	uint8_t Machine;//8bit of 16bit mode
	uint8_t HeaderSize;
	uint8_t SectionNum;
	uint8_t flags;
	WordToBigEndian_t IpEntry;//IP start address
	WordToBigEndian_t ApEntry;//AP start address 
} BfHeader_t;



void swapLEtoBE(WordToBigEndian_t *array, size_t size);
void swapLEtoBE(WordToBigEndian_t *word);

#endif /* DF_DEFINITIONS_H_ */
