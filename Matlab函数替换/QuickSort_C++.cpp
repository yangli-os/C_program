#include<iostream>
#include<queue>
//需要借助queue脚本，但是这个脚本并没有搜到
//使用栈实现快速排序
using namespace std;
 
void QSort(float* array, float count) {
	queue<float> First;                    //队列
	queue<float> Last;
	First.push(0);
	Last.push(count - 1);
	while(!First.empty()) {
		int first = First.front();
		int last = Last.front();
		First.pop(); Last.pop();
		float ctrl = array[first];
		float curr = 0;
		int i = first, j = last;
		while(i < j) {
			if(curr == 0) {
				while(array[j] > ctrl && j != i) j--;
				array[i] = array[j];
				curr = 1;
			} else {
				while(array[i] <= ctrl && j != i) i++;
				array[j] = array[i];
				curr = 0;
			}
		}
		array[i] = ctrl;
		if(first < i - 1) {
			First.push(first);
			Last.push(i - 1);
		}
		if(last > i + 1) {
			First.push(i + 1);
			Last.push(last);
		}
	}
}

void PrintArray(float* a, int n)
{
	for (int i = 0; i < n; i++)
	{
		printf("%f ", a[i]);
	}
	printf("\n");
}
int main() {
	float test[10] = { 4.2,1.2,7.3,9.0,3.0,2.0,6.0,5.0,0.0,8.0 };
	QSort(test, 10);
	PrintArray(test,10);
	return 0;
}
