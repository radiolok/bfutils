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
#include "bfutils.h"
#include <Image.h>
#include <iomanip>
#include <sstream>

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

string GetDebugSymbol(Cmd cmd){
	std::stringstream result;
	switch (cmd.GetCmdChar()){
	case '>':
		result << "\tPTR(" << cmd.GetBias() << ")";
		break;
	case '<':
		result << "\tPTR(-" << cmd.GetBias() << ")";
		break;
	case '+':
		result << "\tADD(" << cmd.GetBias() << ")";
		break;
	case '-':
		result << "\tADD(-" << cmd.GetBias() << ")";
		break;
	case '.':
		result << "\tPRINT";
		break;
	case ',':
		result << "\tGET";
		break;
	case '[':
		result << "\tJZ(" << cmd.GetBias() << ")";
		break;
	case ']':
		result << "\tJNZ(" << cmd.GetBias() << ")";
		break;
	default:

		break;
	}
	return result.str();
}


uint8_t SaveOutput(std::vector<Cmd> &Output, bool binaryastext, const char *path, bool DebugSymbols){
	uint8_t status = 0;
	if (binaryastext){
		std::fstream OutputFile (path, std::fstream::out);
		if (!OutputFile.good()){
			cerr << "Output debug File open error, exiting\r\n";
			return OPEN_OUTPUT_ERROR;
		}
		else{
			size_t ip = 0;
			//copy data:
			for (auto iter = Output.begin(); iter < Output.end(); ++iter, ++ip){
			//Save:
				OutputFile << "IP:0x" << setfill('0') << setw(4) << hex << ip << " "
						<< "\tCMD:0x"  << setfill('0') << setw(4) << hex << iter->GetCmd()
				<< (DebugSymbols? GetDebugSymbol(*iter) : "") << endl;
			}
			OutputFile.close();
		}


	}
	else{
		std::fstream OutputFile;
		OutputFile.open(path, std::fstream::out | std::fstream::trunc);
		OutputFile.close();
		OutputFile.open(path, std::fstream::in | std::fstream::out | std::fstream::binary);
		if (!OutputFile.good()){
			cerr << "Output File open error, exiting\r\n";
			return OPEN_OUTPUT_ERROR;
		}
		else{
			
			Image *image = new Image(static_cast<uint8_t>(DataType::ByteData));

			//Add code Section:
			//Default placement is IP = 0:
			Section codesection(Output, 0, Output.size());
			image->AddSection(codesection);
			//image->AddSection(Section(Output, 0, Output.size()));
			
			//Add Data section.
			//By default it has no static elements
			//Default placement is after code segment:
			//Default length = all free memory
			//Default AP - is the middle of limits
			vector<uint16_t> Data;

			Section datasection(Data, Output.size(), MEMORY_PTR_MAX - Output.size());
			image->AddSection(datasection);
			//image->AddSection(Section(Data, Output.size(), MEMORY_PTR_MAX - Output.size()));
					
			image->SetIpEntry(0);
			image->SetApEntry(MEMORY_PTR_MAX/2 - Output.size()/2);

			image->Write(OutputFile);

			//Save:
			OutputFile.close();
		}

	}

	return status;
}

int main(int argc, char ** argv) {
	if (argc < 2){
		cerr << "Usage:\r\n";
		cerr << argv[0] << " -i source file [-o output file] [-e -s -d]\r\n";
		cerr << "\t-e - enable extended instruction set[UNSUPPORTED]\r\n";
		cerr << "\t-s - save output binary in text format\r\n";
		cerr << "\t-d - enable debug symbols for text format\r\n";
		cerr << "Set source file to compile\r\n";
		return -1;
	}
	int c = 0;
	char *InputPath = NULL;
	char *OutputPath = NULL;
	bool SetCompilerMode = false;
	bool SaveBinaryAsText = false;
	bool DebugSymbols = false;
	while((c = getopt(argc, argv, "i:o:esd")) != -1){
		switch(c){
		case 'i':
			InputPath = optarg;
			break;
		case 'o':
			OutputPath = optarg;
			break;
		case 'e':
			SetCompilerMode = true;
			break;
		case 's':
			SaveBinaryAsText = true;
			break;
		case 'd':
			DebugSymbols = true;
			break;

		}
	}
	if (InputPath[0]){
		std::ifstream SourceFile (InputPath, std::ifstream::binary);

		if (!SourceFile.good()){
			cerr << "Source File"<< InputPath <<" open error, exiting\r\n";
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

			if (SetCompilerMode == 0){
				Bf *BfCompiler = new Bf;
				if (BfCompiler->Compile(SourceBuffer, length, Output)){
					cerr << "Compilation error\r\n";
					return -2;
				}

			}
			else{
				//TODO: Extended command set
			}
			SaveOutput(Output, SaveBinaryAsText, ((*OutputPath)? OutputPath :
					(SaveBinaryAsText? "a.asm": "a.out")), DebugSymbols);


			delete[] SourceBuffer;
		}


	}
    return 0;
}
