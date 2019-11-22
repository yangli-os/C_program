#include "stdafx.h"
#include<ios.stream>
using namespace std；

int main()
{
	for (int i = 1; i <= 100; i++)
	{
		x = i;
		if (++x % 2 == 0)
		{
			cout << x << ",";
			if (++x % 3 == 0)
			{
				cout << x << ",";
				if (++x % 7 == 0)
					cout << x << ",";
			}
		}
	}
	return 0;
}