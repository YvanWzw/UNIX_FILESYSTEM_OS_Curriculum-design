
#include "order.h"
#include <string>
#include <conio.h>
#include <cstdlib>
#include <windows.h>
#include <iomanip>
using namespace std;


User currentUser;
int currentUserId;


int login()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	User *user = new User[10];
	char username[LENGTH_OF_OWNER_ID];
	char password[LENGTH_OF_OWNER_PASSWORD];
	cd("etc");
	int sumOfUser = iNodeA[currentDir->directoryNode[2].iNodeId].iNode.sizeOfFile / SIZE_OF_USER;
	int statusCode = readFromBlock(user, current->iNode.address[2], 0, SIZE_OF_USER, sumOfUser);
	if (statusCode != SUCCESS)
		return statusCode;
	cout << "USERNAME:";
	gets_s(username, LENGTH_OF_OWNER_ID);
	for (int i = 0; i < sumOfUser; i++)
		if (!strcmp(user[i].userName, username))
		{
			cout << "PASSWORD:";
			hiddenPassword(password);
			if (!strcmp(user[i].userPassword, password))
			{
				strcpy_s(currentUser.userName, user[i].userName);
				strcpy_s(currentUser.userPassword, user[i].userPassword);
				strcpy_s(currentUser.userGroup, user[i].userGroup);
				currentUserId = i;
				current = root;
				statusCode = readFromBlock(currentDir, NUMBER_OF_THE_START_OF_THE_DATABLOCK + SUM_OF_GROUPLINK - 2, 0, sizeof(Directory), 1);
				delete[] user;
				user = NULL;
				system("cls");
				return statusCode;
			}
			else
			{
				delete[] user;
				user = NULL;
				return ERROR_PASSWORD_WRONG;
			}
		}
	delete[] user;
	user = NULL;
	return ERROR_USER_NONEXISTENT;
}


int createNewUser()
{
	Directory *dir = new Directory;
	User *user1 = new User[10];
	char username[LENGTH_OF_OWNER_ID];
	char password[LENGTH_OF_OWNER_PASSWORD];
	int statusCode = readFromBlock(dir, root->iNode.address[2], 0, SIZE_OF_DIRECTORY, 1);
	if (statusCode != SUCCESS)
		return statusCode;
	int sumOfUser = iNodeA[dir->directoryNode[2].iNodeId].iNode.sizeOfFile / SIZE_OF_USER;
	statusCode = readFromBlock(user1, iNodeA[dir->directoryNode[2].iNodeId].iNode.address[0], 0, SIZE_OF_USER, sumOfUser);
	if (statusCode != SUCCESS)
		return statusCode;
	cout << "USERNAME:";
	gets_s(username, LENGTH_OF_OWNER_ID);
	for (int i = 0; i < sumOfUser; i++)
		if (!strcmp(user1[i].userName, username))
			return ERROR_USER_EXISTENT;
	cout << "PASSWORD:";
	gets_s(password, LENGTH_OF_OWNER_PASSWORD);
	strcpy_s(user1->userName, username);
	strcpy_s(user1->userPassword, password);
	strcpy_s(user1->userGroup, "ROOT");
	statusCode = writeIntoBlock(user1, iNodeA[dir->directoryNode[2].iNodeId].iNode.address[0], sumOfUser*SIZE_OF_USER, SIZE_OF_USER, 1);
	iNodeA[dir->directoryNode[2].iNodeId].iNode.sizeOfFile += sizeof(User);
	cout << "USER ADD SUCCESS" << endl;
	delete dir;
	dir = nullptr;
	delete[] user1;
	user1 = nullptr;
	return statusCode;
}


int mkdir(const char *name)
{
	if (current->iNode.modeOfFile / 1000 != 1)
		return ERROR_FILE_MODE_WRONG;
	int count = currentDir->numberOfDirectoryNode;
	if (count > SUM_OF_DIRECTORY_IN_ONE_DIR)
		return ERROR_MORE_NUMBER_LIMIT_IN_DIRECTORY;
	for (int i = 0; i < count; i++)
		if (strcmp(currentDir->directoryNode[i].directoryName, name) == 0)
			return ERROR_DIR_NAME_EXIST;
	current->iNode.sizeOfFile += sizeof(DirectoryNode);
	strcpy_s(currentDir->directoryNode[count].directoryName, name);
	int newINodeId = superBlock->pointerOfFreeINode;
	iNodeA[newINodeId].iNode.modeOfFile = 1774;
	linkINode(newINodeId);
	currentDir->directoryNode[count].iNodeId = newINodeId;
	currentDir->numberOfDirectoryNode++;
	int addressValue = 0;
	allocateFreeBlock(addressValue);
	iNodeA[newINodeId].iNode.address[0] = addressValue;
	iNodeA[newINodeId].iNode.address[1] = current->iNode.address[0];
	iNodeA[newINodeId].iNode.sizeOfFile = 0;
	iNodeA[newINodeId].iNode.linkOfFile = 2;
	current->iNode.address[count] = addressValue;
	writeIntoBlock(currentDir, current->iNode.address[0], 0, sizeof(Directory), 1);
	Directory *newDir = new Directory;
	strcpy_s(newDir->directoryNode[0].directoryName, "cd.");
	strcpy_s(newDir->directoryNode[1].directoryName, "cd..");
	newDir->directoryNode[0].iNodeId = newINodeId;
	newDir->directoryNode[1].iNodeId = current->iNodeId;
	newDir->numberOfDirectoryNode = 2;
	writeIntoBlock(newDir, addressValue, 0, sizeof(Directory), 1);
	delete newDir;
	newDir = nullptr;
	return SUCCESS;
}


