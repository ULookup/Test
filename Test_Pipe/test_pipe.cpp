#include<iostream>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<cstring>
#include<string>

using namespace std;

int main(){
    int pipefd[2];
    int n = pipe(pipefd);
    (void)n;

    pid_t id = fork();
    if(id == 0){
        close(pipefd[0]);
        int cnt = 10;
        while(cnt){
        string messages = "Hello, father, 我是子进程:";
        messages += to_string(getpid());
        messages += to_string(cnt);
        messages += "\n";
        write(pipefd[1], messages.c_str(), messages.size());
        cnt--;
        sleep(1);
        }
    }
    else if(id < 0){
        perror("fork failed!\n");
        exit(1);
    }
    else{
        close(pipefd[1]);
        char buffer[1024] = {0};
        while(true){
            sleep(2);
            ssize_t n = read(pipefd[0], buffer, sizeof(buffer)-1);
            if(n <= 0){
                perror("read failed!\n");
                break;
            }
            else{
                printf("client->child: %s\n", buffer);
            } 
        }
        close(pipefd[0]);
        pid_t rpid = waitpid(id, nullptr, 0);
        if(rpid == id){
            cout << "child process " << id << " has terminated!" << endl;
        }
        else{
            perror("waitpid failed!\n");
            exit(1);
        }
        return 0;
    }
}