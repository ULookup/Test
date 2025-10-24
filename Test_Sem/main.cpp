#include <iostream>
#include <string>
#include <unistd.h>
#include "Sem.hpp"
#include "RingQueue.hpp"

using namespace std;

icepop::ringQueue<int> queue(10);
int gdata = 0;

void* proutine(void* args)
{
    string name = static_cast<const char*> (args);
    while(true){
        queue.Enqueue(gdata);
        std::cout << name << "生产了一个数据: " << gdata << std::endl;
        gdata++;
    }
}

void* croutine(void* args)
{
    string name = static_cast<const char*> (args);
    while(true){
        int data = 0;
        queue.Pop(&data);
        std::cout << name << "消费了一个数据: " << data << std::endl;
    }
}

int main()
{
    pthread_t Ctid[2],Ptid[3];


    pthread_create(Ptid, nullptr, proutine, (void*)"product-1");
    pthread_create(Ptid + 1, nullptr, proutine, (void*)"product-2");
    pthread_create(Ptid + 2, nullptr, proutine, (void*)"product-3");
    pthread_create(Ctid, nullptr, croutine, (void*)"consumer-1");
    pthread_create(Ctid + 1, nullptr, croutine, (void*)"consumer-2");

    cout << "创建线程成功！" << endl;

    pthread_join(Ctid[0],nullptr);
    pthread_join(Ctid[1],nullptr);
    pthread_join(Ptid[0],nullptr);
    pthread_join(Ptid[1],nullptr);
    pthread_join(Ptid[2],nullptr);

    cout << "等待线程成功" << endl;

    return 0;
}