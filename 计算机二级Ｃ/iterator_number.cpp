// Third_new.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include<cmath>
#include <iomanip>
using namespace std;

int main()
{
	const double eps = 1e-7;            //计算精度
	float x0 = 0, x1 = 0, a;         
	cin >> a;							//输入，a为double
	x0 = a / 2;                         //计算a的初值，不赋初值下一步将计算a/0
	x1 = (x0 + a / x0) / 2;
	do
	{
		x0 = x1;
		x1 = (x0 + a / x0) / 2;
	}while(fabs(x0 - x1) >= eps);			//计算前一次和后一次的差值
	cout << setprecision(7) << x1;          //输出，保留7位小数
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
