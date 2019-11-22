#include<iostream>

int main()
{
	int a = 10, b, c, d, i, j, k;
	b = c = d = 5;
	i = j = k = 0;
	for (; a > b; ++b)
		i++;
	while (a > ++c)
		j++;
	do {
		k++;
	} while (a > d++);
	std::cout << i << "," << j << "," << k;
	return 0;
}