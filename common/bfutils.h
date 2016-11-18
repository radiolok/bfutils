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

typedef union  _WordToBigEndian_t{
	uint16_t Word;
	struct {
		uint8_t low;
		uint8_t high;
	}Byte;
}WordToBigEndian_t;

typedef struct _BfSection_t{
	WordToBigEndian_t Base;
	WordToBigEndian_t Length;
	WordToBigEndian_t Position;
	WordToBigEndian_t Flags;
}BfSection_t;

typedef struct _BfHeader_t{
	WordToBigEndian_t Magic;
	WordToBigEndian_t Machine;
	BfSection_t Code;
	BfSection_t Data;
} BfHeader_t;

#endif /* DF_DEFINITIONS_H_ */
