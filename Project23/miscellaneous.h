#pragma once

#include "globalVariable.h"
using namespace std;


char* transformConstCharSToCharS(const char* value);
unsigned long int getTimeStamp();
void hiddenPassword(char password[LENGTH_OF_OWNER_PASSWORD]);
void fileEmpty(const string fileName);
void init(const char* filename);
int typeOfIndex(int size);
int numOfaddr(int size, int &num, int &surplusSize);
int numOfaddrI3(int size, int &num1, int &num2, int &surplusSize);
int numOfaddr(int size, int &num);
int numOfaddrI3(int size, int &num1, int &num2);
int numOfSplit(const char *path);
int split(const char *path, int num, char *&name, int &length);
int judgePath(const char *path);
int judgePathRule(const char *path);
int printErrorInfo(int statusCode);
int printMode(int mode);