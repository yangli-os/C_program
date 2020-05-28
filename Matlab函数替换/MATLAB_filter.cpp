//MATLAB中的fiter函数，可以在保证计算精度的前提下，运行在单片机上

#include <stdio.h>
#include <memory.h>

//len取决于len(b)和len(a)的最大值，在main里面
float filter_zi[2] = {0};                                   //zi(len) = {0*len}
float filter_A[3] = {0};                                    //{0*(len+1)}
float filter_B[3] = {0};                                    //{0*(len+1)}
float filter_zf_last[2] = {0};                              //{0*len}

//(output,zf,b,a,len(b),len(a),input,len(input),max(len(a),len(b))-1)
//(输出,zf,b,a,len(b),len(a),输入,len(input),最大值(len(a)和len(b))-1)
void MatlabFilter(float y[], float	zf[],float b[], int lenB, float a[], int lenA, float x[], int lenX, int len)
{
	int	i, j;
	// a[0] must is 1,if not 
	if (a[0] != 1) {
		for (i = 1; i != lenA; i++)
			a[i] /= a[0];
		for (i = 1; i != lenB; i++)
			b[i] /= a[0];
		a[0] = 1;
	}

	if (zf != NULL) {
		memset(zf, 0, len * sizeof(float));
	}
	//formula
	if (zf == NULL ) {

		y[0] = b[0] * x[0];
		for (i = 1; i < lenX; i++) {
			y[i] = 0;
			for (j = 0; j <= (lenB-1); j++) {
				if (i - j < 0)	break;
				y[i] += b[j] * x[i - j];
			}
			for (j = 1; j <= (lenA-1); j++) {
				if (i - j < 0)	break;
				y[i] -= a[j] * y[i - j];
			}
		}

	}

	else {
		memcpy(filter_A, a, lenA * sizeof(float));
		memcpy(filter_B, b, lenB * sizeof(float));
		memcpy(filter_zf_last, filter_zi, len * sizeof(float));
        //formula
		for (i = 0; i != lenX; i++) {
			y[i] = b[0] * x[i] + filter_zf_last[0];
			zf[len - 1] = filter_B[len] * x[i] - filter_A[len] * y[i];
			for (j = len - 2; j >= 0; j--){
				zf[j] = filter_B[j + 1] * x[i] + filter_zf_last[j + 1] - filter_A[j + 1] * y[i];
                }
			memcpy(filter_zf_last, zf, len * sizeof(float));
		}
	}

	return;
}
int main()
{
	float	filter_b[]  = { 0.2373, 0, -0.2373 };
	float	filter_a[] = { 1, -1.5131, 0.5255 };
	float	filter_x[] = { 0.9134,0.6324,0.0975,0.2785,0.5469,0.9575,0.9649,0.1576,0.9706,0.9572};
	int     filter_lenB = sizeof(filter_b)/sizeof(filter_b[0]);                                 //len(b)
	int     filter_lenA = sizeof(filter_a)/sizeof(filter_a[0]);                                 //len(a)
	int     filter_lenx = sizeof(filter_x)/sizeof(filter_x[0]);                                 //len(x)
	float	filter_y[filter_lenx];

	int		filter_len = (filter_lenB-1) > (filter_lenA-1) ? (filter_lenA-1) : (filter_lenB-1);
	float   filter_zf[filter_len];


	MatlabFilter(filter_y, filter_zf, filter_b, filter_lenB, filter_a, filter_lenA, filter_x, filter_lenx, filter_len);

	for (int i = 0; i != filter_lenx; i++)
		printf("%f ", filter_y[i]);
	printf("\n");
	for (int i = 0; i != 2; i++)
		printf("%f ", filter_zf[i]);
}
/*	验证...................
1. MATLAB

b = [0.8147,0.9058,0.1270];
a = { 1, -1.5131, 0.5255 };
x=[0.9134,0.6324,0.0975,0.2785,0.5469,0.9575,0.9649,0.1576,0.9706,0.9572];
[y,zf] = filter(b,1,x);

2. C++

int main()
{
	float	filter_b[]  = { 0.2373, 0, -0.2373 };
	float	filter_a[] = { 1, -1.5131, 0.5255 };
	int	    filter_x[] = { 0.9134,0.6324,0.0975,0.2785,0.5469,0.9575,0.9649,0.1576,0.9706,0.9572};
	
	int     filter_lenB = sizeof(filter_b)/sizeof(filter_b[0]);                             //len(b)
	int     filter_lenA = sizeof(filter_a)/sizeof(filter_a[0]);                             //len(a)
	int     filter_lenx = sizeof(filter_x)/sizeof(filter_x[0]);                                 //len(x)
	float	filter_y[filter_lenx];

	int		filter_len = (filter_lenB-1) > (filter_lenA-1) ? (filter_lenA-1) : (filter_lenB-1);
	float   filter_zf[filter_len];


	MatlabFilter(filter_y, filter_zf, filter_b, filter_lenB, filter_a, filter_lenA, filter_x, filter_lenx, filter_len);

	for (int i = 0; i != filter_lenx; i++)
		printf("%f ", filter_y[i]);
	printf("\n");
	for (int i = 0; i != 2; i++)
		printf("%f ", filter_zf[i]);
*/