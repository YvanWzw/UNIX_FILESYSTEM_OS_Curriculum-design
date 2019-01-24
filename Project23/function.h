#pragma once


#include "globalVariable.h"
#include "struct.h"


int turnOn(char *path, bool initSystem);

int shutDown();

int loadSuperBlock();

int loadINodeBlock();

int updateSuperBlock();

int updateAllInode();

int updateINodeBlock(INodeList *iNodeList);

int writeIntoBlock(const void *_Buffer, int blockId, int offsetInBlcok, int size, int count);

int readFromBlock(void *_Buffer, int blockId, int offsetInBlcok, int size, int count);

int getINodeList();

int linkINode(int iNodeId);

int allocateFreeBlock(int& nextBlockId);

int freeTheBlock(int blockId);

int freeTheINode(int iNodeId);

int	getFreeINodeId();