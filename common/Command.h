#ifndef COMMAND_H_
#define COMMAND_H_

#include "bfutils.h"

#include "Image.h"



typedef struct _Command_t{
	unsigned int cmd;
	int bias;
} Command_t;

void LoadData(Image &image, unsigned int *data);
void LoadCode(Image &image, Command_t *cmd);

void LoadShared(Image &image, uint16_t *data);
#endif