int rmdir(const char *name)
{
	if (current->iNode.modeOfFile / 1000 != 1)
		return ERROR_FILE_MODE_WRONG;
	int count = currentDir->numberOfDirectoryNode;
	bool flag = false;
	if (count > SUM_OF_DIRECTORY_IN_ONE_DIR)
		return ERROR_MORE_NUMBER_LIMIT_IN_DIRECTORY;
	for (int i = 0; i < count; i++)
		if (strcmp(currentDir->directoryNode[i].directoryName, name) == 0)
		{
			count = i;
			flag = true;
			break;
		}
	if (flag == false)
		return ERROR_DIR_NONEXISTENT;
	if (iNodeA[currentDir->directoryNode[count].iNodeId].iNode.modeOfFile / 1000 != 1)
		return ERROR_FILE_MODE_WRONG;
	Directory dirRoot;
	readFromBlock(&dirRoot, iNodeA[currentDir->directoryNode[count].iNodeId].iNode.address[0], 0, sizeof(Directory), 1);
	perOrderForDel(dirRoot);
	current->iNode.sizeOfFile -= sizeof(DirectoryNode);
	currentDir->numberOfDirectoryNode--;
	freeTheBlock(current->iNode.address[count]);
	freeTheINode(current->iNodeId);
	for (int i = count; i < currentDir->numberOfDirectoryNode; i++)
	{
		currentDir->directoryNode[i] = currentDir->directoryNode[i + 1];
		current->iNode.address[i] = current->iNode.address[i + 1];
	}
	writeIntoBlock(currentDir, current->iNode.address[0], 0, sizeof(Directory), 1);
	return SUCCESS;
}


int perOrderForDel(Directory dirRoot)
{
	if (dirRoot.numberOfDirectoryNode != 2)
	{
		Directory temp_dir = dirRoot;
		for (int i = 2; i < dirRoot.numberOfDirectoryNode; i++)
		{
			dirRoot = temp_dir;
			if (iNodeA[dirRoot.directoryNode[i].iNodeId].iNode.modeOfFile / 1000 == 1)
			{
				readFromBlock(&dirRoot, iNodeA[dirRoot.directoryNode[i].iNodeId].iNode.address[0], 0, sizeof(Directory), 1);
				perOrderForDel(dirRoot);
				freeTheINode(dirRoot.directoryNode[i].iNodeId);
			}
			else
				rm(dirRoot.directoryNode[i].directoryName, &dirRoot);
			freeTheBlock(current->iNode.address[i]);
		}
	}
	return SUCCESS;
}


int perOrderForSize(Directory dirRoot, int &size)
{
	if (dirRoot.numberOfDirectoryNode != 0)
	{
		Directory temp_dir = dirRoot;
		for (int i = 2; i < dirRoot.numberOfDirectoryNode; i++)
		{
			dirRoot = temp_dir;
			if (iNodeA[dirRoot.directoryNode[i].iNodeId].iNode.modeOfFile / 1000 == 1)
			{
				size += iNodeA[dirRoot.directoryNode[i].iNodeId].iNode.sizeOfFile;
				readFromBlock(&dirRoot, iNodeA[dirRoot.directoryNode[i].iNodeId].iNode.address[0], 0, sizeof(Directory), 1);
				perOrderForSize(dirRoot, size);
			}
			else
				size += iNodeA[dirRoot.directoryNode[i].iNodeId].iNode.sizeOfFile;
		}
	}
	return SUCCESS;
}


int ls()
{
	if (current->iNode.modeOfFile / 1000 != 1)
		return ERROR_FILE_MODE_WRONG;
	int size = 0;
	Directory dirRoot = *currentDir;
	cout << left << setw(12) << "FILENAME" << setw(15) << "MODE" << setw(8) << "LINK" << setw(10) << "USER" << setw(10) << "GROUP" << setw(12) << "SIZE(BETY)" << endl;
	for (int i = 0; i < currentDir->numberOfDirectoryNode; i++)
	{
		size = 0;
		dirRoot = *currentDir;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE);
		cout << left << setw(12) << currentDir->directoryNode[i].directoryName;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);

		if (i >= 2)
		{
			printMode(iNodeA[currentDir->directoryNode[i].iNodeId].iNode.modeOfFile);
			cout << right << setw(8) << iNodeA[currentDir->directoryNode[i].iNodeId].iNode.linkOfFile;
			cout << setw(10) << iNodeA[currentDir->directoryNode[i].iNodeId].iNode.ownerID;
			cout << setw(10) << iNodeA[currentDir->directoryNode[i].iNodeId].iNode.groupID;
			if (iNodeA[dirRoot.directoryNode[i].iNodeId].iNode.modeOfFile / 1000 == 1)
			{
				size += iNodeA[dirRoot.directoryNode[i].iNodeId].iNode.sizeOfFile;
				readFromBlock(&dirRoot, iNodeA[dirRoot.directoryNode[i].iNodeId].iNode.address[0], 0, sizeof(Directory), 1);
				perOrderForSize(dirRoot, size);
			}
			else
				size += iNodeA[dirRoot.directoryNode[i].iNodeId].iNode.sizeOfFile;
			cout << setw(12) << size;
		}
		cout << endl;
	}
	return SUCCESS;
}


int cd_return()
{
	if (current->iNodeId != 1)
	{
		current = &iNodeA[current->iNodePointer];
		readFromBlock(currentDir, current->iNode.address[0], 0, sizeof(Directory), 1);
	}
	return SUCCESS;
}


int cd_return_file()
{
	current = &iNodeA[current->iNodePointer];
	return SUCCESS;
}


int cd_file(const char *path)
{
	for (int i = 2; i < currentDir->numberOfDirectoryNode; i++)
		if (!strcmp(currentDir->directoryNode[i].directoryName, path))
		{
			if (iNodeA[currentDir->directoryNode[i].iNodeId].iNode.modeOfFile / 1000 == 1)
				return ERROR_FILE_MODE_WRONG;
			int oldCurrent = current->iNodeId;
			current = &iNodeA[currentDir->directoryNode[i].iNodeId];
			current->iNodeId = currentDir->directoryNode[i].iNodeId;
			current->iNodePointer = oldCurrent;
			linkINode(current->iNodeId);
			break;
		}
	return SUCCESS;
}


int cd(const char *path)
{
	int isFind = false;
	for (int i = 2; i < currentDir->numberOfDirectoryNode; i++)
		if (!strcmp(currentDir->directoryNode[i].directoryName, path))
		{
			if (iNodeA[currentDir->directoryNode[i].iNodeId].iNode.modeOfFile / 1000 != 1)
				return ERROR_FILE_MODE_WRONG;
			int oldCurrent = current->iNodeId;
			current = &iNodeA[currentDir->directoryNode[i].iNodeId];
			current->iNodeId = currentDir->directoryNode[i].iNodeId;
			current->iNodePointer = oldCurrent;
			readFromBlock(currentDir, current->iNode.address[0], 0, sizeof(Directory), 1);
			linkINode(current->iNodeId);
			isFind = true;
			break;
		}
	if (isFind == false)
		return ERROR_FILE_NONEXISTENT;
	return SUCCESS;
}


