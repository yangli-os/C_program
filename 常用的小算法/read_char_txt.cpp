#include <iostream>
#include<vector>
#include<fstream>
#include<typeinfo>
#include<string.h>

using namespace std;

int main()
{
 char buf[1024];                //临时保存读取出来的文件内容
 char filePath[1024];
 char fileName[1024];
 int len_str;
 string message;
 ifstream infile;
 infile.open("C://Users//dobi//Desktop//name_list.txt");
 if (infile.is_open())          //文件打开成功,说明曾经写入过东西
 {
  while (infile.good() && !infile.eof())
  {
   memset(buf, 0, 1024);
   infile.getline(buf, 1024);
   len_str = strlen(buf);
   strncpy(filePath, buf, len_str-25);
   strncpy(fileName, buf, len_str);
  }
  infile.close();
 }
}
