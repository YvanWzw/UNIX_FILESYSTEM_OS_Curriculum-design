#pragma once

#include <iostream>
#include <Windows.h>
#include "define.h"
#include "function.h"
#include "miscellaneous.h"
#include "order.h"
#include "struct.h"
#include "error.h"


extern INodeList iNodeA[SUM_OF_INODE];
extern INodeList *root;
extern INodeList *current;
extern SuperBlock *superBlock;
extern FILE *temporalFile;
extern User currentUser;
extern Directory *currentDir;