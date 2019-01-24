
#include "miscellaneous.h"
#include <sys/timeb.h>
#include <fstream>
#include <Windows.h>
#include <conio.h>
using namespace std;


char* transformConstCharSToCharS(const char* value)
{
	char* temp = new char[100];
	strcpy_s(temp, 100, value);
	return temp;
}


unsigned long int getTimeStamp()
{
	timeb time;
	ftime(&time);
	return (unsigned long int)time.time;
}


void hiddenPassword(char password[LENGTH_OF_OWNER_PASSWORD])
{
	char input;
	memset(password, 0, LENGTH_OF_OWNER_PASSWORD);
	for (int i = 0; i <= LENGTH_OF_OWNER_PASSWORD; i++)
	{
		input = _getch();
		if (input == 13)
			break;
		if (i != LENGTH_OF_OWNER_PASSWORD)
			password[i] = input;
		cout << "*";
	}
	cout << endl;
}


void fileEmpty(const string fileName)
{
	fclose(temporalFile);
	fstream file(fileName, ios::out);
	file.close();
}


void init(const char* filename)
{
	//format
	fileEmpty(filename);
	fopen_s(&temporalFile, filename, "rb+");
	superBlock = new SuperBlock;

	//superBlock
	superBlock->numberOfFreeBlock = SUM_OF_DATABLOCK - 4;
	superBlock->numberOfFreeINode = SUM_OF_INODE-4;
	superBlock->pointerOfFreeBlock = SUM_OF_GROUPLINK - 5;
	superBlock->pointerOfFreeINode = 4;
	for (int i = 0; i < SUM_OF_GROUPLINK; i++)
		superBlock->stackOfFreeBlock[i] = NUMBER_OF_THE_START_OF_THE_DATABLOCK + i;
	for (int i = 0; i < SUM_OF_INODE; i++)
		superBlock->stackOfFreeINode[i] = i;
	updateSuperBlock();

	//inodeBlock-super-root
	INodeList *iNode11 = new INodeList;
	iNode11->iNode.address[0] = NUMBER_OF_THE_START_OF_THE_DATABLOCK + SUM_OF_GROUPLINK - 2;
	iNode11->iNode.createTime = getTimeStamp();
	strcpy_s(iNode11->iNode.groupID, "ROOT");
	iNode11->iNode.linkOfFile = 1;
	iNode11->iNode.modeOfFile = 1774;
	iNode11->iNode.modifyTime = getTimeStamp();
	strcpy_s(iNode11->iNode.ownerID, "root");
	iNode11->iNode.sizeOfFile = sizeof(DirectoryNode);
	iNode11->iNodeId = 0;
	iNode11->iNodePointer = NULL;
	iNode11->processCount = 0;
	updateINodeBlock(iNode11);
	//inodeBlock-root
	INodeList *iNode = new INodeList;
	iNode->iNode.address[0] = NUMBER_OF_THE_START_OF_THE_DATABLOCK + SUM_OF_GROUPLINK - 2;
	iNode->iNode.address[1] = NUMBER_OF_THE_START_OF_THE_DATABLOCK + SUM_OF_GROUPLINK - 1;
	iNode->iNode.address[2] = NUMBER_OF_THE_START_OF_THE_DATABLOCK + SUM_OF_GROUPLINK - 3;
	iNode->iNode.createTime = getTimeStamp();
	strcpy_s(iNode->iNode.groupID, "ROOT");
	iNode->iNode.linkOfFile = 2;
	iNode->iNode.modeOfFile = 1774;
	iNode->iNode.modifyTime = getTimeStamp();
	strcpy_s(iNode->iNode.ownerID, "root");
	iNode->iNode.sizeOfFile = sizeof(DirectoryNode);
	iNode->iNodeId = 1;
	iNode->iNodePointer = iNode11->iNodeId;
	iNode->processCount = 0;
	updateINodeBlock(iNode);
	//inodeBlock-etc
	INodeList *node = new INodeList;
	node->iNode.address[0] = NUMBER_OF_THE_START_OF_THE_DATABLOCK + SUM_OF_GROUPLINK - 3;
	node->iNode.address[1] = NUMBER_OF_THE_START_OF_THE_DATABLOCK + SUM_OF_GROUPLINK - 2;
	node->iNode.address[2] = NUMBER_OF_THE_START_OF_THE_DATABLOCK + SUM_OF_GROUPLINK - 4;
	node->iNode.createTime = getTimeStamp();
	strcpy_s(node->iNode.groupID, "ROOT");
	node->iNode.linkOfFile = 2;
	node->iNode.modeOfFile = 1774;
	node->iNode.modifyTime = getTimeStamp();
	strcpy_s(node->iNode.ownerID, "root");
	node->iNode.sizeOfFile = sizeof(DirectoryNode);
	node->iNodeId = 2;
	node->iNodePointer = iNode->iNodeId;
	node->processCount = 0;
	updateINodeBlock(node);
	//inodeBlock-user
	INodeList *node2 = new INodeList;
	node2->iNode.address[0] = NUMBER_OF_THE_START_OF_THE_DATABLOCK + SUM_OF_GROUPLINK - 4;
	node2->iNode.createTime = getTimeStamp();
	strcpy_s(node2->iNode.groupID, "ROOT");
	node2->iNode.linkOfFile = 1;
	node2->iNode.modeOfFile = 2000;
	node2->iNode.modifyTime = getTimeStamp();
	strcpy_s(node2->iNode.ownerID, "root");
	node2->iNode.sizeOfFile = 2 * sizeof(User);
	node2->iNodeId = 3;
	node2->iNodePointer = node->iNodeId;
	node2->processCount = 0;
	updateINodeBlock(node2);

	//dir-super-root
	Directory *dir22 = new Directory;
	strcpy_s(dir22->directoryNode[0].directoryName, "root");
	dir22->directoryNode[0].iNodeId = 1;
	dir22->numberOfDirectoryNode = 1;
	writeIntoBlock(dir22, NUMBER_OF_THE_START_OF_THE_DATABLOCK + SUM_OF_GROUPLINK - 1, 0, sizeof(Directory), 1);
	//dir-root
	Directory *dir = new Directory;
	strcpy_s(dir->directoryNode[0].directoryName, "cd.");
	dir->directoryNode[0].iNodeId = 1;
	strcpy_s(dir->directoryNode[1].directoryName, "cd..");
	dir->directoryNode[1].iNodeId = 0;
	strcpy_s(dir->directoryNode[2].directoryName, "etc");
	dir->directoryNode[2].iNodeId = 2;
	dir->numberOfDirectoryNode = 3;
	writeIntoBlock(dir, NUMBER_OF_THE_START_OF_THE_DATABLOCK + SUM_OF_GROUPLINK - 2, 0, sizeof(Directory), 1);
	//dir-etc
	Directory *dir2 = new Directory;
	strcpy_s(dir2->directoryNode[0].directoryName, "cd.");
	dir2->directoryNode[0].iNodeId = 2;
	strcpy_s(dir2->directoryNode[1].directoryName, "cd..");
	dir2->directoryNode[1].iNodeId = 1;
	strcpy_s(dir2->directoryNode[2].directoryName, "user");
	dir2->directoryNode[2].iNodeId = 3;
	dir2->numberOfDirectoryNode = 3;
	writeIntoBlock(dir2, NUMBER_OF_THE_START_OF_THE_DATABLOCK + SUM_OF_GROUPLINK - 3, 0, sizeof(Directory), 1);

	//user-root
	User *user1 = new User;
	strcpy_s(user1->userGroup, "ROOT");
	strcpy_s(user1->userName, "root");
	strcpy_s(user1->userPassword, "root");
	writeIntoBlock(user1, NUMBER_OF_THE_START_OF_THE_DATABLOCK + SUM_OF_GROUPLINK - 4, 0, sizeof(User), 1);
	//user-qwert
	User *user2 = new User;
	strcpy_s(user2->userGroup, "USER");
	strcpy_s(user2->userName, "qwert");
	strcpy_s(user2->userPassword, "qwert");
	writeIntoBlock(user2, NUMBER_OF_THE_START_OF_THE_DATABLOCK + SUM_OF_GROUPLINK - 4,  sizeof(User), sizeof(User), 1);

	//groupLink
	int idInGroup[SUM_OF_GROUPLINK];
	fseek(temporalFile, NUMBER_OF_THE_START_OF_THE_DATABLOCK*SIZE_OF_ONE_BLOCK, SEEK_SET);
	for (int i = 0; i < SUM_OF_GROUPLINK; i++)
		idInGroup[i] = NUMBER_OF_THE_START_OF_THE_DATABLOCK + i;
	for (int i = 0; i < SUM_OF_DATAGROUP -2; i++)
	{
		for (int j = 0; j < SUM_OF_GROUPLINK; j++)
			idInGroup[j] += SUM_OF_GROUPLINK;
		writeIntoBlock(&idInGroup, NUMBER_OF_THE_START_OF_THE_DATABLOCK + i * SUM_OF_GROUPLINK, 0, 240, 1);
	}
	unsigned long int offset = (NUMBER_OF_THE_START_OF_THE_DATABLOCK + (SUM_OF_DATAGROUP - 1) * SUM_OF_GROUPLINK)*SIZE_OF_ONE_BLOCK;
	fseek(temporalFile, offset, SEEK_SET);
	idInGroup[0] = 0;
	fwrite(&idInGroup[0], sizeof(int), 1, temporalFile);

	//delete
	delete iNode11;
	iNode11 = nullptr;
	delete iNode;
	iNode = nullptr;
	delete node;
	node = nullptr;
	delete node2;
	node2 = nullptr;
	delete dir22;
	dir22 = nullptr;
	delete dir;
	dir = nullptr;
	delete dir2;
	dir2 = nullptr;
	delete user1;
	user1 = nullptr;
	delete user2;
	user2 = nullptr;
	delete superBlock;
	superBlock = nullptr;
}