int cd_absolute(const char *path)
{
	cd_root();
	int numOfNamex = numOfSplit(path);
	char *name = new char[20];
	char namex[12] = { 0 };
	int length = 0;
	for (int k = 2; k < numOfNamex + 1; k++)
	{
		memset(namex, '\0', 12);
		length = 0;
		split(path, k, name, length);
		for (int i = 0; i < length; i++)
			namex[i] = name[i];
		for (int i = 2; i < currentDir->numberOfDirectoryNode; i++)
			if (!strcmp(currentDir->directoryNode[i].directoryName, namex))
			{
				if (iNodeA[currentDir->directoryNode[i].iNodeId].iNode.modeOfFile / 1000 != 1)
				{
					delete[] name;
					name = nullptr;
					return ERROR_FILE_MODE_WRONG;
				}
				int oldCurrent = current->iNodeId;
				current = &iNodeA[currentDir->directoryNode[i].iNodeId];
				current->iNodeId = currentDir->directoryNode[i].iNodeId;
				current->iNodePointer = oldCurrent;
				readFromBlock(currentDir, current->iNode.address[0], 0, sizeof(Directory), 1);
				linkINode(current->iNodeId);
				break;
			}
	}
	delete[] name;
	name = nullptr;
	return SUCCESS;
}


int cd_root()
{
	current = root;
	readFromBlock(currentDir, current->iNode.address[0], 0, sizeof(Directory), 1);
	return SUCCESS;
}


int pwd()
{
	int iNodeId = current->iNodeId;
	int parentId = 0;
	string temp_path = "";
	string path = "";
	Directory temp_dir = *currentDir;
	while (temp_dir.directoryNode[0].iNodeId != 1)
	{
		parentId = temp_dir.directoryNode[1].iNodeId;
		iNodeId = temp_dir.directoryNode[0].iNodeId;
		readFromBlock(&temp_dir, iNodeA[parentId].iNode.address[0], 0, sizeof(Directory), 1);
		for (int i = 2; i < temp_dir.numberOfDirectoryNode; i++)
		{
			if (temp_dir.directoryNode[i].iNodeId == iNodeId)
			{
				temp_path = temp_dir.directoryNode[i].directoryName;
				temp_path = "/" + temp_path;
				temp_path += path;
				path = temp_path;
				break;
			}
		}
	}
	temp_path = "/root";
	path = temp_path + path;
	cout << path;
	return SUCCESS;
}


int passwd()
{
	int count = 0;
	Directory *dir = new Directory;
	User *user2 = new User[10];
	char password[LENGTH_OF_OWNER_PASSWORD];
	char rePassword[LENGTH_OF_OWNER_PASSWORD];
	int statusCode = readFromBlock(dir, root->iNode.address[2], 0, SIZE_OF_DIRECTORY, 1);
	if (statusCode != SUCCESS)
	{
		delete dir;
		dir = nullptr;
		delete[] user2;
		user2 = nullptr;
		return statusCode;
	}
	int sumOfUser = iNodeA[dir->directoryNode[2].iNodeId].iNode.sizeOfFile / SIZE_OF_USER;
	statusCode = readFromBlock(user2, iNodeA[dir->directoryNode[2].iNodeId].iNode.address[0], 0, SIZE_OF_USER, sumOfUser);
	if (statusCode != SUCCESS)
	{
		delete dir;
		dir = nullptr;
		delete[] user2;
		user2 = nullptr;
		return statusCode;
	}
	for (int i = 0; i < sumOfUser; i++)
		if (!strcmp(user2[i].userName, currentUser.userName))
		{
			delete dir;
			dir = nullptr;
			delete[] user2;
			user2 = nullptr;
			count = i;
			break;
		}
	cout << "OLD PASSWORD:";
	gets_s(password, LENGTH_OF_OWNER_PASSWORD);
	if (strcmp(currentUser.userPassword, password))
	{
		delete dir;
		dir = nullptr;
		delete[] user2;
		user2 = nullptr;
		return ERROR_PASSWORD_WRONG;
	}
	memset(password, '\0', sizeof(password));
	cout << "NEW PASSWORD:";
	gets_s(password, LENGTH_OF_OWNER_PASSWORD);
	cout << "REPEAT NEW PASSWORD:";
	gets_s(rePassword, LENGTH_OF_OWNER_PASSWORD);
	if (strcmp(rePassword, password))
	{
		delete dir;
		dir = nullptr;
		delete[] user2;
		user2 = nullptr;
		cout << "NEW PASSWORD IS WRONG!" << endl;
		return ERROR_PASSWORD_WRONG;
	}
	strcpy_s(user2->userName, currentUser.userName);
	strcpy_s(user2->userPassword, password);
	strcpy_s(user2->userGroup, currentUser.userGroup);
	statusCode = writeIntoBlock(user2, iNodeA[dir->directoryNode[2].iNodeId].iNode.address[0], count*SIZE_OF_USER, SIZE_OF_USER, 1);
	if (statusCode != SUCCESS)
	{
		delete dir;
		dir = nullptr;
		delete[] user2;
		user2 = nullptr;
		return statusCode;
	}
	cout << "PASSWORD REVISE SUCCESS" << endl;
	delete dir;
	dir = nullptr;
	delete[] user2;
	user2 = nullptr;
	return SUCCESS;
}


int mv(const char *oldname, const char *newname)
{
	for (int i = 2; i < currentDir->numberOfDirectoryNode; i++)
		if (!strcmp(currentDir->directoryNode[i].directoryName, oldname))
		{
			strcpy_s(currentDir->directoryNode[i].directoryName, newname);
			writeIntoBlock(currentDir, iNodeA[currentDir->directoryNode[i].iNodeId].iNode.address[0], 0, sizeof(Directory), 1);
			break;
		}
	return SUCCESS;
}


