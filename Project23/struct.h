#pragma once


//CREATETIME:2019-1-3 19:14:22


#include "globalVariable.h"


class SuperBlock
{
public:
	unsigned int stackOfFreeBlock[SUM_OF_GROUPLINK];
	unsigned int numberOfFreeBlock;
	unsigned int pointerOfFreeBlock;
	unsigned int stackOfFreeINode[SUM_OF_INODE];
	unsigned int numberOfFreeINode;
	unsigned int pointerOfFreeINode;
};


class INode
{
public:
	INode() :linkOfFile(0), sizeOfFile(0) {}
	unsigned long int sizeOfFile;
	unsigned int linkOfFile;
	unsigned int address[LENGTH_OF_INODE_ADDR];
	char ownerID[LENGTH_OF_OWNER_ID];
	char groupID[LENGTH_OF_GROUP_ID];
	unsigned short int modeOfFile;
	unsigned long int createTime;
	unsigned long int modifyTime;
};


class INodeList
{
public:
	INode iNode;
	int iNodePointer;
	unsigned short int iNodeId;
	unsigned int processCount;
};


class DirectoryNode
{
public:
	DirectoryNode() : iNodeId(0) {}
	char directoryName[LENGTH_OF_FILENAME];
	unsigned short int iNodeId;
};


class Directory
{
public:
	Directory() :numberOfDirectoryNode(0) {}
	unsigned short int numberOfDirectoryNode;
	DirectoryNode directoryNode[SUM_OF_DIRECTORY_IN_ONE_DIR];
};


class User
{
public:
	User() {}
	char userName[LENGTH_OF_OWNER_ID];
	char userPassword[LENGTH_OF_OWNER_PASSWORD];
	char userGroup[LENGTH_OF_GROUP_ID];
};