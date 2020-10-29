//一个基于C的栈入栈出快排算法
#include <string.h>
#include<math.h> 
#include<iostream>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

int Partation(int* arr, int low, int high);
int Partation_Float(float* arr, int low, int high);
void QuickSort(int* arr, int len);
void QuickSortFloat(float* arr, int len);
//一次划分过程

int Partation(int* arr, int low, int high)//返回值为low与high相等时的下标
{
    int tmp = arr[low];//将第一个数字作为基准
    while (low < high)
    {
        while ((low < high) && (arr[high] >= tmp))//从后往前找比基准小的数字往前移
        {
            high--;//没有找到比基准小的数字
        }
        if (low == high)
        {
            break;//直到low与high相等跳出
        }
        else
        {
            arr[low] = arr[high];//将后面小的数字移到前面
        }

        while ((low < high) && (arr[low] <= tmp))//从前往后找比基准大的数字
        {
            low++;//没有找到比基准大的数字
        }
        if (arr[low] > tmp)
        {
            arr[high] = arr[low];//将前面的数字往后移
        }
        else
        {
            break;
        }
    }
    arr[low] = tmp;//基准放入
    return low;//返回基准下标
}

int Partation_Float (float* arr, int low, int high)//返回值为low与high相等时的下标
{
    float tmp = arr[low];//将第一个数字作为基准
    while (low < high)
    {
        while ((low < high) && (arr[high] >= tmp))//从后往前找比基准小的数字往前移
        {
            high--;//没有找到比基准小的数字
        }
        if (low == high)
        {
            break;//直到low与high相等跳出
        }
        else
        {
            arr[low] = arr[high];//将后面小的数字移到前面
        }

        while ((low < high) && (arr[low] <= tmp))//从前往后找比基准大的数字
        {
            low++;//没有找到比基准大的数字
        }
        if (arr[low] > tmp)
        {
            arr[high] = arr[low];//将前面的数字往后移
        }
        else
        {
            break;
        }
    }
    arr[low] = tmp;//基准放入
    return low;//返回基准下标

}

//用栈和队列组数对实现快速排序

void QuickSort(int* arr, int len)//最坏情况下时间复杂度为O(n^2)
{

    int tmp = (int)ceil((log((double)len) / log((double)2)));//ceil向上取整,返回值为double类型
    //int *stack = (int *)malloc(sizeof(int)*2*(int)(log((double)len)/log((double)2)));//未向上取整
    int* stack = (int*)malloc(sizeof(int) * 2 * tmp*5);
    assert(stack != NULL);

    int top = 0;//栈顶指针，下标可取

    int start = 0;
    int end = len - 1;
    int par = Partation(arr, start, end);

    if (par > start + 1)//基准左边至少两个数据
    {
        stack[top++] = start;//入栈左边数据
        stack[top++] = par - 1;
    }

    if (par < end - 1)//基准右边至少两个数据
    {
        stack[top++] = par + 1;//入栈右边数据
        stack[top++] = end;
    }

    while (top > 0)//栈不空，处理栈中数据
    {
        //出栈数据
        end = stack[--top];//后进先出
        start = stack[--top];

        par = Partation(arr, start, end);//继续划分

        //如果两边还是大于两个数据，继续循环
        if (par > start + 1)//基准左边至少两个数据
        {
            stack[top++] = start;//入栈左边数据
            stack[top++] = par - 1;
        }

        if (par + 1 < end)//基准右边至少两个数据
        {
            stack[top++] = par + 1;//入栈右边数据
            stack[top++] = end;
        }
    }
    free(stack);
}

void QuickSortFloat(float* arr, int len)//最坏情况下时间复杂度为O(n^2)
{
    int tmp = (int)ceil((log((double)len) / log((double)2)));//ceil向上取整,返回值为double类型
    //int *stack = (int *)malloc(sizeof(int)*2*(int)(log((double)len)/log((double)2)));//未向上取整
    int* stack = (int*)malloc(sizeof(int) * 2 * tmp*5);
    assert(stack != NULL);

    int top = 0;//栈顶指针，下标可取

    float start = 0.0;
    int end = len - 1;
    int par = Partation_Float(arr, start, end);

    if (par > start + 1)//基准左边至少两个数据
    {
        stack[top++] = start;//入栈左边数据
        stack[top++] = par - 1;
    }

    if (par < end - 1)//基准右边至少两个数据
    {
        stack[top++] = par + 1;//入栈右边数据
        stack[top++] = end;
    }

    while (top > 0)//栈不空，处理栈中数据
    {
        //出栈数据
        end = stack[--top];//后进先出
        start = stack[--top];

        par = Partation_Float(arr, start, end);//继续划分

        //如果两边还是大于两个数据，继续循环
        if (par > start + 1)//基准左边至少两个数据
        {
            stack[top++] = start;//入栈左边数据
            stack[top++] = par - 1;
        }

        if (par + 1 < end)//基准右边至少两个数据
        {
            stack[top++] = par + 1;//入栈右边数据
            stack[top++] = end;
        }
    }
    free(stack);
}
