#include "shm.h"

std::string global_pathname = ".";
int global_proj_id = 0x66;
int global_defaultSize = 4096;

bool SharedMemory::CreateHelper(int flags)
{
    _key = ftok(global_pathname.c_str(), global_proj_id);
    if(_key < 0){
        perror("ftok");
        return false;
    }
    printf("形成键值成功: 0x%x\n", _key);

    _shmid = shmget(_key, _size, flags);
    if(_shmid < 0){
        perror("shmget");
        return false;
    }
    printf("shmid: %d\n", _shmid);
    return true;
}

SharedMemory::SharedMemory(int size)
    :_size(size), _key(0), _shmid(-1),
     _start_addr(nullptr), _windex(0), _rindex(0),
     _datastart(nullptr), _num(nullptr)
     {}

bool SharedMemory::Create() { return CreateHelper(IPC_CREAT | IPC_EXCL | 0666); }

bool SharedMemory::Get() { return CreateHelper(IPC_CREAT); }

bool SharedMemory::Attach()
{
    _start_addr = shmat(_shmid, nullptr, 0);
    if((long long)_start_addr == -1){
        perror("shmat");
        return false;
    }
    std::cout << "将指定的共享内存挂接到自己进程的虚拟地址空间" << std::endl;

    printf("_start_addr : %p\n",_start_addr);
    _num = (int*)_start_addr;
    _datastart = (char*)_start_addr + sizeof(int);
    return true;
}

void SharedMemory::SetZero() { *_num = 0; }

bool SharedMemory::Detach()
{
    int n = shmdt(_start_addr);
    if(n < 0){
        perror("shmdt");
        return false;
    }
    std::cout << "将指定的共享内存从进程的地址空间移除" << std::endl;
    return true;
}

void SharedMemory::AddChar(char ch)
{
    if(*_num == _size)
        return;
    ((char*)_datastart)[_windex++] = ch;
    ((char*)_datastart)[_windex] = '\0';

    std::cout << "debug: " << _windex << ", " << ch << std::endl;
    _windex %= _size;
    (*_num)++;
}

void SharedMemory::PopChar(char *ch)
{
    if(*_num == 0)
        return;
    *ch = ((char*)_datastart)[_rindex]++;
    _rindex %= _size;
    (*_num)--;
    printf("%s\n", _datastart);
}

bool SharedMemory::RemoveShm()
{
    int n = shmctl(_shmid, IPC_RMID, nullptr);
    if(n < 0){
        perror("shmcl");
        return false;
    }
    std::cout << "删除shm成功" << std::endl;
    return true;
}

void SharedMemory::PrintAttr()
{
    struct shmid_ds ds;
    int n = shmctl(_shmid, IPC_STAT, &ds);
    if(n < 0){
        perror("shmctl");
        return;
    }
    
    printf("key: 0x%x\n", ds.shm_perm.__key);
    printf("size: %ld\n", ds.shm_segsz);
    printf("atime: %lu\n", ds.shm_atime);
    printf("nattach: %ld\n", ds.shm_nattch);
}

SharedMemory::~SharedMemory() {}

