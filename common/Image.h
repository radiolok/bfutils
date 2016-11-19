#ifndef IMAGE_H_
#define IMAGE_H_

#include <cstdlib>
#include <vector>
#include <fstream>
#include <iostream>
#include "bfutils.h"

class Section{

	public:
		Section(std::vector<uint16_t> &SectionData, 
				WordToBigEndian_t &MemoryBase, WordToBigEndian_t &MemorySize);

		Section(std::fstream &File, uint16_t *MemoryPtr);
		~Section();
		
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
		Image(std::fstream &File, uint16_t *MemoryPtr);
		~Image();

		bool Error(){return err;};
	private:
		
		bool err;
		BfHeader_t Hdr;
		
		std::vector<Section> Sections;

};

#endif //IMAGE_H_
