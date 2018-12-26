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

#ifndef CMD_H_
#define CMD_H_

#include <stdint.h>
#include <inttypes.h>
#include <string>
#include <stdlib.h>

#include "bfutils.h"

class Cmd {
private:
		uint8_t cmd;
		int16_t bias;
		std::string name;

public:
	Cmd(uint16_t _bin);
	Cmd(uint8_t _cmd);
	Cmd (uint8_t _cmd, int16_t _bias);
	Cmd (uint8_t _cmd, std::string _name);
	Cmd(uint8_t _cmd, int16_t _bias, std::string _name);
	virtual ~Cmd();

	void SetBias(int16_t _bias){bias = _bias;};
	int16_t GetBias(void){return bias;};

	void SetName(const std::string &_name){name = _name;};
	std::string GetName(void){return name;};

	uint8_t Symbol(uint8_t _cmd);
	uint8_t GetCmdChar(){return cmd;};
	uint16_t GetCmd(void);

};

#endif /* CMD_H_ */