int chmod(const char *name, unsigned short int mode)
{
	for (int i = 2; i < currentDir->numberOfDirectoryNode; i++)
		if (!strcmp(currentDir->directoryNode[i].directoryName, name))
		{
			int type = iNodeA[currentDir->directoryNode[i].iNodeId].iNode.modeOfFile / 1000;
			iNodeA[currentDir->directoryNode[i].iNodeId].iNode.modeOfFile = mode + 1000 * type;
			break;
		}
	return SUCCESS;
}


int chown(const char *name, const char *owner)
{
	for (int i = 2; i < currentDir->numberOfDirectoryNode; i++)
		if (!strcmp(currentDir->directoryNode[i].directoryName, name))
		{
			strcpy_s(iNodeA[currentDir->directoryNode[i].iNodeId].iNode.ownerID, owner);
			break;
		}
	return SUCCESS;
}


int chgrp(const char *name, const char *group)
{
	for (int i = 2; i < currentDir->numberOfDirectoryNode; i++)
		if (!strcmp(currentDir->directoryNode[i].directoryName, name))
		{
			strcpy_s(iNodeA[currentDir->directoryNode[i].iNodeId].iNode.groupID, group);
			break;
		}
	return SUCCESS;
}


int cat(const char *name)
{
	bool isFind = false;
	for (int i = 2; i < currentDir->numberOfDirectoryNode; i++)
		if (!strcmp(currentDir->directoryNode[i].directoryName, name))
		{
			if (iNodeA[currentDir->directoryNode[i].iNodeId].iNode.modeOfFile / 1000 == 1)
				return ERROR_FILE_WRONG;
			if (iNodeA[currentDir->directoryNode[i].iNodeId].iNode.modeOfFile % 1000 == 0)
				return ERROR_AUTHORITY;
			int size = iNodeA[currentDir->directoryNode[i].iNodeId].iNode.sizeOfFile;
			int num1 = 0;
			int num2 = 0;
			int surplus = 0;
			char append[513] = { 0 };
			if (typeOfIndex(size) == 0)
			{
				numOfaddr(size, num1, surplus);

				for (int j = 0; j < num1; j++)
				{
					readFromBlock(append, iNodeA[currentDir->directoryNode[i].iNodeId].iNode.address[j], 0, 512, 1);
					for (int k = 0; k < 512; k++)
						cout << append[k];
					memset(append, '\0', sizeof(append));
				}
				readFromBlock(append, iNodeA[currentDir->directoryNode[i].iNodeId].iNode.address[num1], 0, surplus, 1);
				for (int j = 0; j < surplus; j++)
					cout << append[j];
			}
			if (typeOfIndex(size) == 1)
			{
				int block[128] = { 0 };
				numOfaddr(size, num1, surplus);
				readFromBlock(block, iNodeA[currentDir->directoryNode[i].iNodeId].iNode.address[10], 0, sizeof(int), num1 + 1);
				for (int j = 0; j < num1; j++)
				{
					readFromBlock(append, block[j], 0, 512, 1);
					for (int k = 0; k < 512; k++)
						cout << append[k];
					memset(append, '\0', sizeof(append));
				}
				readFromBlock(append, block[num1], 0, surplus, 1);
				for (int j = 0; j < surplus; j++)
					cout << append[j];
			}
			if (typeOfIndex(size) == 2)
			{
				numOfaddrI3(size, num1, num2, surplus);
				int block1[128] = { 0 };
				int block2[128] = { 0 };
				readFromBlock(block1, iNodeA[currentDir->directoryNode[i].iNodeId].iNode.address[11], 0, sizeof(int), num1 + 1);
				for (int j = 0; j < num1; j++)
				{
					readFromBlock(block2, block1[j], 0, sizeof(int), 128);
					for (int k = 0; k < 128; k++)
					{
						readFromBlock(append, block2[k], 0, 512, 1);
						for (int m = 0; m < 512; m++)
							cout << append[m];
						memset(append, '\0', sizeof(append));
					}
				}
				readFromBlock(block2, block1[num1], 0, sizeof(int), num2 + 1);

				for (int j = 0; j < num2; j++)
				{
					readFromBlock(append, block2[j], 0, 512, 1);
					for (int k = 0; k < 512; k++)
						cout << append[k];
					memset(append, '\0', sizeof(append));
				}
				readFromBlock(append, block2[num2], 0, surplus, 1);
				for (int j = 0; j < surplus; j++)
					cout << append[j];
			}
			isFind = true;
			break;
		}
	if (isFind == false)
		return ERROR_FILE_NONEXISTENT;
	cout << endl;
	return SUCCESS;
}


int touch(const char* name)
{
	if (current->iNode.modeOfFile / 1000 != 1)
		return ERROR_FILE_MODE_WRONG;
	int count = currentDir->numberOfDirectoryNode;
	if (count > SUM_OF_DIRECTORY_IN_ONE_DIR)
		return ERROR_MORE_NUMBER_LIMIT_IN_DIRECTORY;
	for (int i = 0; i < count; i++)
		if (strcmp(currentDir->directoryNode[i].directoryName, name) == 0)
			return ERROR_DIR_NAME_EXIST;
	current->iNode.sizeOfFile += sizeof(DirectoryNode);
	strcpy_s(currentDir->directoryNode[count].directoryName, name);
	int newINodeId = superBlock->pointerOfFreeINode;
	iNodeA[newINodeId].iNode.modeOfFile = 2774;
	linkINode(newINodeId);
	currentDir->directoryNode[count].iNodeId = newINodeId;
	currentDir->numberOfDirectoryNode++;
	int addressValue = 0;
	allocateFreeBlock(addressValue);
	iNodeA[newINodeId].iNode.sizeOfFile = 0;
	iNodeA[newINodeId].iNode.linkOfFile = 1;
	current->iNode.address[count] = addressValue;
	writeIntoBlock(currentDir, current->iNode.address[0], 0, sizeof(Directory), 1);
	return SUCCESS;
}


