#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include<opencv2/imgproc/imgproc.hpp> 
#include<vector>
#include<fstream>
#include<typeinfo>
#include"function.h"

using namespace cv;
using namespace std;
diagnosisFunction diaFun;

int read_txt();
float data_all[131072] = { 0 };            //定义一个256*256的数组，用于存放数据
float data_Magnetic[65536] = { 0 };        //定义一个256*256的数组，用于存放数据
float data_Current[65536] = { 0 };         //定义一个256*256的数组，用于存放数据
float data_Current2[256][256] = { 0 };
int read_txt()
{
	ifstream infile;//定义读取文件流，相对于程序来说是in
	infile.open("E://Program//Feature_Magnetic//Feature//Feature_test//Fig.txt");//打开文件
	for (int i = 0; i < 131072; i++)//定义行循环
	{
		infile >> data_all[i];//读取一个值（空格、制表符、换行隔开）就写入到矩阵中，行列不断循环进行
	}
	infile.close();//读取完成之后关闭文件
	return 0;
}

int main()
{
	int red = read_txt();
	for (int i = 0; i < 65536; i++)
	{
		data_Magnetic[i] = data_all[i];
	}
	for (int j = 65536; j < 131072; j++)
	{
		data_Current[j - 65536] = data_all[j];
	}
	diaFun.normalize_MaxMin_255(data_Current, 65536);
	for (int p = 0; p < 256; p++)
	{
		for (int q = 0; q < 256; q++)
		{
			data_Current2[q][p] = data_Current[p * 256 + q];
		}
	}
	Mat srcImage = Mat(Size(256, 256), CV_32F, data_Current2);
	Mat im_color;
	srcImage.convertTo(im_color, CV_8UC1, 255.0 / 255);             //映射从CV_32F转换到CV_8U 的0-255
	Mat grad_Image, grayImage;
	//applyColorMap(srcImage, grad_Image, COLORMAP_JET);
	//cvtColor(srcImage, grayImage,COLOR_BGR2GRAY);
	GaussianBlur(im_color, grad_Image, Size(5, 5), 3, 3);
	vector<Vec3f> circles;
	double dp = 2;
	double minDist = 10;
	double param1 = 10;
	double param2 = 100;
	int min_radius = 0;
	int max_radius = 1000;
	HoughCircles(im_color, circles, HOUGH_GRADIENT, dp, minDist, param1, param2, min_radius, max_radius);
	for (size_t i = 0; i < circles.size(); i++)
	{
		int radius = cvRound(circles[i][2]);
		cout << radius << endl;
	}

	//applyColorMap(srcImage, im_color, COLORMAP_JET);
	//string src = "E://Program//Feature_Magnetic//OpenCV_demo//07-1.bmp";
	//Mat srcImage = imread(src);
	//Mat grad_Image, grayImage;
	//Mat abs_grad_Image, dstImage;
	//grad_Image = Mat(srcImage.rows, srcImage.cols, CV_32F);

	//cvtColor(srcImage, grayImage, COLOR_BGR2RGB);
	///cvtColor(srcImage, grayImage, COLOR_BGR2RGB);
	//imshow("原图：", im_color);
	//waitKey(0);
	//求(x,y)方向梯度
	//Sobel(srcImage, grad_Image, CV_32F, 1, 0, 3, 1, 0, BORDER_DEFAULT);
	//convertScaleAbs(grad_Image, abs_grad_Image);
	//imshow("原图：", src);
	//imshow("Sobel函数效果图：", abs_grad_Image);
	//R波顶点，T波起始点，T波顶点的伪电流密度图的箭头，1维。
   // cout << maxAngle << endl;
}
