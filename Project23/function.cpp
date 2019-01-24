
#include "function.h"
using namespace std;


INodeList iNodeA[SUM_OF_INODE];
INodeList *root = nullptr;
INodeList *current = nullptr;
SuperBlock *superBlock = nullptr;
Directory *currentDir = nullptr;
FILE *temporalFile;


int turnOn(char *path, bool initSystem)
{
	int statusCode = 0;
	fopen_s(&temporalFile, path, "rb+");
	if (initSystem == true)
		init(path);
	superBlock = new SuperBlock;
	currentDir = new Directory;
	statusCode = loadSuperBlock();
	if (statusCode != SUCCESS)
		return statusCode;
	statusCode = loadINodeBlock();
	if (statusCode != SUCCESS)
		return statusCode;
	root = &iNodeA[1];
	statusCode = readFromBlock(currentDir, NUMBER_OF_THE_START_OF_THE_DATABLOCK + SUM_OF_GROUPLINK - 2, 0, sizeof(Directory), 1);
	if (statusCode != SUCCESS)
		return statusCode;
	root->iNodePointer = iNodeA[0].iNodeId;
	current = root;
	statusCode = getFreeINodeId();
	if (statusCode != SUCCESS)
		return statusCode;
	while ((statusCode = login()) != SUCCESS)
	{
		printErrorInfo(statusCode);
		system("pause");
		system("cls");
	}
	info();
	return statusCode;
}


int shutDown()
{
	int statusCode = 0;
	statusCode = updateAllInode();
	if (statusCode != SUCCESS)
		return statusCode;
	statusCode = updateSuperBlock();
	if (statusCode != SUCCESS)
		return statusCode;
	delete superBlock;
	superBlock = nullptr;
	delete currentDir;
	currentDir = nullptr;
	root = nullptr;
	current = nullptr;
	statusCode = fclose(temporalFile);
	return statusCode;
}


int loadSuperBlock()
{
	fseek(temporalFile, SIZE_OF_ONE_BLOCK, SEEK_SET);
	size_t temp_b = fread(superBlock, sizeof(SuperBlock), 1, temporalFile);
	if (temp_b != 1)
		return ERROR_SUPERBLOCK_READ_FAIL;
	return SUCCESS;
}


int loadINodeBlock()
{
	int statusCode = 0;
	statusCode = getINodeList();
	return statusCode;
}


int updateSuperBlock()
{
	if (temporalFile == NULL)
		return ERROR_FILE_LOAD_FAIL;
	fseek(temporalFile, SIZE_OF_ONE_BLOCK, SEEK_SET);
	size_t temp_a = fwrite(superBlock, sizeof(SuperBlock), 1, temporalFile);
	fflush(temporalFile);
	if (temp_a != 1)
		return ERROR_SUPERBLOCK_UPDATE_FAIL;
	return SUCCESS;
}


int updateAllInode()
{
	size_t temp_a = 0;
	int offset = 0;
	int iNodeId = 0;
	for (int i = NUMBER_OF_THE_START_OF_THE_INODEBLOCK; i < NUMBER_OF_THE_START_OF_THE_DATABLOCK; i++)
		for (int j = 0; j < 5; j++)
		{
			offset = j * SIZE_OF_ONE_INODE;
			temp_a = writeIntoBlock(&iNodeA[iNodeId].iNode, i, offset, SIZE_OF_ONE_INODE, 1);
			if (temp_a != SUCCESS)
				return temp_a;
			iNodeId++;
		}
	return SUCCESS;
}


int updateINodeBlock(INodeList *iNodeList)
{
	unsigned long int offset = (((iNodeList->iNodeId + 1) / SUM_OF_INODE_IN_ONE_BLOCK) + 2)*SIZE_OF_ONE_BLOCK + ((iNodeList->iNodeId  % SUM_OF_INODE_IN_ONE_BLOCK))*SIZE_OF_ONE_INODE;
	fseek(temporalFile, offset, SEEK_SET);
	size_t temp_a = fwrite(&iNodeList->iNode, SIZE_OF_ONE_INODE, 1, temporalFile);
	fflush(temporalFile);
	if (temp_a != 1)
		return ERROR_INODEBLOCK_UPDATE_FAIL;
	return SUCCESS;
}


