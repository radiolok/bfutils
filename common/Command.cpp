



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