int rm(const char *name)
{
	for (int i = 2; i < currentDir->numberOfDirectoryNode; i++)
	{
		if (!strcmp(currentDir->directoryNode[i].directoryName, name))
		{
			if (iNodeA[currentDir->directoryNode[i].iNodeId].iNode.modeOfFile / 1000 == 1)
				return ERROR_FILE_WRONG;
			if (iNodeA[currentDir->directoryNode[i].iNodeId].iNode.modeOfFile % 1000 == 0)
				return ERROR_AUTHORITY;
			int size = iNodeA[currentDir->directoryNode[i].iNodeId].iNode.sizeOfFile;
			if (typeOfIndex(size) == 0)
			{
				int num = 0;
				numOfaddr(size, num);
				for (int j = 0; j < num + 1; j++)
					freeTheBlock(iNodeA[currentDir->directoryNode[i].iNodeId].iNode.address[j]);
			}
			else if (typeOfIndex(size) == 1)
			{
				int num = 0;
				numOfaddr(size, num);
				int block[128] = { 0 };
				readFromBlock(block, iNodeA[currentDir->directoryNode[i].iNodeId].iNode.address[10], 0, sizeof(int), num + 1);
				for (int j = 0; j < num + 1; j++)
					freeTheBlock(block[j]);
				freeTheBlock(iNodeA[currentDir->directoryNode[i].iNodeId].iNode.address[10]);
			}
			else if (typeOfIndex(size) == 2)
			{
				int num1 = 0;
				int num2 = 0;
				int block1[128] = { 0 };
				int block2[128] = { 0 };
				numOfaddrI3(size, num1, num2);
				readFromBlock(block1, iNodeA[currentDir->directoryNode[i].iNodeId].iNode.address[11], 0, sizeof(int), num1 + 1);
				for (int j = 0; j < num1 + 1; j++)
				{
					readFromBlock(block2, block1[j], 0, sizeof(int), num2 + 1);
					for (int k = 0; k < num2 + 1; k++)
						freeTheBlock(block2[k]);
					freeTheBlock(block1[j]);
				}
			}
			freeTheINode(currentDir->directoryNode[i].iNodeId);
			current->iNode.sizeOfFile -= sizeof(DirectoryNode);
			currentDir->numberOfDirectoryNode--;
			for (int j = i; j < currentDir->numberOfDirectoryNode; j++)
			{
				currentDir->directoryNode[j] = currentDir->directoryNode[j + 1];
				current->iNode.address[j] = current->iNode.address[j + 1];
			}
			writeIntoBlock(currentDir, current->iNode.address[0], 0, sizeof(Directory), 1);
			break;
		}
	}
	return SUCCESS;
}


int rm(const char *name, Directory *dir)
{
	for (int i = 2; i < dir->numberOfDirectoryNode; i++)
	{
		if (!strcmp(dir->directoryNode[i].directoryName, name))
		{
			if (iNodeA[dir->directoryNode[i].iNodeId].iNode.modeOfFile / 1000 == 1)
				return ERROR_FILE_WRONG;
			int size = iNodeA[dir->directoryNode[i].iNodeId].iNode.sizeOfFile;
			if (typeOfIndex(size) == 0)
			{
				int num = 0;
				numOfaddr(size, num);
				for (int j = 0; j < num + 1; j++)
					freeTheBlock(iNodeA[dir->directoryNode[i].iNodeId].iNode.address[j]);
			}
			else if (typeOfIndex(size) == 1)
			{
				int num = 0;
				numOfaddr(size, num);
				int block[128] = { 0 };
				readFromBlock(block, iNodeA[dir->directoryNode[i].iNodeId].iNode.address[10], 0, sizeof(int), num + 1);
				for (int j = 0; j < num + 1; j++)
					freeTheBlock(block[j]);
				freeTheBlock(iNodeA[dir->directoryNode[i].iNodeId].iNode.address[10]);
			}
			else if (typeOfIndex(size) == 2)
			{
				int num1 = 0;
				int num2 = 0;
				int block1[128] = { 0 };
				int block2[128] = { 0 };
				numOfaddrI3(size, num1, num2);
				readFromBlock(block1, iNodeA[dir->directoryNode[i].iNodeId].iNode.address[11], 0, sizeof(int), num1 + 1);
				for (int j = 0; j < num1 + 1; j++)
				{
					readFromBlock(block2, block1[j], 0, sizeof(int), num2 + 1);
					for (int k = 0; k < num2 + 1; k++)
						freeTheBlock(block2[k]);
					freeTheBlock(block1[j]);
				}
			}
			freeTheINode(dir->directoryNode[i].iNodeId);
			current->iNode.sizeOfFile -= sizeof(DirectoryNode);
			dir->numberOfDirectoryNode--;
			for (int j = i; j < dir->numberOfDirectoryNode; j++)
			{
				dir->directoryNode[j] = dir->directoryNode[j + 1];
				current->iNode.address[j] = current->iNode.address[j + 1];
			}
			writeIntoBlock(dir, current->iNode.address[0], 0, sizeof(Directory), 1);
			break;
		}
	}
	return SUCCESS;
}


int vi(const char *filename)
{
	char *buf = new char[655360];
	int size = 0;
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coordinate = { 0,0 };
	char input;
	system("cls");
	int statusCode = cat(filename);
	if (statusCode != SUCCESS)
		return statusCode;
	SetConsoleCursorPosition(console, coordinate);
	cd_file(filename);
	while (true)
	{
		input = NULL;
		input = _getch();
		if (input == '\x1a')
			break;
		if (input < 0)
		{
			input = _getch();
			switch (int(input))
			{
			case 72:
				if (coordinate.Y > 0)
					coordinate.Y--;
				break;
			case 80:
				coordinate.Y++;
				break;
			case 75:
				if (coordinate.X > 0)
					coordinate.X--;
				break;
			case 77:
				if (coordinate.X < 80)
					coordinate.X++;
				break;
			}
		}
		else
		{

			if (input == 8)
			{
				if (coordinate.X > 0)
					coordinate.X -= 1;
				SetConsoleCursorPosition(console, coordinate);
				cout << " ";
			}
			else if (input == 13)
			{
				coordinate.X = 0;
				coordinate.Y++;
				cout << endl;
				buf[size] = '\n';
				size++;
			}
			else if (input >= 32 && input <= 127)
			{
				cout << input;
				coordinate.X++;
				buf[size] = input;
				size++;
			}
		}
		SetConsoleCursorPosition(console, coordinate);
	}

	saveFile(buf, size, filename);
	delete[] buf;
	buf = nullptr;
	cd_return_file();
	system("cls");
	return SUCCESS;
}


