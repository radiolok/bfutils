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



#include "Command.h"


//We should init 64K data object
void LoadShared(Image &image, uint16_t *data){

	//Let;s find all code segments:
	for (uint8_t section = 0; section < image.GetSectionNum(); ++section){
		Section CurrentSection = image.GetSection(section);
		 for (uint16_t mem = CurrentSection.GetMemoryBase().Word;
				 mem < CurrentSection.GetMemoryBase().Word + 
				 CurrentSection.GetMemorySize().Word; ++mem)
		 	{
				data[mem] = CurrentSection.GetData()[mem];
			}
		
		
	}

}


//We should init 64K cmd object
//
void LoadCode(Image &image, Command_t *cmd){

	//Let;s find all code segments:
	for (uint8_t section = 0; section < image.GetSectionNum(); ++section){
		Section CurrentSection = image.GetSection(section);
		if (CurrentSection.GetType() == SECTION_CODE){
		 for (uint16_t mem = CurrentSection.GetMemoryBase().Word;
				 mem < CurrentSection.GetMemoryBase().Word + 
				 CurrentSection.GetMemorySize().Word; ++mem)
		 	{
				uint16_t command = CurrentSection.GetData()[mem];
				cmd[mem].cmd = (command >> 13) & 0x07;
				cmd[mem].bias = (command & 0x1000)? -(command & 0x0FFF) : (command & 0xFFF);
		 
			}
		
		}
		
	}

}

//We should init 64K data object
void LoadData(Image &image, unsigned int *data){

	//Let;s find all code segments:
	for (uint8_t section = 0; section < image.GetSectionNum(); ++section){
		Section CurrentSection = image.GetSection(section);
		if (CurrentSection.GetType() == SECTION_DATA){
		 for (uint16_t mem = CurrentSection.GetMemoryBase().Word;
				 mem < CurrentSection.GetMemoryBase().Word + 
				 CurrentSection.GetMemorySize().Word; ++mem)
		 	{
				data[mem] = CurrentSection.GetData()[mem];
			}
		
		}
		
	}

}
