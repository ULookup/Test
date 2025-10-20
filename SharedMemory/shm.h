#ifndef __SHM_H__
#define __SHM_H__

#include<iostream>
#include<cstdio>
#include<unistd.h>
#include<string>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>

extern std::string global_pathname;
extern int global_proj_i;
extern int global_defaultSize;

struct data
{
    int num;
    char buffer[4092];
};

class SharedMemory
{
private:
    bool CreateHelper(int flags);

public:
    SharedMemory(int size = global_defaultSize);
    bool Create();
    bool Get();
    bool Attach();
    void SetZero();
    bool Detach();
    void AddChar(char ch);
    void PopChar(char *ch);
    bool RemoveShm();
    void PrintAttr();
    ~SharedMemory();

private:
    key_t _key;
    int _size;
    int _shmid;
    void *_start_addr;
    int *_num;
    char *_datastart;
    int _windex;
    int _rindex;
};

#endif