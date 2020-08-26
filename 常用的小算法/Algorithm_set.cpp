/******************************************************计算最大值最小值相关的**************************************************/

//2个数求最大值
int getMax_int(int a, int b)
{
    return a > b ? a : b;
}

//2个数求最大值,float数据
float getMax_float(float a, float b)
{
    return a > b ? a : b;
}

int max_slope(float* x,int left,int len_x)
// 计算斜率最大点
// *x数组,left起始点的位置，len_x截止点的位置
// 返回斜率最大点所在的位置

{
    int max_position = 0;
    float max_y = x[1] - x[0];
    for (int i = left;i < len_x; i++)
    {   
        if (max_y < x[i + 1] - x[i])
        {
            max_y = x[i + 1] - x[i];
            max_position = i+1;
        }   
    }
    return max_position;
}

float min_position(float* a, int len_a)
{
    //计算最小值所在的位置
    int position = 0;
    float min_y = 65535;               //初始化一个最大值
    for (int i = 0; i < len_a; i++)
    {
        if ( a[i] < min_y)
        {
            min_y = a[i];
            position = i;
        }
    }
    return position;
}


/*****************************************************计算中值相关的****************************************************/
// 计算中值
int mid_int(int* a, int n)
{
    int res;

    sort_int(a, 0, n - 1);                                               //需要调用排序算法，跟快速排序联合使用
    if (n % 2 == 0)
        res = (int)round_self((a[n / 2 - 1] + a[n / 2]) / 2.0);
    else
        res = a[(n - 1) / 2];

    return res;
}

// 计算中值
float midFloat(float* a, int n)
{
    float res;

    sortFloat(a, 0, n - 1);                                               //需要调用排序算法，跟快速排序联合使用
    if (n % 2 == 0)
        res = (a[n / 2 - 1] + a[n / 2]) / 2.0;
    else
        res = a[(n - 1) / 2];

    return res;
}

float mid_mixFloat(float* a, int n)
//计算去掉最大最小的一定数量点后返回中值
{
    float res;
    float mid_mix[100] = { 0 };
    int len_mix = 0;
    sortFloat(a, 0, n - 1);
    for (int i = n*0.25; i < n*0.75; i++)                      //保留点的范围0.25-0.75
    {   
        mid_mix[len_mix++] = a[i];
    }
    if (len_mix % 2 == 0)
        res = (mid_mix[len_mix / 2 - 1] + mid_mix[len_mix / 2]) / 2.0;
    else
        res = mid_mix[(len_mix - 1) / 2];
    return res;
}

/***************************************************排序相关的*************************************************************/

//快速排序
void sortFloat(float s[], int l, int r)
{
//排序数组，0，n-1
//无输出，输入的原始数组改变
    if (l < r)
    {
        int i = l, j = r;
        float x = s[l];
        while (i < j)
        {
            while (i < j && s[j] >= x)
                j--;
            if (i < j)
                s[i++] = s[j];
            while (i < j && s[i] <= x)
                i++;
            if (i < j)
                s[j--] = s[i];
        }
        s[i] = x;
        sortFloat(s, l, i - 1);
        sortFloat(s, i + 1, r);
    }
}

void sort_int(int s[], int l, int r)
{
//排序数组，0，n-1
//无输出，输入的原始数组改变
    if (l < r)
    {
        int i = l, j = r;
        int x = s[l];
        while (i < j)
        {
            while (i < j && s[j] >= x)
                j--;
            if (i < j)
                s[i++] = s[j];
            while (i < j && s[i] <= x)
                i++;
            if (i < j)
                s[j--] = s[i];
        }
        s[i] = x;
        sort_int(s, l, i - 1);
        sort_int(s, i + 1, r);
    }
}


/******************************************************其他功能函数******************************************************/

//round()函数
float round_self(float val)
{
    return (val > 0.0) ? floor(val + 0.5) : ceil(val - 0.5);    
}

void Generate_bell_wave(float* x, int len_wave)
// 生成一个钟型波
// x一个钟型波的前半个波,len_wave输入钟型波的长度
// wave_all全局变量为生成的钟型波
{   
    float wave_all[100] = {0.0};
    int wave_position = 0;
    for (int i = 0; i < len_wave; i++)
    {
        wave_all[wave_position++] = x[i];
    }
    for (int j = len_wave-2; j >= 0; j--)
    {
        wave_all[wave_position++] = x[j];
    }
}

float means_mixFloat(float* a, int n)
//计算去掉最大最小的一定数量点后求均值
{
    float res;
    float mid_mix[100] = { 0 };
    int len_mix = 0;
    sortFloat(a, 0, n - 1);
    for (int i = n * 0.25; i < n * 0.75; i++)               //保留点的范围0.25-0.75
    {
        mid_mix[len_mix++] = a[i];
    }
    //求平均值
    float sum = 0;
    for (int i = 0; i < len_mix; i++) // 求和
        sum += mid_mix[i];
    res =  sum / len_mix; // 得到平均值
    return res;
}

float discrete_area(float* x, int left, int len_x)
{
    //求离散曲线的面积
    //离散数据，左界限，右界限
    //return 一个面积值
    float area = 0.0;
    for (int i = left; i < len_x-1; i++)
    {
        area += (x[i]+x[i+1])/2;
    }
    return area;
}