int typeOfIndex(int size)
{
	if (size <= 5120)
		return 0;
	else if (size <= 65536)
		return 1;
	else if (size <= 8388608)
		return 2;
	else
		return ERROR_FILE_SIZE_MORE_LIMIT;
}


int numOfaddr(int size,int &num,int &surplusSize)
{
	int index = typeOfIndex(size);
	if (index == 0 || index == 1)
	{
		num = size / 512;
		surplusSize = size % 512;
		return SUCCESS;
	}
	else
		return index;
}


int numOfaddr(int size, int &num)
{
	int index = typeOfIndex(size);
	if (index == 0 || index == 1)
	{
		num = size / 512;
		return SUCCESS;
	}
	else
		return index;
}


int numOfaddrI3(int size, int &num1, int &num2, int &surplusSize)
{
	int index = typeOfIndex(size);
	if (index == 2)
	{
		num1 = size / 65536;
		size = size % 65536;
		num2 = size / 512;
		surplusSize = size % 512;
		return SUCCESS;
	}
	else
		return index;
}


int numOfaddrI3(int size, int &num1, int &num2)
{
	int index = typeOfIndex(size);
	if (index == 2)
	{
		num1 = size / 65536;
		size = size % 65536;
		num2 = size / 512;
		return SUCCESS;
	}
	else
		return index;
}


