#include "shm.h"

int main()
{
    SharedMemory shm;
    shm.Create();
    shm.Attach();
    shm.PrintAttr();
    sleep(3);

    while (true){
        char c;
        shm.PopChar(&c);
        printf("server get char: %c\n", c);
        sleep(1);
    }

    shm.Detach();
    shm.RemoveShm();
    return 0;
}