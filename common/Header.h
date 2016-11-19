#ifndef HEADER_H_
#define HEADER_H_

#include <vector>
#include <fstream>

#include "bfutils.h"

class Image{

	public:
		Image();
		~Image();

		bool Error(){return err;};
	private:
		
		bool err;
		BfHeader_t Hdr;
		
		vector<Section> Sections;

}

class Section{

	public:
		Section();
		~Section();
		
		WordToBigEndian_t GetFileBase(){return Hdr.FileBase;};
		WordToBigEndian_t GetMemoryBase(){return Hdr.MemoryBase;};
		WordToBigEndian_t GetFileSize(){return Hdr.FileSize;};
		WordToBigEndian_t GetMemorySize(){return Hdr.MemorySize;};

		uint16_t *GetData(){return Data;};

		bool Error(){return err;};
	private:

		bool err;
		BfSection_t Hdr;

		uint16_t *Data;

}

#endif HEADER_H_
