#include <iostream>
#include<vector>
#include<fstream>
#include<typeinfo>

using namespace std;

int main()
{
	char buf[1024];                //临时保存读取出来的文件内容
	string message;
	ifstream infile;
	infile.open("../data.txt");
	if (infile.is_open())          //文件打开成功,说明曾经写入过东西
	{
		while (infile.good() && !infile.eof())
		{
			memset(buf, 0, 1024);
			infile.getline(buf, 1204);
			message = buf;
                   //这里可能对message做一些操作
				cout << message << endl;
		}
		infile.close();
	}
}
