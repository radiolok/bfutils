#include "Image.h"


using namespace std;

Section::Section(vector<Cmd> &SectionCmd, uint16_t MemoryBase, uint16_t MemorySize){

	Hdr.MemoryBase.Word = MemoryBase;
	Hdr.MemorySize.Word = MemorySize;

	Hdr.FileSize.Word = SectionCmd.size() * sizeof(uint16_t);


	if (Hdr.FileSize.Word != 0){
		Data = new uint16_t[Hdr.FileSize.Word];
		size_t pos = 0;
		for (auto iter = SectionCmd.begin(); iter < SectionCmd.end(); ++iter){
			Data[pos] = iter->GetCmd();	
		}
	}
}
Section::Section(vector<uint16_t> &SectionData, uint16_t MemoryBase, uint16_t MemorySize){

	Hdr.MemoryBase.Word = MemoryBase;
	Hdr.MemorySize.Word = MemorySize;

	Hdr.FileSize.Word = SectionData.size() * sizeof(uint16_t);


	if (Hdr.FileSize.Word != 0){
		Data = new uint16_t[Hdr.FileSize.Word];
		size_t pos = 0;
		for (auto iter = SectionData.begin(); iter < SectionData.end(); ++iter){
			Data[pos] = *iter;	
		}
	}
}

Section::Section(std::fstream &File, uint16_t *MemoryPtr){

	err = false;	
	//Read Section Image	
	File.read(reinterpret_cast<char *>(&Hdr), sizeof(BfSection_t));
	swapLEtoBE((WordToBigEndian_t *)&Hdr, sizeof(BfSection_t));

	//Remember position after Section header reading
	streampos Image_pos = File.tellg();

	Data = MemoryPtr + Hdr.MemoryBase.Word;
	//if current section size is not NULL, read data:	
	if (GetFileSize().Word != 0){
	
		File.seekg(GetFileBase().Word);
		File.read(reinterpret_cast<char *>(Data), GetFileSize().Word);
		swapLEtoBE((WordToBigEndian_t*)Data, GetFileSize().Word);
	}
	//Restore File position
	File.seekg(Image_pos);
}

Section::~Section(){

}

void Section::WriteHeader(std::fstream &File){
	//Remember Section Header Position:
	Hdr.FileBase.Word = File.tellg();

	//Convert LE and BE:
	swapLEtoBE((WordToBigEndian_t*)&Hdr, sizeof(BfSection_t));

	File.write(reinterpret_cast<char *>(&Hdr), sizeof(BfSection_t));

}

void Section::WriteData(std::fstream &File){
	
	//Remember Current place:
	streampos FileBase = File.tellp();

	//Go to Header position:
	File.seekg(Hdr.FileBase.Word);	
	File.seekp(Hdr.FileBase.Word);	
	//Read-modify-write Header;
	File.read(reinterpret_cast<char *>(&Hdr), sizeof(BfSection_t));
	swapLEtoBE((WordToBigEndian_t *)&Hdr, sizeof(BfSection_t));

	Hdr.FileBase.Word = FileBase;

	swapLEtoBE((WordToBigEndian_t*)&Hdr, sizeof(BfSection_t));
	File.write(reinterpret_cast<char *>(&Hdr), sizeof(BfSection_t));

	//return to data:
	File.seekp(FileBase);
	if (Hdr.FileSize.Word != 0){
		swapLEtoBE((WordToBigEndian_t*)Data, sizeof(Hdr.FileSize.Word));
		File.write(reinterpret_cast<char *>(Data), Hdr.FileSize.Word);
	}
}

//This constructor will read image and load sections into their places
Image::Image(std::fstream &File, uint16_t *MemoryPtr){

	err = false;
	//Read BF header:
	File.read(reinterpret_cast<char *>(&Hdr), sizeof(BfHeader_t));
	swapLEtoBE(&Hdr.Magic);
	swapLEtoBE(&Hdr.IpEntry);	
	swapLEtoBE(&Hdr.ApEntry);	

	if (Hdr.Magic.Word != BF_MAGIC)
	{
		err = true;
		std::cerr << "Wrong BF image!\r\n";
		return;
	}
	//Read and load sections:
	
	for (uint8_t section = 0; section < Hdr.SectionNum; ++section){
	
		Sections.push_back(Section(File, MemoryPtr));
		if (Sections.back().Error()){
			std::cerr << "Section " << section << "Read error!\r\n";
		}	
	}
	
}

//This constructor is used by compiler:
Image::Image(uint8_t _Machine){
	Hdr.Magic.Word = BF_MAGIC;
	Hdr.Machine = _Machine;
	Hdr.HeaderSize = sizeof(BfHeader_t);
	Hdr.SectionNum = 0;
	Hdr.flags = 0;
	Hdr.IpEntry.Word = 0;
	Hdr.ApEntry.Word = 0;
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
	File.write(reinterpret_cast<char *>(&Hdr), sizeof(BfHeader_t));

	for (auto section = Sections.begin(); section < Sections.end(); ++section){
		section->WriteHeader(File);
	}
	for (auto section = Sections.begin(); section < Sections.end(); ++section){
		section->WriteData(File);
	}
}

