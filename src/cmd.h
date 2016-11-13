/*
 * cmd.h
 *
 *  Created on: 13 нояб. 2016 г.
 *      Author: radiolok
 */

#ifndef CMD_H_
#define CMD_H_

#include <stdint.h>
#include <inttypes.h>
#include <string>
#include <stdlib.h>

class Cmd {
private:
		uint8_t cmd;
		int16_t bias;
		std::string name;

public:
		Cmd(uint8_t _cmd);
		Cmd (uint8_t _cmd, int16_t _bias);
		Cmd (uint8_t _cmd, std::string _name);
		Cmd(uint8_t _cmd, int16_t _bias, std::string _name);
	virtual ~Cmd();

	void SetBias(int16_t _bias){bias = _bias;};
	int16_t GetBias(void){return bias;};

	void SetName(const std::string &_name){name = _name;};
	std::string GetName(void){return name;};

	uint16_t GetCmd(void);

};

#endif /* CMD_H_ */
