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

#ifndef IMAGE_H_
#define IMAGE_H_

#include <cstdlib>
#include <vector>
#include <fstream>
#include <iostream>
#include "bfutils.h"
#include "cmd.h"
#include <string.h>


class Section{

	public:
		Section(std::vector<Cmd> &SectionCmd, uint16_t MemoryBase, uint16_t MemorySize);
		Section(std::vector<uint16_t> &SectionData, 
				uint16_t MemoryBase, uint16_t MemorySize);

		Section(std::fstream &File);
		~Section();
	
		uint8_t GetType(){return Hdr.type;};	
		uint16_t GetFileBase(){return Hdr.FileBase;};
		uint16_t GetMemoryBase(){return Hdr.MemoryBase;};
		uint16_t GetFileSize(){return Hdr.FileSize;};
		uint16_t GetMemorySize(){return Hdr.MemorySize;};

		

		void WriteHeader(std::fstream &File);
		void WriteData(std::fstream &File);

		uint16_t *GetData(){return Data;};

		bool Error(){return err;};
	private:

		bool err;
		BfSection_t Hdr;

		uint16_t *Data;

};

class Image{

	public:
		Image(std::fstream &File, bool hex);
		Image(uint8_t _machine);

		~Image();

		void AddSection(Section &section);
		uint8_t GetSectionNum(void){return Hdr.SectionNum;};
		Section &GetSection(uint8_t section){return Sections[section];};

		void SetIpEntry(ADDRESS_TYPE Ptr){Hdr.IpEntry = Ptr;};
		void SetApEntry(ADDRESS_TYPE Ptr){Hdr.ApEntry = Ptr;};

		ADDRESS_TYPE GetIpEntry(){return Hdr.IpEntry;};
			
		ADDRESS_TYPE GetApEntry(){return Hdr.ApEntry;};

		bool ReadHex(std::fstream &File);
		
		void Write(std::fstream &File);


		bool Error(){return err;};
	private:
		
		bool m_hex;
		bool err;
		BfHeader_t Hdr;
		
		std::vector<Section> Sections;

};

#endif //IMAGE_H_
