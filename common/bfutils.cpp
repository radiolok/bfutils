#include "bfutils.h"



void swapLEtoBE(WordToBigEndian_t *word){

	uint8_t  tmp = 0;
	tmp = word->Byte.low;
	word->Byte.low = word->Byte.high;
	word->Byte.high = tmp;

}

void swapLEtoBE(WordToBigEndian_t *array, size_t size){

	uint8_t  tmp = 0;
	for (size_t i = 0; i < size/sizeof(uint16_t); ++i){
		tmp = array[i].Byte.low;
		array[i].Byte.low = array[i].Byte.high;
		array[i].Byte.high = tmp;
	}

}
