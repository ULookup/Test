#include<unistd.h>
#include<stdio.h>
#include<iostream>
#include<stdlib.h>

using namespace std;

int g_unval;
int g_val = 100;

int main(int argc,char *argv[],char *env[])
{
  const char *str = "Hello, Linux!";
  printf("code addr: %p\n", main);
  printf("init global addr: %p\n", &g_val);
  printf("uninit global addr: %p\n", &g_unval);

  static int test = 10;
  char *heap_mem = (char*)malloc(10);
  char *heap_mem1 = (char*)malloc(10);
  char *heap_mem2 = (char*)malloc(10);
  char *heap_mem3 = (char*)malloc(10);

  printf("heap addr: %p\n", heap_mem); //heap_mem(0), &heap_mem(1)
  printf("heap addr: %p\n", heap_mem1); //heap_mem(0), &heap_mem(1)
  printf("heap addr: %p\n", heap_mem2); //heap_mem(0), &heap_mem(1)
  printf("heap addr: %p\n", heap_mem3); //heap_mem(0), &heap_mem(1)
  printf("test static addr: %p\n", &test); //heap_mem(0), &heap_mem(1)
  printf("stack addr: %p\n", &heap_mem); //heap_mem(0), &heap_mem(1)
  printf("stack addr: %p\n", &heap_mem1); //heap_mem(0), &heap_mem(1)
  printf("stack addr: %p\n", &heap_mem2); //heap_mem(0), &heap_mem(1)
  printf("stack addr: %p\n", &heap_mem3); //heap_mem(0), &heap_mem(1)

  printf("read only string addr: %p\n", str);
  for(int i = 0 ;i < argc; i++)
  {
  printf("argv[%d]: %p\n", i, argv[i]);
  }
  for(int i = 0; env[i]; i++)
  {
  printf("env[%d]: %p\n", i, env[i]);
  }

  return 0;
}
