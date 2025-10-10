#include"tool.hpp"
#include"task.hpp"
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<iostream>

int main(int argc, char *argv[], char *env[])
{
  pid_t id = fork();
  if(id < 0){
    cout << "fork error!" << endl;
  }
  else if(id == 0){
    cout << "successfully forked!" << endl;

    sleep(5);
    extern char **environ;
    const char *myargv[] = {"-a" , "-c" , "-m", NULL};
    execvpe("childProc",(char* const*)myargv, environ);
    //execute child process
    
    exit(0);
  }

  int status = 0;
  pid_t retid = 0;
  while(true)
  {
    sleep(1);
    retid = waitpid(id,&status,WNOHANG);
    cout << "waiting for child process ending..." << endl;

    if(retid == 0){
      cout << "child process is still running!" << endl;
        Tool tool;
        tool.push_back(Download);
        tool.push_back(Upload);  
        tool.execute();
    }
    else if(retid == id){
      cout << "child process end." << endl;
      break;
    }
    else{
      cout << "wait fail!" << endl;
      break;
    }
  }

  return 0;
}
