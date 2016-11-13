/* Copyright (c) 2016, Artem Kashkanov
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

#include <iostream>
#include <fstream>
#include <vector>
#include "cmd.h"
#include <getopt.h>
#include <string.h>
#include <linux/limits.h>
#include "Bf.h"

using namespace std;

vector<Cmd> CmdList;

bool IsABfSymbol(uint8_t c){
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

int main(int argc, char ** argv) {
	if (argc < 2){
		cerr << "Set source file to compile\r\n";
		return -1;
	}
	int c = 0;
	char InputPath[PATH_MAX] = {0x00};
	uint8_t SetMode = 0;
	while((c = getopt(argc, argv, "i:o:e")) != -1){
		switch(c){
		case 'i':
			strcpy(InputPath, optarg);
			break;
		case 'o':
			//output file name
			break;
		case 'e':
			SetMode = 1;
			break;
		}
	}
	if (InputPath[0]){
		std::ifstream SourceFile (argv[1], std::ifstream::binary);

		if (!SourceFile.good()){
			cerr << "Source File open error, exiting\r\n";
					return -2;
		}
		else{
			//get file length
			SourceFile.seekg (0, SourceFile.end);
		    size_t length = SourceFile.tellg();
		    SourceFile.seekg (0, SourceFile.beg);

		    uint8_t  *SourceBuffer = new uint8_t [length];

		    SourceFile.read((char *)SourceBuffer, length);

		    SourceFile.close();

		    vector<Cmd> Output;

			if (SetMode == 0){
				Bf *BfCompiler = new Bf;
				if (BfCompiler->Compile(SourceBuffer, length, Output)){
					cerr << "Compilation error\r\n";
					return -2;
				}

			}
			else{
				//TODO: Extended command set
			}
			//TODO: file Output save


			delete[] SourceBuffer;
		}


	}
    return 0;
}