int saveFile(char* buf, int size, const char *filename)
{
	int num1 = 0;
	int num2 = 0;
	int surplus = 0;
	int freeBlock = 0;
	if (typeOfIndex(size) == 0)
	{
		numOfaddr(size, num1, surplus);
		for (int i = 0; i < num1; i++)
		{
			allocateFreeBlock(freeBlock);
			current->iNode.address[i] = freeBlock;
			writeIntoBlock(&buf[i * 512], freeBlock, 0, 512, 1);
		}
		allocateFreeBlock(freeBlock);
		current->iNode.address[num1] = freeBlock;
		writeIntoBlock(&buf[num1 * 512], freeBlock, 0, surplus, 1);
		current->iNode.sizeOfFile += size;
	}
	else if (typeOfIndex(size) == 1)
	{
		int block[128] = { 0 };
		numOfaddr(size, num1, surplus);
		for (int i = 0; i < num1; i++)
		{
			allocateFreeBlock(freeBlock);
			block[i] = freeBlock;
			writeIntoBlock(&buf[i * 512], freeBlock, 0, 512, 1);
		}
		allocateFreeBlock(freeBlock);
		block[num1] = freeBlock;
		writeIntoBlock(&buf[num1 * 512], freeBlock, 0, surplus, 1);
		allocateFreeBlock(freeBlock);
		current->iNode.address[10] = freeBlock;
		writeIntoBlock(block, freeBlock, 0, sizeof(int), num1 + 1);
		current->iNode.sizeOfFile += size;
	}
	else if (typeOfIndex(size) == 2)
	{
		numOfaddrI3(size, num1, num2, surplus);
		int block1[128] = { 0 };
		int block2[128] = { 0 };
		for (int i = 0; i < num1; i++)
		{
			for (int j = 0; j < 128; j++)
			{
				allocateFreeBlock(freeBlock);
				block2[j] = freeBlock;
				writeIntoBlock(&buf[i * 128 * 512 + j * 512], freeBlock, 0, 512, 1);
			}
			allocateFreeBlock(freeBlock);
			block1[i] = freeBlock;
			writeIntoBlock(block2, freeBlock, 0, sizeof(int), 128);
			memset(block2, 0, sizeof(block2));
		}
		for (int i = 0; i < num2; i++)
		{
			allocateFreeBlock(freeBlock);
			block2[i] = freeBlock;
			writeIntoBlock(&buf[num1 * 128 * 512 + i * 512], freeBlock, 0, 512, 1);
		}
		allocateFreeBlock(freeBlock);
		block2[num2] = freeBlock;
		writeIntoBlock(&buf[num1 * 128 * 512 + num2 * 512], freeBlock, 0, surplus, 1);
		allocateFreeBlock(freeBlock);
		block1[num1] = freeBlock;
		writeIntoBlock(block2, freeBlock, 0, sizeof(int), num2 + 1);
		allocateFreeBlock(freeBlock);
		current->iNode.address[11] = freeBlock;
		writeIntoBlock(block1, freeBlock, 0, sizeof(int), num1 + 1);
		current->iNode.sizeOfFile += size;
	}
	return SUCCESS;
}


int cp(const char *filename, const char *path)
{
	int tempINode = current->iNodeId;
	judgePathRule(path);
	judgePath(path);
	int fileINodeId = 0;
	for (int i = 2; i < currentDir->numberOfDirectoryNode; i++)
	{
		if (!strcmp(currentDir->directoryNode[i].directoryName, filename) && (iNodeA[currentDir->directoryNode[i].iNodeId].iNode.modeOfFile / 1000 != 1))
			fileINodeId = currentDir->directoryNode[i].iNodeId;
	}
	cd_absolute(path);
	touch(filename);
	for (int i = 2; i < currentDir->numberOfDirectoryNode; i++)
	{
		if (!strcmp(currentDir->directoryNode[i].directoryName, filename))
			iNodeA[currentDir->directoryNode[i].iNodeId].iNode.modeOfFile = iNodeA[fileINodeId].iNode.modeOfFile;
	}
	int size = iNodeA[fileINodeId].iNode.sizeOfFile;
	int index = typeOfIndex(size);
	int num1 = 0;
	int num2 = 0;
	int surplus = 0;
	int freeBlock = 0;
	char buf[512] = { 0 };
	cd_file(filename);
	if (index == 0)
	{
		numOfaddr(size, num1, surplus);

		for (int i = 0; i < num1; i++)
		{
			readFromBlock(buf, iNodeA[fileINodeId].iNode.address[i], 0, 512, 1);

			allocateFreeBlock(freeBlock);
			current->iNode.address[i] = freeBlock;
			writeIntoBlock(&buf, freeBlock, 0, 512, 1);

			memset(buf, '\0', sizeof(buf));
		}
		readFromBlock(buf, iNodeA[fileINodeId].iNode.address[num1], 0, surplus, 1);
		allocateFreeBlock(freeBlock);
		current->iNode.address[num1] = freeBlock;
		writeIntoBlock(&buf, freeBlock, 0, surplus, 1);
		current->iNode.sizeOfFile = size;
	}
	else if (index == 1)
	{
		int block_r[128] = { 0 };
		int block_w[128] = { 0 };
		numOfaddr(size, num1, surplus);
		readFromBlock(block_r, iNodeA[fileINodeId].iNode.address[10], 0, sizeof(int), num1 + 1);
		for (int i = 0; i < num1; i++)
		{
			readFromBlock(buf, block_r[i], 0, 512, 1);
			allocateFreeBlock(freeBlock);
			block_w[i] = freeBlock;
			writeIntoBlock(&buf, freeBlock, 0, 512, 1);
			memset(buf, '\0', sizeof(buf));
		}
		readFromBlock(buf, block_r[num1], 0, surplus, 1);
		allocateFreeBlock(freeBlock);
		block_w[num1] = freeBlock;
		writeIntoBlock(&buf, freeBlock, 0, surplus, 1);
		allocateFreeBlock(freeBlock);
		current->iNode.address[10] = freeBlock;
		writeIntoBlock(block_w, freeBlock, 0, sizeof(int), num1 + 1);
		current->iNode.sizeOfFile = size;
	}
	else if (index == 2)
	{
		numOfaddrI3(size, num1, num2, surplus);
		int block_r1[128] = { 0 };
		int block_r2[128] = { 0 };
		int block_w1[128] = { 0 };
		int block_w2[128] = { 0 };
		readFromBlock(block_r1, iNodeA[fileINodeId].iNode.address[11], 0, sizeof(int), num1 + 1);
		for (int i = 0; i < num1; i++)
		{
			readFromBlock(block_r2, block_r1[i], 0, sizeof(int), 128);
			for (int j = 0; j < 128; j++)
			{
				readFromBlock(buf, block_r2[j], 0, 512, 1);
				allocateFreeBlock(freeBlock);
				block_w2[j] = freeBlock;
				writeIntoBlock(&buf, freeBlock, 0, 512, 1);
				memset(buf, '\0', sizeof(buf));
			}
			allocateFreeBlock(freeBlock);
			block_w1[i] = freeBlock;
			writeIntoBlock(block_w2, freeBlock, 0, sizeof(int), 128);
			memset(block_w2, 0, sizeof(block_w2));
		}
		readFromBlock(block_r2, block_r1[num1], 0, sizeof(int), num2 + 1);
		for (int i = 0; i < num2; i++)
		{
			readFromBlock(buf, block_r2[i], 0, 512, 1);
			allocateFreeBlock(freeBlock);
			block_w2[i] = freeBlock;
			writeIntoBlock(&buf, freeBlock, 0, 512, 1);
			memset(buf, '\0', sizeof(buf));
		}
		readFromBlock(buf, block_r2[num2], 0, surplus, 1);
		allocateFreeBlock(freeBlock);
		block_w2[num2] = freeBlock;
		writeIntoBlock(&buf, freeBlock, 0, surplus, 1);
		allocateFreeBlock(freeBlock);
		block_w1[num1] = freeBlock;
		writeIntoBlock(block_w2, freeBlock, 0, sizeof(int), num2 + 1);
		allocateFreeBlock(freeBlock);
		current->iNode.address[11] = freeBlock;
		writeIntoBlock(block_w1, freeBlock, 0, sizeof(int), num1 + 1);
		current->iNode.sizeOfFile = size;
	}
	current = &iNodeA[tempINode];
	return SUCCESS;
}


