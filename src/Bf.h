/*
 * Bf.h
 *
 *  Created on: 13 нояб. 2016 г.
 *      Author: radiolok
 */

#ifndef BF_H_
#define BF_H_

#include <inttypes.h>
#include <vector>
#include "cmd.h"

#include "df_definitions.h"

class Bf {
private:
	bool IsASymbol(uint8_t c);

	bool IsAIncrementer(uint8_t c);

	bool IsMemInc(uint8_t c);

	uint8_t Translate(const uint8_t *SourceBuffer, size_t length, std::vector<Cmd> &Output);

	uint8_t Linking(std::vector<Cmd> &Output);

	size_t FindLoopEnd(std::vector<Cmd> &Output, size_t CurrentIp);


public:
	uint8_t Compile(const uint8_t *SourceBuffer, size_t length, std::vector<Cmd> &Output);

	Bf();
	virtual ~Bf();
};

#endif /* BF_H_ */