int writeIntoBlock(const void *_Buffer, int blockId, int offsetInBlcok, int size, int count)
{
	if (temporalFile == NULL)
		return ERROR_FILE_LOAD_FAIL;
	unsigned long int offset = blockId * SIZE_OF_ONE_BLOCK + offsetInBlcok;
	fseek(temporalFile, offset, SEEK_SET);
	size_t temp_a = fwrite(_Buffer, size, count, temporalFile);
	fflush(temporalFile);
	if (temp_a != count)
		return ERROR_BLOCK_WRITE_FAIL;
	return SUCCESS;
}


int readFromBlock(void *_Buffer, int blockId, int offsetInBlcok, int size, int count)
{
	if (temporalFile == NULL)
		return ERROR_FILE_LOAD_FAIL;
	unsigned long int offset = blockId * SIZE_OF_ONE_BLOCK + offsetInBlcok;
	fseek(temporalFile, offset, SEEK_SET);
	size_t temp_a = fread(_Buffer, size, count, temporalFile);
	if (temp_a != count)
		return ERROR_BLOCK_READ_FAIL;
	return SUCCESS;
}


int getINodeList()
{
	size_t temp_a;
	int offset = 0;
	int iNodeId = 0;
	for (int i = NUMBER_OF_THE_START_OF_THE_INODEBLOCK; i < NUMBER_OF_THE_START_OF_THE_DATABLOCK; i++)
		for (int j = 0; j < 5; j++)
		{
			offset = j * SIZE_OF_ONE_INODE;
			temp_a = readFromBlock(&iNodeA[iNodeId].iNode, i, offset, SIZE_OF_ONE_INODE, 1);
			if (temp_a != SUCCESS)
				return temp_a;
			iNodeA[iNodeId].iNodeId = iNodeId;
			iNodeId++;
		}
	return SUCCESS;
}


int getFreeINodeId()
{
	for (int i = 0; i < 60; i++)
	{
		if (iNodeA[i].iNode.linkOfFile == 0)
		{
			superBlock->pointerOfFreeINode = i;
			break;
		}
	}
	return SUCCESS;
}


int linkINode(int iNodeId)
{
	if (iNodeA[iNodeId].iNode.linkOfFile == 0)
	{
		iNodeA[iNodeId].iNodeId = iNodeId;
		iNodeA[iNodeId].iNode.linkOfFile++;
		iNodeA[iNodeId].iNode.sizeOfFile = 0;
		iNodeA[iNodeId].iNode.createTime = getTimeStamp();
		iNodeA[iNodeId].iNode.modifyTime = getTimeStamp();
		strcpy_s(iNodeA[iNodeId].iNode.ownerID, currentUser.userName);
		strcpy_s(iNodeA[iNodeId].iNode.groupID, currentUser.userGroup);
		superBlock->numberOfFreeINode--;
		int statusCode = getFreeINodeId();
		return statusCode;
	}
	iNodeA[iNodeId].processCount++;
	return SUCCESS;
}


int allocateFreeBlock(int &nextBlockId)
{
	if (superBlock->numberOfFreeBlock == 0)
		return ERROR_BLOCK_WORKOUT;
	if (superBlock->pointerOfFreeBlock == 1)
	{
		nextBlockId = superBlock->stackOfFreeBlock[0];
		int statusCode = readFromBlock(superBlock->stackOfFreeBlock, nextBlockId, 0, 4, 60);
		superBlock->pointerOfFreeBlock = SUM_OF_GROUPLINK - 1;
		return statusCode;
	}
	superBlock->pointerOfFreeBlock--;
	superBlock->numberOfFreeBlock--;
	nextBlockId = superBlock->stackOfFreeBlock[superBlock->pointerOfFreeBlock];
	return SUCCESS;
}


int freeTheBlock(int blockId)
{
	if (superBlock->pointerOfFreeBlock == SUM_OF_GROUPLINK - 1)
	{
		int statusCode = writeIntoBlock(&superBlock->stackOfFreeBlock, blockId, 0, sizeof(unsigned int), 60);
		if (statusCode != SUCCESS)
			return statusCode;
		superBlock->pointerOfFreeBlock = 0;
		superBlock->stackOfFreeBlock[0] = blockId;
	}
	else
	{
		superBlock->stackOfFreeBlock[superBlock->pointerOfFreeBlock + 1] = blockId;
		superBlock->pointerOfFreeBlock++;
	}
	superBlock->numberOfFreeBlock++;
	return SUCCESS;
}


int freeTheINode(int iNodeId)
{
	if (iNodeA[iNodeId].iNode.linkOfFile == 0)
		return ERROR_INODE_NONEXISTENT;
	iNodeA[iNodeId].iNode.linkOfFile = 0;
	superBlock->numberOfFreeINode++;
	return SUCCESS;
}