int ln(const char *filename, const char *path)
{
	INodeList tempINode = *current;
	judgePathRule(path);
	judgePath(path);
	int fileINodeId = 0;
	for (int i = 2; i < currentDir->numberOfDirectoryNode; i++)
	{
		if (!strcmp(currentDir->directoryNode[i].directoryName, filename))
			fileINodeId = currentDir->directoryNode[i].iNodeId;
	}
	cd_absolute(path);
	if (current->iNode.modeOfFile / 1000 != 1)
		return ERROR_FILE_MODE_WRONG;
	int count = currentDir->numberOfDirectoryNode;
	if (count > SUM_OF_DIRECTORY_IN_ONE_DIR)
		return ERROR_MORE_NUMBER_LIMIT_IN_DIRECTORY;
	for (int i = 0; i < count; i++)
		if (strcmp(currentDir->directoryNode[i].directoryName, filename) == 0)
			return ERROR_DIR_NAME_EXIST;
	current->iNode.sizeOfFile += sizeof(DirectoryNode);
	strcpy_s(currentDir->directoryNode[count].directoryName, filename);
	currentDir->directoryNode[count].iNodeId = fileINodeId;
	currentDir->numberOfDirectoryNode++;
	writeIntoBlock(currentDir, current->iNode.address[0], 0, sizeof(Directory), 1);
	return SUCCESS;
}


int help()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	cout << "----------------------------------------------------------" << endl;
	cout << "HELP: # COMMAND NAME - PARAMETER : EXPLANATION" << endl;
	cout << "----------------------------------------------------------" << endl;
	cout << "FILE COMMAND :" << endl;
	cout << "# touch - filename : create a new file in the current directory. " << endl;
	cout << "# vi - filename : edit a file which is in the current directory. " << endl;
	cout << "# rm - filename : remove a file which is in the current directory. " << endl;
	cout << "# cat - filename : read a file which is in the current directory. " << endl;
	cout << "# cp - filename - path : copy a file which is in the current directory to other path. " << endl;
	cout << "----------------------------------------------------------" << endl;
	cout << "DIRECTORY COMMAND :" << endl;
	cout << "# mkdir - directory name : create a new directory in the current directory. " << endl;
	cout << "# rmdir - directory name : remove a directory which is in the current directory. " << endl;
	cout << "# cd - absolute path / directory name : enter into a directory if it is existen. " << endl;
	cout << "# cd.. : return its parent directory. " << endl;
	cout << "----------------------------------------------------------" << endl;
	cout << "COMMON COMMAND :" << endl;
	cout << "# mv - old name - new name : change the name of file or directory which is in the current directory. " << endl;
	cout << "# ln - filename - path : create a link at the specified path. " << endl;
	cout << "----------------------------------------------------------" << endl;
	cout << "USER AND AUTHORITY COMMAND :" << endl;
	cout << "# adduser : create a new user. " << endl;
	cout << "# passwd : change the password of the current user. " << endl;
	cout << "# logout : logout and login by other account. " << endl;
	cout << "# chmod - filename / directory name - new authority : change the authority of a file or directory which is in the current directory. " << endl;
	cout << "# chgrp - filename / directory name - new group : change the group of a file or directory which is in the current directory. " << endl;
	cout << "----------------------------------------------------------" << endl;
	cout << "OTHER COMMAND :" << endl;
	cout << "# ls : print a list about the infomation of files and directories which are in the current directory. " << endl;
	cout << "# pwd : show the current directory. " << endl;
	cout << "# version : show the version of the system. " << endl;
	cout << "# format : format the whole system. " << endl;
	cout << "# shutdown : exit the system. " << endl;
	cout << "# clear : clear the console. " << endl;
	cout << "# super : show the infomation for super block. " << endl;
	cout << "# help : explain how to use the system. " << endl;
	cout << "----------------------------------------------------------" << endl;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	return SUCCESS;
}


