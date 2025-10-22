#include <iostream>
#include <vector>
#include <pthread.h>
#include <unistd.h>

using namespace std;

void *threadRoutine(void *arg)
{
    cout << "新线程: " << pthread_self() << " 创建成功！" << endl;
    int cnt = 3;
    while(cnt){
        cout << "线程: " << pthread_self() << "执行任务中...(剩余时间: " << cnt << "s)" << endl;
        cnt--;
        sleep(1);
    }
    //pthread_exit((void*)1);
    return 0;
}

int main()
{
    vector<pthread_t> threadIds;
    for (int i = 0; i < 5; i++){
        pthread_t tid = 0;
        int n = pthread_create(&tid,nullptr,threadRoutine,nullptr);
        threadIds.push_back(tid);
        sleep(1);
    }

    sleep(5);
    for(auto& e : threadIds){
        pthread_join(e,nullptr);
        cout << "等待线程成功！" << endl;
        sleep(1); 
    }

    return 0;
}