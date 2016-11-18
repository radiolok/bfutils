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

uint8_t GetHeader(std::vector<Cmd> &Output, std::vector<WordToBigEndian_t> &Data, BfHeader_t &Header){
	uint8_t status = SUCCESS;
	Header.Magic.Word = BF_MAGIC;
	Header.Machine.Word = 0;

	Header.Code.Base.Word = 0;
	Header.Code.Length.Word = Output.size();
	Header.Code.Position.Word = 0;
	Header.Code.Flags.Word = 0;

	Header.Data.Base.Word = Header.Code.Base.Word + Header.Code.Length.Word + BF_INDENT;
	Header.Data.Length.Word = Data.size();
	Header.Data.Position.Word = Header.Code.Base.Word + Header.Code.Length.Word + BF_INDENT;
	Header.Data.Flags.Word = 0;

	return status;
}

uint8_t WriteHeader(std::ofstream &File, BfHeader_t &BfHeader){
	uint8_t status = SUCCESS;
	File << BfHeader.Magic.Byte.high << BfHeader.Magic.Byte.low;
	File << BfHeader.Machine.Byte.high << BfHeader.Machine.Byte.low;
	
	File << BfHeader.Code.Base.Byte.high << BfHeader.Code.Base.Byte.low;
	File << BfHeader.Code.Length.Byte.high << BfHeader.Code.Length.Byte.low;
	File << BfHeader.Code.Position.Byte.high << BfHeader.Code.Position.Byte.low;
	File << BfHeader.Code.Flags.Byte.high << BfHeader.Code.Flags.Byte.low;
	
	File << BfHeader.Data.Base.Byte.high << BfHeader.Data.Base.Byte.low;
	File << BfHeader.Data.Length.Byte.high << BfHeader.Data.Length.Byte.low;
	File << BfHeader.Data.Position.Byte.high << BfHeader.Data.Position.Byte.low;
	File << BfHeader.Data.Flags.Byte.high << BfHeader.Data.Flags.Byte.low;
	
	return status;
}

uint8_t SaveOutput(std::vector<Cmd> &Output, bool binaryastext, const char *path, bool DebugSymbols){
	uint8_t status = 0;
	if (binaryastext){
		std::ofstream OutputFile (path, std::ofstream::out);
		if (!OutputFile.good()){
			cerr << "Output File open error, exiting\r\n";
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
		std::ofstream OutputFile (path, std::ofstream::binary);
		if (!OutputFile.good()){
			cerr << "Output File open error, exiting\r\n";
			return OPEN_OUTPUT_ERROR;
		}
		else{
			vector<WordToBigEndian_t> Data;
			BfHeader_t BfHeader;

			GetHeader(Output, Data, BfHeader);

			WriteHeader(OutputFile, BfHeader);

			uint16_t *buffer = new uint16_t [Output.size()];
			//copy data:
			WordToBigEndian_t cmd;
			for (auto iter = Output.begin(); iter < Output.end(); ++iter, ++buffer){
				//*buffer = iter->GetCmd();
				cmd.Word = iter->GetCmd();
				OutputFile.write(reinterpret_cast<const char *>(&cmd.Byte.high), sizeof(cmd.Byte.high));
				OutputFile.write(reinterpret_cast<const char *>(&cmd.Byte.low), sizeof(cmd.Byte.low));
			}
			//Save:
//			OutputFile.write((char*)buffer, sizeof(uint16_t)*Output.size());
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
	char InputPath[PATH_MAX] = {0x00};
	char OutputPath[PATH_MAX] = {0x00};
	bool SetCompilerMode = false;
	bool SaveBinaryAsText = false;
	bool DebugSymbols = false;
	while((c = getopt(argc, argv, "i:o:esd")) != -1){
		switch(c){
		case 'i':
			strcpy(InputPath, optarg);
			break;
		case 'o':
			strcpy(OutputPath, optarg);
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
