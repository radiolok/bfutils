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

#include <iostream>
#include <fstream>
#include <vector>
#include "cmd.h"

#include <string.h>
#include "Bf.h"
#include "bfutils.h"
#include <Image.h>
#include <iomanip>
#include <sstream>
#include <getopt.h>

#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif


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

size_t loopLevel = 0;

string GetDebugSymbol(Cmd cmd, bool loopShift = false){
	std::stringstream result;
	result << ' ';
	if (loopShift)
	{
		if (loopLevel < 10)
		{			
			std::string blanks(loopLevel, '|');
			result << blanks;
		}
		else{
			std::string blanks(8, '|');
			result << loopLevel << ">>" << blanks;
		}
	}
	


	switch (cmd.GetCmdChar()){
	case '>':
		result << "AP += " <<abs(cmd.GetBias());
		break;
	case '<':
		result << "AP -= " << abs(cmd.GetBias());
		break;
	case '+':
		result << "*AP += " << abs(cmd.GetBias());
		break;
	case '-':
		result << "*AP -= " << abs(cmd.GetBias());
		break;
	case '.':
		result << "putc";
		break;
	case ',':
		result << "getc";
		break;
	case '[':
		loopLevel++;
		result << "(*AP==0)? IP " << ((cmd.GetBias()>0)? "+= " : "-= ") << abs(cmd.GetBias()) << ": PASS";
		break;
	case ']':
		loopLevel--;
		result << "(*AP!=0)? IP " << ((cmd.GetBias()>0)? "+= " : "-= ")  << abs(cmd.GetBias()) << ": PASS";
		break;
	case '0':
		result << "*AP = 0" ;
		break;
	default:
		fprintf(stderr, "Unknown Opcode: %c\n", cmd.GetCmdChar());
		break;
	}
	return result.str();
}


uint8_t SaveOutput(std::vector<Cmd> &Output,  const compiler_options_t &options){
	uint8_t status = 0;
	char path[PATH_MAX];
	if (options.OutputPath == NULL)
	{
		strcpy(path, (options.SaveBinaryAsText)? "a.asm" : "a.out");
	}
	else{
		strcpy(path, options.OutputPath);
	}
	if (options.SaveBinaryAsText){
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
				<< (options.DebugSymbols? GetDebugSymbol(*iter, options.DebugLoopShifting) : "") << endl;
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

compiler_options_t options;


int main(int argc, char ** argv) {
	if (argc < 2){
		cerr << "Usage:\r\n";
		cerr << argv[0] << " -i source file [-o output file] [-e -s -d]\r\n";
		cerr << "\t-e - enable extended instruction set[UNSUPPORTED]\r\n";
		cerr << "\t-s - save output binary in text format\r\n";
		cerr << "\t-d - enable debug symbols for text format\r\n";
		cerr << "\t-On - enable optimization\r\n";
		cerr << "\t-l loop shifting on debug\r\n";
		cerr << "Set source file to compile\r\n";
		return -1;
	}
	int c = 0;
	options.InputPath = NULL;
	options.OutputPath = NULL;
	options.SetCompilerMode = false;
	options.SaveBinaryAsText = false;
	options.DebugSymbols = false;
	options.OptimizationLevel = 0;
	options.DebugLoopShifting = false;


	while((c = getopt(argc, argv, "dsei:lo:")) != -1){
		switch(c){
		case 'i':
			options.InputPath = optarg;
			break;
		case 'o':
			options.OutputPath = optarg;
			break;
		case 'O':
			options.OptimizationLevel = atol(optarg);
			break;
		case 'e':
			options.SetCompilerMode = true;
			break;
		case 's':
			options.SaveBinaryAsText = true;
			break;
		case 'd':
			options.DebugSymbols = true;
			break;
		case 'l':
			options.DebugLoopShifting = true;
			break;
		case '?':
			return -1;
			break;
		}
	}
	if (options.InputPath){
		std::ifstream SourceFile (options.InputPath, std::ifstream::binary);

		if (!SourceFile.good()){
			cerr << "Source File"<< options.InputPath <<" open error, exiting\r\n";
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
			Bf *BfCompiler = new Bf(options);
			if (BfCompiler->Compile(SourceBuffer, length, Output)){
				cerr << "Compilation error\r\n";
				return -2;
			}
			
			SaveOutput(Output,options); 


			delete[] SourceBuffer;
		}


	}
	else{
		std::cerr << "No input file, exiting!" << std::endl;		
	}
    return 0;
}
