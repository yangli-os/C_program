//高阶拟合函数
#define rank_ 7
//x,x长度，y,rank:拟合阶数，3阶拟合
void ployNomialFit3(double* data_x, int len_data_x, double* data_y, int rank, double* atemp, double* b, double a[][rank_ + 1])
{
	for (int i = 0; i < len_data_x; i++) {  //
		atemp[1] += data_x[i];
		atemp[2] += pow(data_x[i], 2);
		atemp[3] += pow(data_x[i], 3);
		atemp[4] += pow(data_x[i], 4);
		atemp[5] += pow(data_x[i], 5);
		atemp[6] += pow(data_x[i], 6);
		b[0] += data_y[i];
		b[1] += data_x[i] * data_y[i];
		b[2] += pow(data_x[i], 2) * data_y[i];
		b[3] += pow(data_x[i], 3) * data_y[i];
	}

	atemp[0] = len_data_x;

	for (int i = 0; i < rank + 1; i++) {  //构建线性方程组系数矩阵，b[]不变
		int k = i;
		for (int j = 0; j < rank + 1; j++)  a[i][j] = atemp[k++];
	}


	//以下为高斯列主元消去法解线性方程组
	for (int k = 0; k < rank + 1 - 1; k++) {  //n - 1列
		int column = k;
		double mainelement = a[k][k];

		for (int i = k; i < rank + 1; i++)  //找主元素
			if (fabs(a[i][k]) > mainelement) {
				mainelement = fabs(a[i][k]);
				column = i;
			}
		for (int j = k; j < rank + 1; j++) {  //交换两行
			double atemp = a[k][j];
			a[k][j] = a[column][j];
			a[column][j] = atemp;
		}
		double btemp = b[k];
		b[k] = b[column];
		b[column] = btemp;

		for (int i = k + 1; i < rank + 1; i++) {  //消元过程
			double Mik = a[i][k] / a[k][k];
			for (int j = k; j < rank + 1; j++)  a[i][j] -= Mik * a[k][j];
			b[i] -= Mik * b[k];
		}
	}

	b[rank + 1 - 1] /= a[rank + 1 - 1][rank + 1 - 1];  //回代过程
	for (int i = rank + 1 - 2; i >= 0; i--) {
		double sum = 0;
		for (int j = i + 1; j < rank + 1; j++)  sum += a[i][j] * b[j];
		b[i] = (b[i] - sum) / a[i][i];
	}
	//高斯列主元消去法结束

//       printf("P(x) = %f%+fx%+fx^2%+fx^3\n\n", b[0], b[1], b[2], b[3]);
	   //拟合后的曲线数据重新赋值给原数组
	for (int s = 0; s < len_data_x; s++)
	{
		data_y[s] = b[0] + data_x[s] * b[1] + pow(data_x[s], 2) * b[2] + pow(data_x[s], 3) * b[3];
	}
}

//x,x长度，y,rank:拟合阶数，7阶拟合
void ployNomialFit7(double* data_x, int len_data_x, double* data_y, int rank, double* atemp, double* b, double a[][rank_ + 1])
{
	for (int i = 0; i < len_data_x; i++) {  //
		atemp[1] += data_x[i];
		atemp[2] += pow(data_x[i], 2);
		atemp[3] += pow(data_x[i], 3);
		atemp[4] += pow(data_x[i], 4);
		atemp[5] += pow(data_x[i], 5);
		atemp[6] += pow(data_x[i], 6);
		atemp[7] += pow(data_x[i], 7);
		atemp[8] += pow(data_x[i], 8);
		atemp[9] += pow(data_x[i], 9);
		b[0] += data_y[i];
		b[1] += data_x[i] * data_y[i];
		b[2] += pow(data_x[i], 2) * data_y[i];
		b[3] += pow(data_x[i], 3) * data_y[i];
		b[4] += pow(data_x[i], 4) * data_y[i];
		b[5] += pow(data_x[i], 5) * data_y[i];
		b[6] += pow(data_x[i], 6) * data_y[i];
		b[7] += pow(data_x[i], 7) * data_y[i];
	}

	atemp[0] = len_data_x;

	for (int i = 0; i < rank + 1; i++) {  //构建线性方程组系数矩阵，b[]不变
		int k = i;
		for (int j = 0; j < rank + 1; j++)  a[i][j] = atemp[k++];
	}


	//以下为高斯列主元消去法解线性方程组
	for (int k = 0; k < rank + 1 - 1; k++) {  //n - 1列
		int column = k;
		double mainelement = a[k][k];

		for (int i = k; i < rank + 1; i++)  //找主元素
			if (fabs(a[i][k]) > mainelement) {
				mainelement = fabs(a[i][k]);
				column = i;
			}
		for (int j = k; j < rank + 1; j++) {  //交换两行
			double atemp = a[k][j];
			a[k][j] = a[column][j];
			a[column][j] = atemp;
		}
		double btemp = b[k];
		b[k] = b[column];
		b[column] = btemp;

		for (int i = k + 1; i < rank + 1; i++) {  //消元过程
			double Mik = a[i][k] / a[k][k];
			for (int j = k; j < rank + 1; j++)  a[i][j] -= Mik * a[k][j];
			b[i] -= Mik * b[k];
		}
	}

	b[rank + 1 - 1] /= a[rank + 1 - 1][rank + 1 - 1];  //回代过程
	for (int i = rank + 1 - 2; i >= 0; i--) {
		double sum = 0;
		for (int j = i + 1; j < rank + 1; j++)  sum += a[i][j] * b[j];
		b[i] = (b[i] - sum) / a[i][i];
	}
	//高斯列主元消去法结束

	//printf("P(x) = %f%+fx%+fx^2%+fx^3+fx^3%+fx^4%+fx^5%+fx^6%\n\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);
	//拟合后的曲线数据重新赋值给原数组
	for (int s = 0; s < len_data_x; s++)
	{
		data_y[s] = b[0] + data_x[s] * b[1] + pow(data_x[s], 2) * b[2] + pow(data_x[s], 3) * b[3] + pow(data_x[s], 4) * b[4] + pow(data_x[s], 5) * b[5] + pow(data_x[s], 6) * b[6] + pow(data_x[s], 7) * b[7];
	}
