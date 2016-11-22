#ifndef IMAGE_H_
#define IMAGE_H_

#include <cstdlib>
#include <vector>
#include <fstream>
#include <iostream>
#include "bfutils.h"
#include "cmd.h"


class Section{

	public:
		Section(std::vector<Cmd> &SectionCmd, uint16_t MemoryBase, uint16_t MemorySize);
		Section(std::vector<uint16_t> &SectionData, 
				uint16_t MemoryBase, uint16_t MemorySize);

		Section(std::fstream &File);
		~Section();
	
		uint8_t GetType(){return Hdr.type;};	
		WordToBigEndian_t GetFileBase(){return Hdr.FileBase;};
		WordToBigEndian_t GetMemoryBase(){return Hdr.MemoryBase;};
		WordToBigEndian_t GetFileSize(){return Hdr.FileSize;};
		WordToBigEndian_t GetMemorySize(){return Hdr.MemorySize;};

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
		Image(std::fstream &File);
		Image(uint8_t _machine);

		~Image();

		void AddSection(Section &section);
		uint8_t GetSectionNum(void){return Hdr.SectionNum;};
		Section &GetSection(uint8_t section){return Sections[section];};

		void SetIpEntry(ADDRESS_TYPE Ptr){Hdr.IpEntry.Word = Ptr;};
		void SetApEntry(ADDRESS_TYPE Ptr){Hdr.ApEntry.Word = Ptr;};

		ADDRESS_TYPE GetIpEntry(){return Hdr.IpEntry.Word;};
			
		ADDRESS_TYPE GetApEntry(){return Hdr.ApEntry.Word;};
		
		void Write(std::fstream &File);


		bool Error(){return err;};
	private:
		
		bool err;
		BfHeader_t Hdr;
		
		std::vector<Section> Sections;

};

#endif //IMAGE_H_
