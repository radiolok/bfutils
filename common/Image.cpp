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

#include "Image.h"


using namespace std;



//add code section
Section::Section(vector<Cmd> &SectionCmd, uint16_t MemoryBase, uint16_t MemorySize){

	Hdr.MemoryBase = MemoryBase;
	Hdr.MemorySize = MemorySize;

	Hdr.FileSize = SectionCmd.size() * sizeof(uint16_t);

	Hdr.type = SECTION_CODE;

	if (Hdr.FileSize != 0){
		Data = new uint16_t[Hdr.FileSize];
		size_t pos = 0;
		for (auto iter = SectionCmd.begin(); iter < SectionCmd.end(); ++iter, ++pos){
			Data[pos] = iter->GetCmd();	
		}
	}
}


//Add data section
Section::Section(vector<uint16_t> &SectionData, uint16_t MemoryBase, uint16_t MemorySize){

	Hdr.MemoryBase = MemoryBase;
	Hdr.MemorySize = MemorySize;

	Hdr.FileSize = SectionData.size() * sizeof(uint16_t);

	Hdr.type = SECTION_DATA;

	if (Hdr.FileSize != 0){
		Data = new uint16_t[Hdr.FileSize];
		size_t pos = 0;
		for (auto iter = SectionData.begin(); iter < SectionData.end(); ++iter){
			Data[pos] = *iter;	
		}
	}
}


//Load Section from file
Section::Section(std::fstream &File){

	err = false;	
	//Read Section Image	
	File.read(reinterpret_cast<char *>(&Hdr), sizeof(BfSection_t));
	swapLEtoBE(&Hdr, sizeof(BfSection_t));

	//Remember position after Section header reading
	streampos Image_pos = File.tellg();

	Data = new uint16_t [GetMemorySize()];
	//if current section size is not NULL, read data:	
	if (GetFileSize() != 0){
	
		File.seekg(GetFileBase());
		File.read(reinterpret_cast<char *>(Data), GetFileSize());
		swapLEtoBE(Data, GetFileSize());
	}
	else
	{
		memset(Data, 0, GetMemorySize()*sizeof(uint16_t));
	}
	//Restore File position
	File.seekg(Image_pos);
}

Section::~Section(){

}

void Section::WriteHeader(std::fstream &File){
	//Remember Section Header Position:
	Hdr.FileBase = File.tellp();

	//Convert LE and BE:
	swapLEtoBE(&Hdr, sizeof(BfSection_t));
	File.write(reinterpret_cast<char *>(&Hdr), sizeof(BfSection_t));
	swapLEtoBE(&Hdr, sizeof(BfSection_t));

}

void Section::WriteData(std::fstream &File){
	
	//Remember Current place:
	streampos FileBase = File.tellp();
	//Go to Header position:
	File.seekg(Hdr.FileBase);	
	File.seekp(Hdr.FileBase);	
	Hdr.FileBase = FileBase;

	swapLEtoBE(&Hdr, sizeof(BfSection_t));
	File.write(reinterpret_cast<char *>(&Hdr), sizeof(BfSection_t));
	swapLEtoBE(&Hdr, sizeof(BfSection_t));

	//return to data:
	File.seekp(FileBase);
	File.seekg(FileBase);


	if (Hdr.FileSize != 0){
		swapLEtoBE(Data, Hdr.FileSize);
		File.write(reinterpret_cast<char *>(Data), Hdr.FileSize);
		swapLEtoBE(Data, Hdr.FileSize);
	}
}

//This constructor will read image and load sections into their places
Image::Image(std::fstream &File, bool hex){

	err = false;
	//Read BF header:
	File.read(reinterpret_cast<char *>(&Hdr), sizeof(BfHeader_t));
	swapLEtoBE(&Hdr.Magic);
	swapLEtoBE(&Hdr.IpEntry);	
	swapLEtoBE(&Hdr.ApEntry);	

	if (Hdr.Magic != BF_MAGIC)
	{
		err = true;
		std::cerr << "Wrong BF image!\r\n";
		return;
	}
	//Read and load sections:
	
	for (uint8_t section = 0; section < Hdr.SectionNum; ++section){
	
		Sections.push_back(Section(File));
		if (Sections.back().Error()){
			std::cerr << "Section " << section << "Read error!\r\n";
		}	
	}
	
}

//This constructor is used by compiler:
Image::Image(uint8_t _Machine){
	Hdr.Magic = BF_MAGIC;
	Hdr.Machine = _Machine;
	Hdr.HeaderSize = sizeof(BfHeader_t);
	Hdr.SectionNum = 0;
	Hdr.flags = 0;
	Hdr.IpEntry = 0;
	Hdr.ApEntry = 0;
}

void Image::AddSection(Section &section){

	Sections.push_back(section);
	Hdr.SectionNum = Sections.size();
}

Image::~Image(){
}


void Image::Write(std::fstream &File){


	swapLEtoBE(&Hdr.Magic);
	swapLEtoBE(&Hdr.IpEntry);
	swapLEtoBE(&Hdr.ApEntry);
	Hdr.HeaderSize += sizeof(BfHeader_t) * Hdr.SectionNum;
	File.write(reinterpret_cast<char *>(&Hdr), sizeof(BfHeader_t));

	swapLEtoBE(&Hdr.Magic);
	swapLEtoBE(&Hdr.IpEntry);
	swapLEtoBE(&Hdr.ApEntry);

	for (auto section = Sections.begin(); section < Sections.end(); ++section){
		section->WriteHeader(File);
	}
	for (auto section = Sections.begin(); section < Sections.end(); ++section){
		section->WriteData(File);
	}
}

