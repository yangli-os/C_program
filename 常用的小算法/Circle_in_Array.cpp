#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include<opencv2/imgproc/imgproc.hpp> 
#include<vector>
#include<fstream>
#include<typeinfo>

using namespace cv;
using namespace std;
int min_position(float* a, int len_a);
int max_position(float* a, int len_a);
void normalize_MaxMin(float* x, int len_x, int multiple);
/*********************这部分数据其实只需要一半也就是256*256即可************************************************/
int read_txt();
float data_all[131072] = { 0 };          //定义一个256*256*2的数组，用于存放数据
float data_M[65536] = { 0 };      //定义一个256*256的数组，用于存放数据
float data_C[65536] = { 0 };       //定义一个256*256的数组，用于存放数据
float data_C2[256][256] = { 0 };
int read_txt()
{
	ifstream infile;                                                                     //定义读取文件流，相对于程序来说是in
	infile.open("E:\\data.txt");         						     //打开文件
	for (int i = 0; i < 131072; i++)                                                     //定义行循环
	{
		infile >> data_all[i];                                                       //读取一个值（空格、制表符、换行隔开）就写入到矩阵中，行列不断循环进行
	}
	infile.close();                                                                      //读取完成之后关闭文件
	return 0;
}

int main()
{
	int red = read_txt();
	for (int i = 0; i < 65536; i++)
	{
		data_M[i] = data_all[i];
	}
	for (int j = 65536; j < 131072; j++)
	{
		data_C[j - 65536] = data_all[j];
	}
	//0~255的最大最小归一化
	normalize_MaxMin(data_C, 65536, 255);

	for (int p = 0; p < 256; p++)
	{
		for (int q = 0; q < 256; q++)
		{
			data_C2[q][p] = data_C[p * 256 + q];
		}
	}
	//把数据生成256*256的图
	Mat srcImage = Mat(Size(256, 256), CV_32F, data_C2);
	Mat im_color;
	srcImage.convertTo(im_color, CV_8UC1, 255.0 / 255);             //映射从CV_32F转换到CV_8U 的0-255
	Mat grad_Image;
	//GaussianBlur(im_color, grad_Image, Size(5, 5), 3, 3);            //高斯平滑滤波

	/********************************************************************检测圆形****************************************************************************************/
	vector<Vec3f> circles;
	double dp = 2;
	double minDist = 100;                                                      //两个圆心之间的最小距离
	double param1 = 10;                                                        //Canny边缘检测的较大阈值
	double param2 = 100;                                                       //累加器阈值
	int min_radius = 0;                                                        //圆形半径的最小值
	int max_radius = 1000;                                                     //圆形半径的最大值
	//识别圆形
	HoughCircles(im_color, circles, HOUGH_GRADIENT, dp, minDist, param1, param2, min_radius, max_radius);
	//在图像中标记出圆形
	for (size_t i = 0; i < circles.size(); i++)
	{
		//读取圆心
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		//读取半径
		int radius = cvRound(circles[i][2]);
		//绘制圆心
		circle(im_color,center,3,Scalar(0,255,0),-1,8,0);
		//绘制图
		circle(im_color, center, radius, Scalar(0, 0, 255), 3, 8, 0);
		//cout << radius << endl;
	}
	//显示结果
	imshow("Circle in picture", im_color);
	waitKey(0);
	return 0;
}

int max_position(float* a, int len_a)
{
    //计算最小值所在的位置
    int position = 0;
    float max_y = -65535;               //初始化一个最大值
    for (int i = 0; i < len_a; i++)
    {
        if (a[i] > max_y)
        {
            max_y = a[i];
            position = i;
        }
    }
    return position;
}

int min_position(float* a, int len_a)
{
    //计算最小值所在的位置
    int position = 0;
    float min_y = 65535;               //初始化一个最大值
    for (int i = 0; i < len_a; i++)
    {
        if (a[i] < min_y)
        {
            min_y = a[i];
            position = i;
        }
    }
    return position;
}

void normalize_MaxMin(float* x, int len_x, int multiple)
{
    //最大最小归一化到0-1；
    float max_x = x[max_position(x, len_x)];
    float min_x = x[min_position(x, len_x)];
    for (int i = 0; i < len_x; i++)
    {
        x[i] = ((float)(x[i] - min_x) / (float)(max_x - min_x))*multiple;
    }
}
