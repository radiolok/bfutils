/*
 * cmd.cpp
 *
 *  Created on: 13 нояб. 2016 г.
 *      Author: radiolok
 */

#include "cmd.h"


Cmd::Cmd(uint8_t _cmd):cmd(_cmd), bias(0){

}
Cmd::Cmd (uint8_t _cmd, int16_t _bias) : cmd(_cmd), bias(_bias){

}
Cmd::Cmd (uint8_t _cmd, std::string _name): cmd(_cmd), bias(0), name(_name){

}
Cmd::Cmd(uint8_t _cmd, int16_t _bias, std::string _name): cmd(_cmd), bias(_bias), name(_name){

}

Cmd::~Cmd() {
	// TODO Auto-generated destructor stub
}


uint16_t Cmd::GetCmd(void){
	uint16_t result = 0;

	/* 0 - 11 bits - shift
	 * 12 bit - substitute
	 * 13 - 15 bits - cmd
	 *
	 *
	 * */
	result = ((cmd&0x07) << 13) | (abs(bias) & 0x0FFF) | (bias < 0? (1<<12) : 0);

	return result;
}
