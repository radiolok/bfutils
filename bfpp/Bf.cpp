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

#include "Bf.h"

Bf::Bf(const compiler_options_t &_options) : options(_options){
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
	uint8_t status = SUCCESS;

	uint8_t CurrentCmd = 0;
	size_t CurrentBias = 0;
	size_t LoopMarkIterator;
	//Need to clear out unused symbols:
	size_t new_length = 0;
	uint8_t *NewBuffer = new uint8_t[length];
	for (size_t i = 0 ; i < length; ++i){
		if (IsASymbol(SourceBuffer[i])){
			NewBuffer[new_length] = SourceBuffer[i];
			++new_length;
		}
	}
	//Start translating
	for (size_t i = 0 ; i < new_length; ++i)
	 {
		 if (IsASymbol(NewBuffer[i]))
		 {
			 CurrentCmd = NewBuffer[i];
			 switch (CurrentCmd){
			 	 //For this Cmd's we need to calc shift:
			 	case '>':
				case '<':
				case '+':
				case '-':
					CurrentBias++;
					if (i < new_length -1){
						if (CurrentCmd != NewBuffer[i+1]){
							Output.push_back(Cmd(CurrentCmd, CurrentBias));
							CurrentBias = 0;
						}
					}
					else{
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
		 }
	 }
	return status;
}

size_t Bf::FindLoopEnd(std::vector<Cmd> &Output, size_t CurrentIp){
	size_t i = 1;
	size_t NewIp = CurrentIp;
	size_t MaxIp = Output.size();
	while (i){
		NewIp++;
		if (NewIp >=MaxIp){
			return 0;
		}
		switch(Output[NewIp].GetCmdChar()){
		case '[':
			i++;
			break;
		case ']':
			i--;
			break;
		}
	}
	//We found loop ending:
	Output[NewIp].SetBias(-(NewIp-CurrentIp));
	return NewIp;
}


uint8_t Bf::Linking(std::vector<Cmd> &Output){
	uint8_t status = SUCCESS;
	//Let's Get Pointers for loops:
	//[ should get shift > 0
	//] should get shift < 0
	size_t CurrentIp = 0;
	size_t MaxIp = Output.size();
	if (MaxIp == CurrentIp){
		return LINKING_ERROR;
	}

	for (CurrentIp = 0; CurrentIp < MaxIp; CurrentIp++){
		if (Output[CurrentIp].GetCmdChar() == '['){
			//Let's find Loop Closing:
			size_t CloseIp = FindLoopEnd(Output, CurrentIp);
			if (CloseIp){
				Output[CurrentIp].SetBias(CloseIp-CurrentIp);
			}
			else{
				return LINKING_ERROR;
			}
		}
	}

	return status;
}

uint8_t Bf::Compile(const uint8_t *SourceBuffer, size_t length, std::vector<Cmd> &Output){

	uint8_t status = SUCCESS;

	status = Translate(SourceBuffer, length, Output);
	if (status){
		return TRANSLATION_ERROR;
	}

	status = Linking(Output);
	if (status){
		return LINKING_ERROR;
	}
	return status;
}