int numOfSplit(const char *path)
{
	int num = 0;
	for (int i = 0; i < 66; i++)
	{
		if (path[i] == '/')
			num++;
	}
	return num;
}


int split(const char *path,int num,char *&name,int &length)
{
	int sum = numOfSplit(path);
	int flag = 0;
	for (int i = 0; i < 66; i++)
	{
		if (path[i] == '/')
			flag++;
		if (flag == num)
		{
			for (int j = i + 1; j < i + 1 + LENGTH_OF_FILENAME; j++)
				if (path[j] != '/')
				{
					name[j - i - 1] = path[j];
					length++;
				}
				else
					break;
			break;
		}
	}
	return SUCCESS;
}


int judgePathRule(const char *path)
{
	char *name = new char[12];
	int length = 0;
	memset(name, '\0', 12);
	split(path, 1, name, length);
	if (strcmp(name, "root") || path[0] != '/')
	{
		delete[] name;
		name = nullptr;
		return ERROR_PATH_WRONG;
	}
	delete[] name;
	name = nullptr;
	return SUCCESS;
}


int judgePath(const char *path)
{
	int numOfName = numOfSplit(path);
	char *name = new char[12];
	int length = 0;
	INodeList temp = *root;
	Directory dir;
	bool isFind = false;
	for (int i = 2; i < numOfName+1; i++)
	{
		isFind = false;
		memset(name, '\0', 12);
		readFromBlock(&dir, temp.iNode.address[0], 0, sizeof(Directory), 1);
		length = 0;
		split(path, i, name, length);
		for (int j = 2; j < dir.numberOfDirectoryNode; j++)
		{
			if (!strcmp(dir.directoryNode[j].directoryName, name) && (temp.iNode.modeOfFile / 1000 == 1))
			{
				isFind = true;
				temp = iNodeA[dir.directoryNode[j].iNodeId];
				break;
			}
		}
		if (isFind == false)
		{
			delete[] name;
			name = nullptr;
			return ERROR_PATH_NONEXISTENT;
		}
	}
	delete[] name;
	name = nullptr;
	return SUCCESS;
}


