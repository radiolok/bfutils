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

#ifndef BF_H_
#define BF_H_

#include <inttypes.h>
#include <vector>
#include "cmd.h"
#include "iostream"
#include "bfutils.h"

class Bf {
private:
	bool IsASymbol(uint8_t c);

	bool IsAIncrementer(uint8_t c);

	bool IsMemInc(uint8_t c);

	uint8_t Translate(const uint8_t *SourceBuffer, size_t length, std::vector<Cmd> &Output);

	uint8_t Linking(std::vector<Cmd> &Output);

	size_t FindLoopEnd(std::vector<Cmd> &Output, size_t CurrentIp);


	const compiler_options_t &options;

public:
	uint8_t Compile(const uint8_t *SourceBuffer, size_t length, std::vector<Cmd> &Output);

	Bf(const compiler_options_t &_options);
	virtual ~Bf();
};

#endif /* BF_H_ */
