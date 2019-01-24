#pragma once


#include "globalVariable.h"
#include "struct.h"


int login();
int createNewUser();
int mkdir(const char *name);
int rmdir(const char *name);
int perOrderForDel(Directory dirRoot);
int perOrderForSize(Directory dirRoot, int &size);
int ls();
int cd(const char *path);
int cd_file(const char *path);
int cd_return_file();
int cd_return();
int cd_absolute(const char *path);
int cd_root();
int pwd();
int passwd();
int mv(const char *oldname, const char *newname);
int chmod(const char *name, unsigned short int mode);
int chgrp(const char *name, const char *group);
int cat(const char *name);
int touch(const char* name);
int rm(const char *name);
int rm(const char *name, Directory *dir);
int vi(const char *filename);
int saveFile(char* buf, int size, const char *filename);
int cp(const char *filename, const char *path);
int ln(const char *filename, const char *path);
int info();
int getOrder();
int con();
int help();
int super();