int printMode(int mode)
{
	int authority = mode % 1000;
	if (mode / 1000 == 1)
		cout << "d";
	else
		cout << "-";
	int div = 100;
	for (int i = 0; i < 3; i++)
	{
		int num = authority / div;
		authority = authority % div;
		int a[3] = { 0 };
		int b = 2;
		while (num != 0)
		{
			a[b--] = num % 2;
			num = num / 2;
		}
		for (int i = 0; i < 3; i++)
		{
			if (a[i] == 1)
			{
				if (i == 2)
					cout << "x";
				else if (i == 1)
					cout << "w";
				else if (i == 0)
					cout << "r";
			}
			else
				cout << "-";
		}
		div /= 10;
	}
	return SUCCESS;
}


int printErrorInfo(int statusCode)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
	switch (statusCode)
	{
	case SUCCESS:
		break;
	case ERROR_FILE_OPEN_FAIL:
		cerr << "Open the specified file fail. Please sure the file is existent!" << endl;
		break;
	case ERROR_SUPERBLOCK_READ_FAIL:
		cerr << "Read superblock fail!" << endl;
		break;
	case ERROR_FILE_LOAD_FAIL:
		cerr << "Load the specified file fail. Please sure the file is existent" << endl;
		break;
	case ERROR_INODEBLOCK_UPDATE_FAIL:
		cerr << "Update the inodeblock fail!" << endl;
		break;
	case ERROR_BLOCK_WRITE_FAIL:
		cerr << "There are some accdents happened with writing into the block!" << endl;
		break;
	case ERROR_BLOCK_READ_FAIL:
		cerr << "There are some accdents happened with reading from the block!" << endl;
		break;
	case ERROR_INODE_WORKOUT:
		cerr << "The inode is workout!" << endl;
		break;
	case ERROR_INODE_READ_FAIL:
		cerr << "There are some accdents happened with reading from the inode!" << endl;
		break;
	case ERROR_FREEBLOCK_READ_FAIL:
		cerr << "There are some accdents happened with reading from the free block!" << endl;
		break;
	case ERROR_BLOCK_WORKOUT:
		cerr << "The block is workout!" << endl;
		break;
	case ERROR_DATABLOCK_INITIALIZE_FAIL:
		cerr << "There are some accdents happened with initializing the datablock!" << endl;
		break;
	case ERROR_PASSWORD_WRONG:
		cerr << "The password is not matched! Please pay attention to capitalization and input again." << endl;
		break;
	case ERROR_USER_NONEXISTENT:
		cerr << "The user is nonexistent! Please pay attention to capitalization and input again." << endl;
		break;
	case ERROR_FILE_MODE_WRONG:
		cerr << "It is not a directoey!" << endl;
		break;
	case ERROR_MORE_NUMBER_LIMIT_IN_DIRECTORY:
		cerr << "You must limit the length of directory name and use the command 'help' for details." << endl;
		break;
	case ERROR_DIR_NAME_EXIST:
		cerr << "The name has existed." << endl;
		break;
	case ERROR_SUPERBLOCK_UPDATE_FAIL:
		cerr << "There are some accdents happened with updating from the superblock!" << endl;
		break;
	case ERROR_UNKNOWN:
		cerr << "Unknown error! Please contact developer and we will return you a content answer." << endl;
		break;
	case ERROR_INODE_NONEXISTENT:
		cerr << "Inode is nonexistent!" << endl;
		break;
	case ERROR_FILE_SIZE_MORE_LIMIT:
		cerr << "The size of the file exceeds limit!" << endl;
		break;
	case ERROR_PATH_WRONG:
		cerr << "This is not a valid absolute path!" << endl;
		break;
	case ERROR_PATH_NONEXISTENT:
		cerr << "The path does not exist!" << endl;
		break;
	case ERROR_USER_EXISTENT:
		cerr << "Username has existed." << endl;
		break;
	case ERROR_DIR_NONEXISTENT:
		cerr << "Directory does not exist!" << endl;
		break;
	case ERROR_COMMAND_NONEXISTENT:
		cerr << "Command does not exist!" << endl;
		break;
	case ERROR_FILE_WRONG:
		cerr << "This is not a file!" << endl;
		break;
	case ERROR_FILE_NONEXISTENT:
		cerr << "Can not find the file!" << endl;
		break;
	case ERROR_AUTHORITY:
		cerr << "You have not authority to do it!" << endl;
		break;
	}

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	return statusCode;
}