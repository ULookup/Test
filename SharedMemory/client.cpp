#include "shm.h"

int main()
{
    SharedMemory shm;
    shm.Get();
    shm.Attach();

    char c = 'A';
    for(;c <= 'Z';c++){
        shm.AddChar(c);
        sleep(1);
    }
    shm.Detach();
    return 0;
}