/*
 * Bf.cpp
 *
 *  Created on: 13 нояб. 2016 г.
 *      Author: radiolok
 */

#include "Bf.h"

Bf::Bf() {
	// TODO Auto-generated constructor stub

}

Bf::~Bf() {
	// TODO Auto-generated destructor stub
}

bool Bf::IsASymbol(uint8_t c){
	bool result = false;
	switch (c){
	case '>':
	case '<':
	case '+':
	case '-':
	case '.':
	case ',':
	case '[':
	case ']':
	case '~': //is a debug symbol
		result = true;
		break;
	default:

		break;
	}

	return result;
}

bool Bf::IsAIncrementer(uint8_t c){
	bool result = false;
	switch (c){
	case '<':
	case '>':
	case '+':
	case '-':
		result = true;
		break;
	default:
		break;
	}
	return result;
}

bool Bf::IsMemInc(uint8_t c){
	bool result = false;
	switch (c){
	case '>':
	case '<':
		result = true;
		break;
	default:

		break;
	}

	return result;
}


uint8_t Bf::Translate(const uint8_t *SourceBuffer, size_t length, std::vector<Cmd> &Output){
	uint8_t status = 0;

	uint8_t PreviousCmd = 0;
	uint8_t CurrentCmd = 0;
	size_t CurrentBias = 0;
	size_t LoopMarkIterator;

	for (size_t i = 0 ; i < length; ++i)
	 {
		 if (IsASymbol(SourceBuffer[i]))
		 {
			 CurrentCmd = SourceBuffer[i];
			 switch (CurrentCmd){
			 	 //For this Cmd's we need to calc shift:
			 	case '>':
				case '<':
				case '+':
				case '-':
					if (CurrentCmd == PreviousCmd){
						CurrentBias ++;
						if (i  == (length -1)){//Write last command:
							Output.push_back(Cmd(CurrentCmd, CurrentBias));
						}
					}
					else{//We got new command:
						Output.push_back(Cmd(CurrentCmd, CurrentBias));
						CurrentBias = 0;
					}
					break;
				case '.':
				case ',':
					Output.push_back(Cmd(CurrentCmd));
					CurrentBias = 0;
					break;
				case '[':
				case ']':
					std::string LoopMark = "Mark" + LoopMarkIterator++;
					Output.push_back(Cmd(CurrentCmd, LoopMark));
					CurrentBias = 0;
					break;
 			 }
			 PreviousCmd = CurrentCmd;
		 }
	 }
	return status;
}


uint8_t Bf::Compile(const uint8_t *SourceBuffer, size_t length, std::vector<Cmd> &Output){

	uint8_t status = 0;

	status = Translate(SourceBuffer, length, Output);

	return status;
}