int con()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
	cout << "[ " << currentUser.userName << "@Cymba_local ";
	pwd();
	cout << " ] : " << (strcmp(currentUser.userName, "root") == 0 ? "# " : "$ ");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	return SUCCESS;
}


int info()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED);
	cout << "Cymba Unix [version 1.0.2019.1.17]" << endl;
	cout << "ZheJiang University of Technology - Software Engineering" << endl;
	cout << "Design By Cymba For Operating System Curriculum Design" << endl;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	cout << endl;
	return SUCCESS;
}


int super()
{
	cout <<"free block:"<< superBlock->numberOfFreeBlock << endl;
	cout <<"point of free block"<< superBlock->pointerOfFreeBlock << endl;
	cout << "stack of free block" << endl;
	for (int i = 0; i < 60 ; i++)
		cout << superBlock->stackOfFreeBlock[i] << " ";
	cout << endl;
	cout <<"free inode"<< superBlock->numberOfFreeINode << endl;
	cout <<"point of free inode"<< superBlock->pointerOfFreeINode << endl;
	cout << "stack of free inode" << endl;
	for (int i = 0; i < 60; i++)
		cout << superBlock->stackOfFreeINode[i] << " ";
	cout << endl;
	return SUCCESS;
}


int getOrder()
{
	int i = 0;
	char input[128] = { 0 };
	char ch;
	char *buf;
	int statusCode = 0;
	con();
	while ((ch = getchar()) != '\n')
	{
		input[i] = ch;
		i++;
	}
	char *command = strtok_s(input, " ", &buf);
	if (i == 0)
		return ERROR_COMMAND_NONEXISTENT;
	if (!strcmp(command, "ls"))
	{
		statusCode = ls();
	}
	else if (!strcmp(command, "mkdir"))
	{
		char *parameter = strtok_s(NULL, " ", &buf);
		statusCode = mkdir(parameter);
	}
	else if (!strcmp(command, "rmdir"))
	{
		char *parameter = strtok_s(NULL, " ", &buf);
		statusCode = rmdir(parameter);
	}
	else if (!strcmp(command, "pwd"))
	{
		statusCode = pwd();
		cout << endl;
	}
	else if (!strcmp(command, "cat"))
	{
		char *parameter = strtok_s(NULL, " ", &buf);
		statusCode = cat(parameter);
	}
	else if (!strcmp(command, "touch"))
	{
		char *parameter = strtok_s(NULL, " ", &buf);
		statusCode = touch(parameter);
	}
	else if (!strcmp(command, "rm"))
	{
		char *parameter = strtok_s(NULL, " ", &buf);
		statusCode = rm(parameter);
	}
	else if (!strcmp(command, "vi"))
	{
		char *parameter = strtok_s(NULL, " ", &buf);
		statusCode = vi(parameter);
	}
	else if (!strcmp(command, "version"))
	{
		statusCode = info();
	}
	else if (!strcmp(command, "cd"))
	{
		char *parameter = strtok_s(NULL, " ", &buf);
		if ((statusCode = judgePathRule(parameter)) == SUCCESS)
		{
			if ((statusCode = judgePath(parameter)) == SUCCESS)
				statusCode = cd_absolute(parameter);
		}
		else
			statusCode = cd(parameter);
	}
	else if (!strcmp(command, "adduser"))
	{
		statusCode = createNewUser();
	}
	else if (!strcmp(command, "passwd"))
	{
		statusCode = passwd();
	}
	else if (!strcmp(command, "mv"))
	{
		char *parameter1 = strtok_s(NULL, " ", &buf);
		char *parameter2 = strtok_s(NULL, " ", &buf);
		statusCode = mv(parameter1, parameter2);
	}
	else if (!strcmp(command, "rm"))
	{
		char *parameter = strtok_s(NULL, " ", &buf);
		statusCode = rm(parameter);
	}
	else if (!strcmp(command, "cp"))
	{
		char *parameter1 = strtok_s(NULL, " ", &buf);
		char *parameter2 = strtok_s(NULL, " ", &buf);
		if ((statusCode = judgePathRule(parameter2)) == SUCCESS)
		{
			if ((statusCode = judgePath(parameter2)) == SUCCESS)
				statusCode = cp(parameter1, parameter2);
		}
	}
	else if (!strcmp(command, "ln"))
	{
		char *parameter1 = strtok_s(NULL, " ", &buf);
		char *parameter2 = strtok_s(NULL, " ", &buf);
		if ((statusCode = judgePathRule(parameter2)) == SUCCESS)
		{
			if ((statusCode = judgePath(parameter2)) == SUCCESS)
				statusCode = ln(parameter1, parameter2);
		}
	}
	else if (!strcmp(command, "chmod"))
	{
		int mode = 0;
		char *parameter1 = strtok_s(NULL, " ", &buf);
		char *parameter2 = strtok_s(NULL, " ", &buf);
		mode = (*parameter2 - '0') * 100 + (*(parameter2 + 1) - '0') * 10 + (*(parameter2 + 2) - '0');
		statusCode = chmod(parameter1, mode);
	}
	else if (!strcmp(command, "chgrp"))
	{
		char *parameter1 = strtok_s(NULL, " ", &buf);
		char *parameter2 = strtok_s(NULL, " ", &buf);
		statusCode = chgrp(parameter1, parameter2);
	}
	else if (!strcmp(command, "help"))
	{
		statusCode = help();
	}
	else if (!strcmp(command, "logout"))
	{
		system("cls");
		fclose(temporalFile);
		statusCode = turnOn(transformConstCharSToCharS("sb.dat"), false);
	}
	else if (!strcmp(command, "format"))
	{
		system("cls");
		fclose(temporalFile);
		statusCode = turnOn(transformConstCharSToCharS("sb.dat"), true);
	}
	else if (!strcmp(command, "shutdown"))
	{
		statusCode = SHUTDOWN;
	}
	else if (!strcmp(command, "cd.."))
	{
		statusCode = cd_return();
	}
	else if (!strcmp(command, "version"))
	{
		statusCode = info();
	}
	else if (!strcmp(command, "clear"))
	{
		system("cls");
	}
	else if (!strcmp(command, "super"))
	{
	super();
	}
	else
		statusCode = ERROR_COMMAND_NONEXISTENT;
	return statusCode;
}