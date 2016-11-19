#include "Image.h"


using namespace std;

Image::Image(ifstream &File, uint16_t *MemoryPtr){

	err = false;
	//Read BF header:
	File.read(reinterpret_cast<char *>(&Hdr), sizeof(BfHeader_t));
	swapLEtoBE(&Hdr.Magic);
	swapLEtoBE(&Hdr.IPEntry);	
	swapLEtoBE(&Hdr.APEntry);	

	if (Hdr.Magic != BF_MAGIC)
	{
		err = true;
		cerr << "Wrong BF image!\r\n";
		return;
	}
	//Read and load sections:
	
	for (uint8_t section = 0; section < Hdr.SectionNum; ++section){
	
		Sections.push_back(Image(File, MemoryPtr));
		if (Sections.back().Error()){
			cerr << "Section " << section << "Read error!\r\n";
		}	
	}
	
}

Image::~Image(){
}

Section::Section(vector<uint16_t> &SectionData, WordToBigEndian_t &MemoryBase, WordToBigEndian_t &MemorySize){

	Hdr.MemoryBase = MemoryBase;
	Hdr.MemorySize = MemorySize;

	Hdr.FileSize = SectionData.size() * sizeof(uint16_t);


	if (Hdr.FileSize != 0){
		Data = new uint16_t[Hdr.FileSize];
		size_t pos = 0;
		for (auto iter = SectionData.begin(); iter < SectionData.end(); ++iter){
			Data[pos] = *iter;	
		}
	}
}

Setion::Section(ifstream &File, uint16_t *MemoryPtr){

	err = false;	
	//Read Section Image	
	File.read(reinterpret_cast<char *>(&Hdr), sizeof(BfSection_t));
	swapLEtoBE(&Hdr, sizeof(BfSection_t));

	//Remember position after Section header reading
	streampos Image_pos = File.tellg();

	Data = MemoryPtr + Hdr.MemoryBase;
	//if current section size is not NULL, read data:	
	if (GetFileSize() != 0){
	
		File.seekg(GetFileBase());
		File.read(reinterpret_cast<char *>(Data), GetFileSize());
		swapLEtoBE((WordToBigEndian_t*)Data, GetFileSize());
	}
	//Restore File position
	File.seekg(Image_pos);
}

Section::~Section(){

}

Section::WriteHeader(ofstream &File){
	//Remember Section Header Position:
	Hdr.FileBase = File.tellg();

	//Convert LE and BE:
	swapLEtoBE((WordToBigEndian_t*)Hdr, sizeof(BfSection_t));

	File.write(reinterpret_cast<char *>(Hdr), sizeof(BfSection_t));

}

Section::WriteData(ostream &File){
	
	//Remember Current place:
	streampos FileBase = File.tellg();

	//Go to Header position:
	File.seekg(Hdr.FileBase);	
	//Read-modify-write Header;
	File.read(reinterpret_cast<char *>(&Hdr), sizeof(BfSection_t));
	swapLEtoBE(&Hdr, sizeof(BfSection_t));

	Hdr.FileBase = FileBase;

	swapLEtoBE((WordToBigEndian_t*)Hdr, sizeof(BfSection_t));
	File.write(reinterpret_cast<char *>(Hdr), sizeof(BfSection_t));

	//return to data:
	File.seekg(FileBase);
	if (Hdr.FileSize != 0){
		swapLEtoBE((WordToBigEndian_t*)Data, sizeof(Hdr.FileSize));
		File.write(reinterpret_cast<char *>(Data), sizeof(Hdr.FileSize));
	}
}
