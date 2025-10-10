#pragma once
#include<iostream>
#include<unistd.h>
using namespace std;
void Download()
{
  cout << "Downloading..." << endl;
  for(int i = 10;i>=0;i--){
    cout << i << "s left" << endl;
  }
  cout << "Download complete" << endl;
}

void Upload()
{
  cout << "Uploading..." << endl;
  for(int i = 10;i>=0;i--){
    cout << i << "s left" << endl;
  }
  cout << "Upload complete" << endl;
}
