/*
 * cmd.cpp
 *
 *  Created on: 13 нояб. 2016 г.
 *      Author: radiolok
 */

#include "cmd.h"

Cmd::Cmd(uint16_t _bin){
	/* 000 - +-, if - bit 12 = 1
	 * 001 - <>, if > bit 12 = 1
	 * 010 - Console Input
	 * 011 - Console Output
	 * 100 - JZ
	 * 101 - JNZ
	 * 110 - LD_IP
	 * 111 - LD_AP
	 * */
	cmd = 0;
	uint16_t cmd_bin = ((_bin>>13) & 0x0007);
	bool sign = ((_bin >> 12)&0x01)? true: false;
	switch (cmd_bin){
	case 0:
		cmd = sign? '-':'+';
		break;
	case 1:
		cmd = sign? '<': '>';
		break;
	case 2:
		cmd = ',';
		break;
	case 3:
		cmd = '.';
		break;
	case 4:
		cmd = '[';
		break;
	case 5:
		cmd = ']';
		break;
	case 6:
		cmd = 'I';
		break;
	case 7:
		cmd = 'A';
		break;
	}
	bias = (_bin&0x0FFF);

}

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

uint8_t Cmd::Symbol(uint8_t _cmd){
	bool result = false;
	switch (_cmd){
	case '>':
		break;
	case '<':
		break;
	case '+':
		break;
	case '-':
		break;
	case '.':
		break;
	case ',':
		break;
	case '[':
		break;
	case ']':
		break;
	default:

		break;
	}

	return result;
}


uint16_t Cmd::GetCmd(void){
	uint16_t result = 0;

	/* 0 - 11 bits - shift
	 * 12 bit - substitute
	 * 13 - 15 bits - cmd
	 *
	 *
	 * */

	/* 000 - +-, if - bit 12 = 1
	 * 001 - <>, if > bit 12 = 1
	 * 010 - Console Input
	 * 011 - Console Output
	 * 100 - JZ
	 * 101 - JNZ
	 * 110 - LD_IP
	 * 111 - LD_AP
	 * */
	switch (cmd){
	case '>':
		result = (1 << 13) | (abs(bias) & 0x0FFF);
		break;
	case '<':
		result = (1 << 13) | (1 << 12) | (abs(bias) & 0x0FFF);
		break;
	case '+':
		result = (abs(bias) & 0x0FFF);
		break;
	case '-':
		result = (0x1 << 12) | (abs(bias) & 0x0FFF);//Sign
		break;
	case '.':
		result = (0x3 << 13);
		break;
	case ',':
		result = (0x2 << 13);
		break;
	case '[':
		result = (0x4 << 13) |  (abs(bias) & 0x0FFF) | ((bias < 0)? (1<<12) : 0);
		break;
	case ']':
		result = (0x5 << 13)|  (abs(bias) & 0x0FFF) | ((bias < 0)? (1<<12) : 0);
		break;
	}
	return result;
}
