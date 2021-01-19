// HM117_algorithm.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "HM117_algorithm.h"
#include <string>
//负责人：夏顺   时间： 2019.12.31  微信：15369748330  代码开发环境是win7 , vs2019
//本程序总体来看共分为两个模块，第一个模块是停止打压算法的模块，第二个是停止打压后计算血压具体数值的模块。以tingzhidaya为标志位
//算法的思想流程图见文档，下面主要对程序进行注释和说明。

////停止打压算法流程概述：函数的入口是两个数组，一是脉搏波数据，另一个是血压数据，这两个数据的数据是实时更新的，我们约定每增加125个点，
////调用一次该BP_Analyse函数，返回值中返回STOP_FLAG标志位，我们会一直打压，一直更新数组，直到STOP_FLAG标志位置1，所以我们的算法可以理解为：
////在某一次更新数组后，我们将标志位置1，让其停止打压。这里注意：数组的更新方式是末端不断增加新的125个点的新增方式，不是取代的方式。

uint16_t checkstop[40][6] = { 0 };   //二维数组，用来存放每次更新数组后，我们需要的参数。共六列：
////checkstop列数据说明，第一列最大值点，第二列上升个数，第三列下降个数，第四列脉搏波置信度个数，第五列不上升不下降的异常个数，第六列总体脉搏波个数
//说明：这里这里的最大值点是指目前的脉搏波数据中第几个波是最大脉搏波，上升个数是指对于新增的125点是否新增了一个峰值点，这个峰值点是相对于前一个点是上升还是下降。若上升，算作上升个数，若下降，算作下降个数
uint16_t p_max_id = 0;  //最大脉搏波点的p_peak中的id
uint16_t p_id = 0;      //最大脉搏波点在checkstop中的id
uint16_t mjudge_peak_height = 80;  //
uint16_t p_max_guding_flag = 0; //标志位：是否已确定最大值点
uint16_t p_max_yichang_id[10] = {0};  //一维数组：记录确定的最大值点是否为异常波形点。
uint16_t uu_num[40] = { 0 };   //一维数据：用来记录峰值点的个数
uint16_t p_max_yichang_num = 0;  //标志位：记录p_max_yichang_id[10]这个数组的真实长度。
uint16_t little_disturb = 0;    //记录较小的干扰的个数
uint16_t confidence_stop = 0;   //记录置信度不再增长的个数
uint16_t jueduigaodutaida = 0;
uint16_t first_checkstop_num = 0;  //当有六个峰值点存在时，此时的checkstop_num数值
uint16_t first_checkstop_num_flag = 0;  // first_checkstop_num数值的标志位。
uint16_t tingzhidaya = 0;       //重要标志位： 是否停止打压，若为0表示不停止打压，即继续打压。若为1表示停止继续打压，开始计算具体的血压数值。
uint16_t checkstop_num = 0;      //指第几次更新数组，即是第几个传输进来的125个点，由外部传入。
uint16_t Xstop = 0;              //记录开始存储时波峰的个数，即checkstop[40][6]数组的长度。
uint16_t little_high = 0;        //用来记录非常特殊的情况，这种情况是打压过大，波形形状接近于直线，称之为很小高度波形。
uint16_t tingzhi_ganrao = 0;     //当停止打压时记录可能存在的干扰，用于波形置信度的合成。
int16_t  peakadd_stop = 0;       //一般情况下，随着打压的进行，数组的更新，波峰数量会一直增长，但可能停止增长，此变量用来记录停止增长的个数。
uint16_t p_max_worse = 0;
uint16_t p_data30id = 0;         //我们在记录血压时，为减小干扰会舍弃压力值小于30mmhg的数据，此变量记录当压力值为30mmhg时，横坐标的id。
uint16_t  stop_flag = 0;         //停止打压的标志位，置0则不停止打压，置1则停止打压。
uint16_t peak_now = 0;           //当前峰值点的高度
uint16_t peak_need = 0;          //我们需要停止打压时的峰值点的高度
uint16_t bp_now = 0;             //当前的压力值
uint16_t bp_max = 0;             //我们需要的停止打压时的最大压力值
uint16_t x_stop_max = 0;         //找到最大值点时的横坐标
uint16_t y_stop_max = 0;         //找到最大值点时的纵坐标
uint16_t sum_three = 0;          //最大值点之后最近的三个峰值点的高度之和
uint16_t low_peak = 0;           //低于我们需要的停止打压时的峰值点的高度的次数

uint16_t p_diff = 200;           //当前后两个峰值点的差距小于p_diff时，我们视为是上升或者下降，否则都是过大或者过小的不正常上升或者下降。
uint16_t b_bp_v = 0;             //存在绑的比较松的袖带或者胳膊比较细的用户，导致打压不正常，通过检测第六次更新数组，即p_num=750时的压力值，自适应的调整p_diff和stop_sum_three的数值。
uint16_t stop_sum_three = 300;   //当最大值点的高度与sum_three的差值大于stop_sum_three时，应视为不正常的最大值点（最新的算法将其删除了）
uint16_t dbp = 0, sbp = 0;       //低压值，高压值
uint16_t MAX_numt = 0;           //计算血压值阶段转化后的最大值点的横坐标（这里以及下文的转化是指在计算血压时，由于内存的原因，将拟合后的点的个数减小20倍，每隔20个点取一个值。程序内能够看懂）
uint16_t peak_max_id = 1;        // 在停止打压阶段计算的最大值点的横坐标
uint16_t numt = 0;               // 计算血压值阶段转化后的总的点的个数。  
uint16_t avg_max_id = 0;         //计算血压值阶段最大值点的横坐标
uint16_t avg_max_y = 0;			 //计算血压值阶段最大值点的纵坐标
uint16_t f_hr = 0;               //心率

//函数说明：算法仅此一个函数：函数的入口包括p_data, p_bp, p_num_wave, p_num_bp, checkstop_num五个参数，其含义分别脉搏波数据，血压数据，脉搏波数据的长度，血压数据的长度，第几次调用本函数。
//函数的输出包括：HR,SBP, ABP,DBP, XLBQ_FLAG, BP_CONFIDENCE, STOP_FLAG,LOOSE_CUFF八个参数，其含义分别是心率，高压，平均压，低压，心率不齐标志位，置信度，停止打压标志位，袖带过松标志位。
void BP_Analyse(uint16_t * p_data, uint16_t* p_bp, uint16_t p_num_wave, uint16_t p_num_bp, uint16_t checkstop_num,  uint16_t* HR, uint16_t* SBP, uint16_t* ABP, uint16_t* DBP, uint8_t* XLBQ_FLAG, uint16_t* BP_CONFIDENCE, uint16_t* STOP_FLAG , uint16_t * LOOSE_CUFF)
{
	
	*HR = 0;
	*SBP = 0;
	*DBP = 0;
	*ABP = 0;
	*XLBQ_FLAG = 0; 
	*BP_CONFIDENCE = 10;
	*STOP_FLAG = 0;
	*LOOSE_CUFF = 0;
	checkstop_num++;  //每次调用，此参数加一
	uint16_t ITX = 0;
	uint16_t ITX2 = 0;
	int i = 0, j = 0, h = 0;
	float FTX = 0;
	float FTX2=0;
	float FTX3 = 0;
	float FTX4 = 0;
	uint16_t  utx = 0;
	uint16_t p_num = p_num_wave;
	uint8_t BP_CONFIDENCE_PRE = 0;  //预置信度，参与最终置信度的合成
	uint16_t sample_rate = 128; //采样率

	int SUMint = 0;
	if (checkstop_num == 200)
	{
		tingzhidaya = 1;
	}
	//该函数用来检测袖带是否过松，若打压10秒后，袖带压力值仍然小于5，则可以判定过松或者空打。
	if (p_num >= sample_rate * 10)  
	{
		if (p_bp[p_num_bp - 1] <= 5)
		{
			*STOP_FLAG = 1;
			tingzhidaya = 1; //停止打压标志位置1
			*LOOSE_CUFF = 1; //袖带松标志位置1
		}

	}

	if (tingzhidaya == 0) // 若停止打压标志位为0，则表明正在进行停止打压的过程，这个过程是反复的，直到标志位置1
	{
		if (p_bp[p_num_bp - 1] < 30)  //袖带较松的进一步判定
		{
			
			if (p_num >= sample_rate * 16)
			{
				*LOOSE_CUFF = 1;
				*STOP_FLAG = 1;
				return;
			}
			else
			{
				*STOP_FLAG = 0;
				return;
			}
			
		}
		else if (first_checkstop_num_flag == 0) // 记录当压力值大于30时此时的checkstop_num，记为first_checkstop_num
		{
			first_checkstop_num = checkstop_num - 1;
			first_checkstop_num_flag = 1;

		}

		for (i = 0; i < p_num_bp; i++)
		{
			if (p_bp[i] == 30)  //当压力值达到30时，记录此时的下标
			{
				p_data30id = i;
				//break;
			}
		}
		////下面两个for循环将压力值为30之前的脉搏波和血压都清为0，意为舍弃前压力值为30的数据。
		for (i = 0; i < p_data30id * 10; i++)
		{
			p_data[i] = 0;
			/*	p_bp[i] = p_bp[i + ITX2];*/
		}
		for (i = 0; i < p_data30id; i++)
		{
			p_bp[i] = 0;

		}


		SUMint = 0;
		h = 0;

		for (i = 0; i < p_num; i++)
		{

			if (p_data[i] <= 0)// 何时停止打压的算法，不用去直线趋势	
			{
				p_data[i] = 0;  //去除数据中的负数异常数据
			}

		}


        //下面的代码主要功能是将峰值点找出来，基本思路是上升沿的终点和下降沿的起点是峰值点
		uint16_t down_count = 0, up_count = 0;  //上升的个数和下降的个数
		uint8_t d_nn = 0, u_nn = 0;   //上升的有效最终个数，下降的有效最终个数
		uint16_t Down_arr[121][3] = { 0 };  //下降沿数组，三列分别表示下降个数，下降沿的终点横坐标，下降沿的终点纵坐标
		uint16_t Up_arr[121][3] = { 0 };  //上升沿数组，三列分别表示上升个数，上升沿的终点横坐标，上升沿的终点纵坐标
		uint16_t down_count_plat = 0, up_count_plat = 0;   //不上升也不下降称之为平台期，该变量指平台期的个数。
		for (i = 0; i < p_num - 1; i++)
		{
			if (d_nn < 120 && u_nn < 120)
			{
				if (p_data[i] >= p_data[i + 1])
				{
					down_count++;
					if (p_data[i] == p_data[i + 1]) //平台期
					{
						down_count_plat++;

					}

					if (i == p_num - 2 && down_count > down_count_plat) // 把最后一个包含进去
					{
						Down_arr[d_nn][0] = down_count;
						Down_arr[d_nn][1] = i;          //下降沿的终点横坐标 
						Down_arr[d_nn][2] = p_data[i];  //下降沿的终点纵坐标
						d_nn++;
						down_count = 0;
						down_count_plat = 0;
					}
				}
				else
				{
					if (down_count > 5 && (down_count > down_count_plat))
					{
						Down_arr[d_nn][0] = down_count;
						Down_arr[d_nn][1] = i;          //下降沿的终点横坐标 
						Down_arr[d_nn][2] = p_data[i];  //下降沿的终点纵坐标
						d_nn++;
						down_count = 0;
						down_count_plat = 0;
					}
					down_count = 0;
					down_count_plat = 0;
				}

				if (p_data[i] <= p_data[i + 1])
				{
					up_count++;
					if (p_data[i] == p_data[i + 1]) //平台期
					{
						up_count_plat++;

					}
					if (i == p_num - 2 && up_count > up_count_plat) // 把最后一个包含进去
					{
						Up_arr[u_nn][0] = up_count;
						Up_arr[u_nn][1] = i;          //上升沿的终点横坐标
						Up_arr[u_nn][2] = p_data[i];  //上升沿终点纵坐标
						u_nn++;
						up_count = 0;
						up_count_plat = 0;
					}
				}
				else
				{
					if (up_count > 5 && (up_count > up_count_plat))
					{
						Up_arr[u_nn][0] = up_count;
						Up_arr[u_nn][1] = i;          //上升沿的终点横坐标
						Up_arr[u_nn][2] = p_data[i];  //上升沿终点纵坐标
						u_nn++;
						up_count = 0;
						up_count_plat = 0;
					}

					up_count = 0;
					up_count_plat = 0;
				}
			}
		}
		uint8_t max_nn = 0;
		max_nn = u_nn > d_nn ? u_nn : d_nn;  //max_nn取u_nn 和 d_nn 之中的较大值
		uint8_t uu = 0;
		uint16_t p_peak[121][3] = { 0 }; //波峰数组：记录波峰的横纵坐标
		uint16_t p_valley[121][2] = { 0 }; //波谷数组：记录一个波峰的前一个波谷横纵坐标和后一个波谷的横纵坐标
		uint16_t next_down_x = 0, next_down_y = 0;   //  后一个下降沿的横坐标和纵坐标
		uint16_t before_down_x = 0, before_down_y = 0; //前一个下降沿的横坐标和纵坐标
		////上面的代码将所有下降沿和上升沿找了出来，以下代码分别将上升沿和下降沿一一对应，不能搞错乱掉。
		for (i = 0; i < u_nn; i++)
		{
			for (j = 1; j < d_nn; j++)
			{

				if (Down_arr[j][1] < Up_arr[i][1])
				{
					continue;

				}
				else
				{
					next_down_x = Down_arr[j][1];
					next_down_y = Down_arr[j][2];
					before_down_x = Down_arr[j - 1][1];
					before_down_y = Down_arr[j - 1][2];
					break;
				}
			}

			if ((abs_int(Up_arr[i][2] - next_down_y) > mjudge_peak_height && abs_int(Up_arr[i][2] - next_down_y) < 4000 && abs_int(Up_arr[i][2] - before_down_y) > mjudge_peak_height && abs_int(Up_arr[i][2] - before_down_y) < 4000) && (abs_int(Up_arr[i][1] - before_down_x) > 10 && (next_down_x - Up_arr[i][1]) > 10))
			{
				//2.14日修改，把这里1500的阈值进行扩大到4000，其实这里可以没有必要进行最大阈值的限制，不影响以前完整的算法。


				/*p_valley[uu][0] = before_down_x;*/
				p_valley[uu][0] = before_down_y;   //记录左边波谷的高度
				/*p_valley[uu][2] = next_down_x;*/
				p_valley[uu][1] = next_down_y;     //记录右边波谷的高度

				p_peak[uu][0] = Up_arr[i][1];   // 记录波峰的横坐标
				p_peak[uu][1] = Up_arr[i][2] - p_valley[uu][1];   //波峰的高度：定义为最高值点与右边波谷的高度差值（停止打压算法阶段这样定义）
				uu++;                                               
			}

			if (abs_int(Up_arr[i][2] - next_down_y) > 1500 || (abs_int(Up_arr[i][2] - before_down_y) > 1500))
			{
				//p_peak[uu][0] = Up_arr[i][1];
				//p_peak[uu][1] = Up_arr[i][2];
				//p_valley[uu][0] = before_down_x;
				//p_valley[uu][1] = before_down_y;
				//p_valley[uu][2] = next_down_x;
				//p_valley[uu][3] = next_down_y;
				//uu++;
				BP_CONFIDENCE_PRE = BP_CONFIDENCE_PRE + 0;  //若脉搏波高度过大，则需要降低一定置信度    //2.14日修改，将其改成0，以前是BP_CONFIDENCE_PRE + 8，将BP_CONFIDENCE_PRE + 8放在了下文新的地方。
			}
			//}
			//else if ((abs_int(Up_arr[i][1] - before_down_x) > 10 && abs_int(Up_arr[i][1] - next_down_x) > 10))
			//{
			//	p_peak[uu][0] = Up_arr[i][1];
			//	p_peak[uu][1] = Up_arr[i][2];
			//	//p_valley[uu][0] = before_down_x; /////////////////
			//	p_valley[uu][0] = before_down_y;
			//	//p_valley[uu][2] = next_down_x; /////////////////
			//	p_valley[uu][1] = next_down_y;
			//
			//	//p_peak[uu][1] = Up_arr[i][2] - p_valley[uu][0];
			//	uu++;
			//}

		}


	
	//FILE* fw55;
	//errno_t err55;
	//err55 = fopen_s(&fw55, "D:\\HM117\\117DATA\\1136CC.txt", "wb");
	//for (int i = 0; i < p_num; i++)
	//{

	//	fprintf(fw55, "%d\r\n", p_data[i]);
	//}
	//fclose(fw55);


	//求最大峰值点

	//for (i = 0; i < uu; i++)
	//{
	//	if (p_peak[i][1] > ITX)
	//	{
	//		ITX = p_peak[i][1];
	//		peak_max_id = i;
	//	}
	//}


	//判断何时停止，11.13新增加
	/////////////////////////////////////////////
	//uint16_t checkstop[100][3] = { 0 };  //三列，分别是最大值点横坐标，每次检测时的上升个数，下降个数



		ITX = 0;
		ITX2 = 0;
		
		uint16_t qianqiyichang = 0;//前期异常
		uint16_t yichang = 0;//异常波形
		uint16_t  epoch_num = 0; //这个参数比较重要，它的含义是每一次新增的125个点中，新增了几个峰值点，这个数值正常情况是1，心率慢的人可能是0，心率快的人可能是2.

		uu_num[checkstop_num] = uu - 1 > 0 ? uu - 1 : 0;
		////epoch_num参数的计算，主要是从uu_num数组中获取
		epoch_num = uu_num[checkstop_num] - uu_num[checkstop_num - 1] > 0 ? uu_num[checkstop_num] - uu_num[checkstop_num - 1] : 0;
		
		if (checkstop_num <= 6+ first_checkstop_num)  //这里的规定是至少调用6次本函数，保证不会提前停止的太早或者太离谱
		{
			stop_flag = 0; //flag =0 表示不停止。继续打压
			*STOP_FLAG = 0;
			return;
		}

		/*checkstop_num = checkstop_num - first_checkstop_num;*/
		/*p_max_id = 6;*/

		//FILE* fw5;
		//errno_t err5;
		//err5 = fopen_s(&fw5, "D:\\HM117\\117DATA\\1136CC.txt", "wb");
		//for (int i = 0; i < p_num; i++)
		//{

		//	fprintf(fw5, "%d\r\n", p_data[i]);
		//}
		//fclose(fw5);

		////checkstop赋值阶段

		
		uint16_t max_x_bubian = 0;  //是指checkstop数组中的第一列（即最大值点列）,随着数据的更新，该第一列值不变的个数
		uint16_t up_x_bubian = 0;   //是指checkstop数组中的第二列（即上升个数列）,随着数据的更新，该第二列值不变的个数
		uint8_t p_max_better = 0;   //指找到最大值点后，后续又出现更好的最大值点的情况，该变量是指更好的最大值点保持不变的次数
		ITX = 0;
		ITX2 = 10000;
		p_diff = 200;
		stop_sum_three = 300;
		if (p_num >= 750 && p_num<= 1000)   //为p_diff设定自适应数值，主要是通过查看p_bp的第75个压力值的大小，若压力值较大，大于70，则说明打压比较快，或者绑的比较松，需要调整阈值。
		{                                   //至于为何是取第75个点，为何该点数值要大于70，是经验所得。
			b_bp_v = p_bp[75];
			if (b_bp_v > 70)
			{
				p_diff = 200 + 2 * (b_bp_v -70);        //自适应调整p_diff阈值
				stop_sum_three = 300 + 2 * (b_bp_v - 70);
			}
		}





		//脉搏波粗略置信度概念的设置，主要针对脉搏波左右高度是否相差太大以及单个波峰高度是否过高。
		SUMint = 0;
		h = 0;
		for (i = 0; i < uu; i++)
		{
			if (abs_int((p_valley[i][0] - p_valley[i][1])) < p_diff && p_peak[i][1] < 1300) //左高度减去右高度
			{
				p_peak[i][2] = 1;   //p_peak是最重要的一个数组之一，它有三列，三别记录着每个波峰的横坐标，纵坐标和置信度。是观察波峰是否正确的第一选择。
			}
			else
			{
				p_peak[i][2] = 0;   
			}

		}
		//2020.2.14更改内容
		//更改原因：对于一些老人的数据，其脉搏波高度非常大，但却是正常波形。所以此处判断置信度时不能只用简单的高度
		//以下为2020.2.14新增，目的是将这些老人比较大的脉搏波也识别为正常波形，尽量使改动不影响以前的算法
		//所以这种情况我们应该将其视为特殊情况，筛选出来，特殊对待。我们设置置信度的概念是为了防止干扰，老人的波形和干扰的区别是，干扰时突然出现的，老人的波形却是逐渐上升的。
		////特殊情况筛选：
		for (i = 0; i < uu; i++)
		{
			if (p_peak[i][2] == 0) //针对置信度为0的波形
			{
				if (i > 4)
				{
					//判断其连续性
					if(abs_int(p_peak[i][0] - p_peak[i-1][0]) < abs_int(( p_peak[i - 1][0]- p_peak[i - 2][0] + p_peak[i - 2][0] - p_peak[i - 3][0]+ p_peak[i - 3][0] - p_peak[i - 4][0])/3  ) +50)
					{
						p_peak[i][2] =1;  //若是连续上升或者下降在一定范围，而不是突然出现的，则认为是没有问题，重新置1.
					}
					else
					{
						BP_CONFIDENCE_PRE = BP_CONFIDENCE_PRE + 8;     //这个else才是真正的干扰，将BP_CONFIDENCE_PRE放在这里
					}
				}
			}
		}
		//以上为2.14日代码修改，应该适用于老年人数据，同时也不影响年轻人的数据。



		//这个if的功能是：在新的更新的点中找不到峰值点的处理方法，找不到峰值点称之为峰值停止，即变量peakadd_stop，当peakadd_stop到达一定数值，我们判定可以停止打压了。
		if (epoch_num == 0 && p_num > 750)
		{
			for (i = 1; i < 125; i++)
			{
				if (p_data[p_num - i] > ITX)
				{
					ITX = p_data[p_num - i]; //最大值
				}
				else if (p_data[p_num - i] < ITX2)
				{
					ITX2 = p_data[p_num - i];
				}

			}
			if (uu > 2)
			{
				if (ITX - ITX2 < 80)
				{
					little_high++;  //此处参照little_high的定义，找不到峰值点时还有一种情况，就是整体趋势接近于直线，没有起伏。此时波峰高度极小。
				}
				peakadd_stop++;
			}
		}
		else
		{
			peakadd_stop--;
			peakadd_stop = peakadd_stop > 0 ? peakadd_stop : 0;
		}
		ITX = 0;
		ITX2 = 0;

		//停止打压的算法的真正核心部分，根据新增的峰值点的个数决定执几次，每次执行过程过会更新checkstop数组。
		while (epoch_num != 0) // checkstop更新
		{
			//初始化参数
			yichang = 0;
			max_x_bubian = 0;
			up_x_bubian = 0;
			peak_now = 0;
			peak_need = 0;
			bp_now = 0;
			bp_max = 0;
			p_max_better = 0;
			sum_three = 0;
			low_peak = 0;

			//这个for循环是指将异常的峰值点的置信度置0
			for (i = 0; i < p_max_yichang_num; i++)
			{
				p_peak[p_max_yichang_id[p_max_yichang_num-1]][2] = 0;
				
			}
			//checkstop的第5列赋值，含义是现有多少个峰值点，不算最后的一个峰值点（原因是最后一个所谓的峰值点可能不完整，故不可信）。
			checkstop[Xstop][5] = uu - 1 - (epoch_num - 1);  //checkstop的第5列是指第几个峰值 ，例如第4个峰值，则在p_peak中应该是下标为3的那个点，因为下标是从0开始的。  

			for (i = 0; i < uu - 1 - (epoch_num - 1); i++)  //  该for循环为checkstop的第三列，也就是置信度列进行赋值。
			{
				if (p_peak[i][2] == 1)
				{
					checkstop[Xstop][3]++; 
				}
			}

			for (i = 0; i < uu - 1 - (epoch_num - 1) - 1; i++)     //该for循环针对checkstop的第1 2 3 4 列进行赋值。是进行各种算法的前提
			{
				if (p_peak[i + 1][1] - p_peak[i][1] > 0 && p_peak[i + 1][1] - p_peak[i][1] < p_diff)   //2020.2.14日备注，这里的p_diff是设置的默认200，2.14没有修改，因为通过观察，老人数据幅度虽然很大，但是是没有问题的，每个脉搏波的增长高度是在200以内的
				{																						//所以这里是没有修改的，但这里仍然需要关注。若后续有些正常的数据，但是增长的幅度很高，超过200，则此处需要合理适当改动。
					checkstop[Xstop][1]++;   //第二列是上升的个数列

					if (p_peak[i + 1][1] > ITX && p_peak[i + 1][2] == 1)
					{
						ITX = p_peak[i + 1][1];
						checkstop[Xstop][0] = i + 1;   //第一列的最大值列
					}
				}
				//else 
				//{
				//	yichang_up++;
				//	checkstop[checkstop_num][3] = yichang_up;
				//}
				else if (p_peak[i][1] - p_peak[i + 1][1] > 0 && p_peak[i][1] - p_peak[i + 1][1] < p_diff)
				{
					checkstop[Xstop][2]++;              //第三列的下降个数列
				}
				else
				{
					yichang++;
					checkstop[Xstop][4] = yichang;        // 第四列的异常个数列
				}
			}
			Xstop++;                                //每一个循环，Xstop自增1
			epoch_num--;                            //每一个循环，epoch_num自减1
			/////////////checkstop判断阶段
			ITX = 0;
			ITX2 = 0;
			for (i = 0; i < Xstop; i++)   //这个for循环用来查找目前的checkstop数组中，第1列最大值列和第2列上升个数列的最大值。
			{
				if (checkstop[i + 1][0] > ITX)
				{
					ITX = checkstop[i + 1][0];
				}

				if (checkstop[i + 1][1] > ITX2)
				{
					ITX2 = checkstop[i + 1][1];
					/*up_x_bubian++;*/
				}
			}
			for (i = 0; i < Xstop; i++)      //这个for循环开始查找最大值点，确定部分停止条件。这里特别注意for循环的范围，以及for循环对全局变量的影响（此处全局变量是指定义在BP_Analyse这个大函数之外的变量）
			{
				if (p_max_guding_flag == 0)  //参照p_max_guding_flag定义，针对还没找到最大值点的情况，我们要去确定最大值点
				{
					if (checkstop[i][0] == ITX && p_peak[checkstop[i][5] - 1][2] == 1 && checkstop[i][0] != p_max_yichang_id[p_max_yichang_num-1])
					{
						max_x_bubian++;   //若最大值列的数字保持不变，计数加1 ；
					}
					else
					{
						max_x_bubian = 0;
					}
					
					if (checkstop[i][1] == ITX2)
					{
						up_x_bubian++;       ////若上升个数列的数字保持不变，计数加1，（上升个数在最新版本中删除了） ；
					}
					if ((p_bp[p_num_bp-1]) > 140)  // 没有找到最大值的情况下， 如果压力值已经打到140mmhg，则考虑干扰情况
					{
						if ((checkstop[Xstop - 1][5] - checkstop[Xstop - 1][3]) - (checkstop[0][5] - checkstop[0][3]) >= 4) //若后期混乱
						{

							stop_flag = 1;  //存在干扰，让其停止
							tingzhi_ganrao = checkstop[Xstop - 1][4] * 2 + (checkstop[Xstop - 1][5] - checkstop[Xstop - 1][3]) * 2 + 8;
							tingzhidaya = 1;   //并记录停止干扰的数值。
							break;   //这种情况需要中断循环

						}
					}
					//这种情况是确定最大值点的情况，最大值不变的数量大于3，并且该最大值点的高度要大于200；
					if (max_x_bubian >= 3 && p_peak[checkstop[Xstop - 1][0]][1] > 200)
					{

						h = 0;
						for (j = 0; j < uu - 1 - (epoch_num - 1); j++)
						{
							if (j > ITX && j < ITX + 4 && p_peak[j][2] == 1) //最近2个峰值点的平均值
							{
								sum_three = sum_three + p_peak[j][1];
								h++;
							}
							if (h >= 2)
							{
								break;
							}
						}
						if (h > 0)
						{
							sum_three = sum_three / h; //平均高度  
						}
						h = 0;

						//以上一段代码 是计算该最大值点的后三个点的的平均高度，看其是否与最大值点差距过大，若过大，考虑最大值点其实是干扰。（可选）

						y_stop_max = p_peak[checkstop[Xstop - 1][0]][1];
						x_stop_max = p_peak[checkstop[Xstop - 1][0]][0];
						//如果该最大值点对应的压力值低于60mmhg，我们认为这个最大值点是不正确的点。
						if ((p_bp[x_stop_max / 10]) < 60 ) //   || abs_int( y_stop_max  - sum_three) > stop_sum_three) //300
						{
							p_max_guding_flag = 0;
							p_max_yichang_id[p_max_yichang_num] = checkstop[Xstop - 1][0];
							p_max_yichang_num++;

						}
						else // 是最大值点，确定最大值点的id和重置标志位
						{
							p_max_id = ITX;
							p_max_guding_flag = 1;
							p_id = i;
						}
					}
				}
				else  // 这个else指，已经找到最大值点，这时我们仍然有错判最大值点的可能性。
				{

					if (p_id > 0 && i > p_id && checkstop[i][0] - checkstop[i - 1][0] > 0)
					{/////这种情况是因为某种原因确定了最大值，但后面有更好的最大值  (后面的最大值比前面的大)
						p_max_better++;
						if (p_max_better >= 3)
						{
							p_max_id = 0;
							p_max_guding_flag = 0;
							p_id = 0;
						}
					}

				}

			}

			if (p_max_guding_flag == 0) // 此处已不在for循环内。 该分支是针对一直找不到最大值点，判断是否存在的可能干扰，用confidence_stop全局变量表征，译为置信度停止增长；
			{
				if ((p_bp[p_num_bp - 1]) > 140)
				{

					if (checkstop[Xstop - 1][3] - checkstop[Xstop - 1 - 1][3] < checkstop[Xstop - 1][5] - checkstop[Xstop - 1 - 1][5])//置信度波形不再增长
					{
						confidence_stop++;
						if (confidence_stop >= 3)  // 当置信度停止增长达到一定数量,该值是经验和实验所得。
						{
							stop_flag = 1;	//stop_flag是停止打压标志位，这种情况下的停止打压，要计算打压过程中的干扰；						
							tingzhi_ganrao = checkstop[Xstop - 1][4] * 2 + (checkstop[Xstop - 1][5] - checkstop[Xstop - 1][3]) * 2 + 10; //打压干扰设置
							tingzhidaya = 1;
							break;    //主动停止
						}

					}
					//if (Xstop < 3)
					//{
					//	stop_flag = 1;
					//	tingzhidaya = 1;
					//	break;
					//}
					
				}
				if ((p_bp[p_num_bp - 1]) > 160) //这种情况是在实际测量中出现的较小概率的情况（产品部鲍淑娟），这种情况下的波形特点是心率低，胳膊细，导致打压过久（到200多），原因是出现的波峰点很少，当开始往checkstop数组中记录时（大于六个波峰），已经错过最大值点。具体表现就是压力值很大，但Xstop却仍然很小。
				{
					if (Xstop < 5)
					{
						stop_flag = 1;
						tingzhidaya = 1;
						break;
					}
				}

				for (int fff = 0; fff < uu - epoch_num; fff++) //这种情况是针对一直找不到最大值点（压力值大于140还未找到），可能是干扰可能是错过等原因找不到，但此时波形已经开始下降，此时的波形高度已经开始变小，并将一直减小
				{
					if (p_bp[(p_peak[fff][0]) / 10] > 140)
					{
						if (p_peak[fff][1] <= 200) //波峰的高度低于200，视为较低的波峰阈值，该值是经验和实验所得，不建议改变。
						{
							low_peak++;
							if (low_peak >= 2)
							{
								stop_flag = 1;
								tingzhi_ganrao = tingzhi_ganrao + 10;  //由于错过最大值点或者最大值点干扰，需要减少部分置信度
							}
						}
					}
				}
			}


			peak_now = p_peak[uu - 2 - epoch_num ][1] ;		//记录此时最大的波峰高度	
			bp_now = p_bp[p_num_bp - 1];                   //记录此时的压力值大小
			if (stop_flag == 1)                     //如果在之前已经有满足停止打压的条件，这里可以直接停止打压了（一般都是找不到最大值点的多种情况）
			{
				tingzhidaya = 1;
				break;
			}
			if (p_max_guding_flag == 1)          //大部分的正常的波形的停止打压条件都在该分支内（能找到最大值点）
			{
				peak_need =((p_peak[p_max_id][1]) * 55)/100;     //记录我们需要停止的波峰高度 （和最大波峰值相关，0.55倍）
				bp_max = p_bp[p_peak[p_max_id][0] / 10];         //记录我们需要停止的压力值大小（最大波峰时的压力值）

				if ((p_peak[uu-2-epoch_num][2]) == 1)  //置信度要过关
				{
					if ((peak_now <= peak_need ) && (uu - 2 - epoch_num-1)>p_max_id)   //如果当前波峰值小于需停波峰值，则停止
					{
                        stop_flag = 1;					
					}
					if (peak_now <= 200)   ////如果当前波峰值较小，则停止
					{
						stop_flag = 1;
					}
				}
				else                                //如果置信度有异常，可以视为小干扰（因为已经能找到最大值了）
				{
					little_disturb++;
					if (little_disturb >= 4)       //小干扰的数量过多，也要停止。
					{
						stop_flag = 1;
					}
				}

				if (bp_max < 80)                     //以上是从波峰角度，确定停止条件，这里是从压力值角度确定停止条件
				{                                    //原理是：在找到最大值点后，收缩压出现的地方不可能偏离最大值点太远，所以从压力值角度设置停止条件
					if (bp_now > bp_max * 1.8 && p_max_id != 0)  //从不同的最大值点的压力值，设置不同的压力值停止系数
					{
						stop_flag = 1;
					}
				}
				else if (bp_max < 100)                          //从不同的最大值点的压力值，设置不同的压力值停止系数
				{
					if (bp_now > bp_max * 1.7 && p_max_id != 0)
					{
						stop_flag = 1;
					}
				}
				else if (bp_max < 120)                         
				{
					if (bp_now > bp_max * 1.6 && p_max_id != 0)
					{
						stop_flag = 1;
					}
				}
				else if (bp_max < 140)                         
				{
					if (bp_now > bp_max * 1.5 && p_max_id != 0)
					{
						stop_flag = 1;
					}
				}
				else
				{
					if (bp_now > bp_max * 1.4 && p_max_id != 0)
					{
						stop_flag = 1;
					}
				}

				if (bp_now >= 250)    //最后设定一个保护值，若由于多种原因,以上停止条件都未满足，那么这将是最后一道防线，压力值不能大于250，250压力值是法规要求。
				{
					stop_flag = 1;
				}

			}
			else                //这里的else是指没有找到最大值点时，也要设定的压力保护值。
			{
				if (bp_now >= 250)//p_bp[p_peak[checkstop[Xstop - 1][0]][0] / 10]) >= 250
				{
					stop_flag = 1;
				}

			}
			if (Xstop - 1 >= 39)  //因为checkstop数组的长度是40，所以这里不能数组越界，这个大小已足够各种血压值的情况
			{
				stop_flag = 1;
			}

			if (stop_flag == 1)  //停止打压标志位
			{
				tingzhidaya = 1;
				break;
			}

		}    //while循环结束	

		//最后进行各种停止条件的综合和标志位的设置        
        bp_now = p_bp[p_num_bp - 1];   
        if (bp_now >= 250)   //当前压力值
		{
            stop_flag = 1;
		}
                
		if (little_high >= 3)  //小干扰情况
		{
			*STOP_FLAG = 0;
			tingzhidaya = 1;
			//return;
		}
		else if (peakadd_stop >= 3)  //置信度停止增长
		{
			*STOP_FLAG = 0;
			tingzhidaya = 1;
			//tingzhi_ganrao = 20;
			tingzhi_ganrao = checkstop[Xstop - 1][4] * 2 ;
			//return;
		}
		else if (stop_flag == 1)    ////
		{

			*STOP_FLAG = 0;
			tingzhidaya = 1;
		}
		else
		{
			*STOP_FLAG = 0;
			tingzhidaya = 0;
			//return;
		}
	}

//	FILE* fw5;
//errno_t err5;
//err5 = fopen_s(&fw5, "D:\\HM117\\117DATA\\1136CC.txt", "wb");
//for (int i = 0; i < p_num; i++)
//{
//
//	fprintf(fw5, "%d\r\n", p_data[i]);
//}
//fclose(fw5);


	if (tingzhidaya == 1)   //若停止打压标志位为1，则表明打压过程已经结束，开始进行计算血压值得模块，这个模块只运行一次，通过赋值STOP_FLAG标志位实现停止。
	{

		//在开始前，先将之前的数组和变量重新赋值为初始化状态
		for (i = 0; i < 40; i++)
		{
			for (j = 0; j < 6; j++)
			{
				checkstop[i][j] = 0;
			}
		}
		for (i = 0; i < 10; i++)
		{
			p_max_yichang_id[i] = 0;
		}
		for (i = 0; i < 40; i++)
		{
			uu_num[i] = 0;
		}

		p_max_id = 0;
		p_id = 0;
		mjudge_peak_height = 80;
		p_max_guding_flag = 0;

		p_max_yichang_num = 0;
		little_disturb = 0;
		confidence_stop = 0;
		low_peak = 0;
		first_checkstop_num = 0;
		first_checkstop_num_flag = 0;
		tingzhidaya = 0;
		checkstop_num = 0;
		Xstop = 0;
		little_high = 0;
		
		peakadd_stop = 0;
		p_max_worse = 0;
		p_data30id = 0;

		SUMint = 0;
		ITX = 0;
		ITX2 = 0;
		

		BP_CONFIDENCE_PRE = 0;




		//之前说过，我们只要压力值大于30mmhg之后的数据，所以我们要先找到压力值为30mmhg时的id位置
		ITX = 30;
		for (i = 0; i < p_num_bp; i++)
		{
			if (p_bp[i] == ITX)
			{
				ITX2 = i;  //位置确定
				//break;
			}
		}
		//uint16_t p_num2 = p_num;
		p_num = p_num - ITX2 * 10;    //重新赋值脉搏波长度
		p_num_bp = p_num_bp - ITX2;   //重新赋值血压值长度

		if(p_num < 750 || p_num >5000 || p_num_bp <30)   //防止死机情况，若发生该情况，应该全局变量置0，STOP_FLAG置1并return
		{
			*HR = 0;
			*SBP = 0;
			*BP_CONFIDENCE = 10;
			*DBP = 0;
			*ABP = 0;

			*XLBQ_FLAG = 0;


			tingzhi_ganrao = 0;
			dbp = 0;
			sbp = 0;
			MAX_numt = 0;
			peak_max_id = 1;
			numt = 0;
			avg_max_id = 0;
			avg_max_y = 0;
			f_hr = 0;
			stop_flag = 0;
			peak_now = 0;
			peak_need = 0;
			bp_now = 0;
			bp_max = 0;
			*STOP_FLAG = 1;
			return;
		}

		for ( i = 0; i < p_num; i++)         ////重新赋值脉搏波数组
		{
			p_data[i] = p_data[i + ITX2 * 10];
			/*	p_bp[i] = p_bp[i + ITX2];*/
		}
		for ( i = 0; i < p_num_bp; i++)       //重新赋值血压数据数组
		{
			p_bp[i] = p_bp[i + ITX2];

		}

		////以下代码是去除直线趋势代码。
		SUMint = 0;
		h = 0;
		FTX2 = 0;
		FTX = 0;
		for (i = 0; i < p_num; i++)
		{

			SUMint += i;      //mean_x
			h += p_data[i];  // //mean_y
			FTX2 += i * p_data[i];
			FTX += i * i;
		}

		if (p_num > 0)
		{
			SUMint /= p_num;
			h /= p_num;
			FTX4 = FTX2 / p_num - SUMint * h;  //Sxy
			FTX3 = FTX / p_num - SUMint * SUMint;   //Sxx
		}
		else   //防止死机
		{
			*HR = 0;
			*SBP = 0;
			*BP_CONFIDENCE = 10;
			*DBP = 0;
			*ABP = 0;

			*XLBQ_FLAG = 0;


			tingzhi_ganrao = 0;
			dbp = 0;
			sbp = 0;
			MAX_numt = 0;
			peak_max_id = 1;
			numt = 0;
			avg_max_id = 0;
			avg_max_y = 0;
			f_hr = 0;
			stop_flag = 0;
			peak_now = 0;
			peak_need = 0;
			bp_now = 0;
			bp_max = 0;
			*STOP_FLAG = 1;
			return;
		}


		//计算趋势的斜率
		if (FTX3 != 0)
		{
			FTX = FTX4 / FTX3;  //grad
		}
		else  //防止死机
		{
			*HR = 0;
			*SBP = 0;
			*BP_CONFIDENCE = 10;
			*DBP = 0;
			*ABP = 0;

			*XLBQ_FLAG = 0;


			tingzhi_ganrao = 0;
			dbp = 0;
			sbp = 0;
			MAX_numt = 0;
			peak_max_id = 1;
			numt = 0;
			avg_max_id = 0;
			avg_max_y = 0;
			f_hr = 0;
			stop_flag = 0;
			peak_now = 0;
			peak_need = 0;
			bp_now = 0;
			bp_max = 0;
			*STOP_FLAG = 1;
			return;
		}
		

		FTX2 = (0 - FTX) * SUMint + h;  //yint




		SUMint = 0;
		h = 0;
		for (i = 0; i < p_num; i++)
		{

			SUMint = (p_data[i] - (FTX * i + FTX2) + 500 + 0.5);// 这里要看一下我们的数据中+500后有没有负数，有的话我们应该整体抬升，否则负数非常影响后面的程序和算法。

			if (SUMint < h && SUMint <= 0)
			{
				h = SUMint;
			}
		}
		for (i = 0; i < p_num; i++)
		{
			p_data[i] = (p_data[i] - (FTX * i + FTX2) + 500 + 0.5 - h + 1); // 去除直线趋势之后，数据会分布在0附近，我们+500，让其分布在500附近。
		}
		////以上代码是去除直线趋势代码


		//以下代码是平滑滤波代码，平滑滤波的主要原因是我们为了更好的寻找上升沿和下降沿，将数据进行平滑是很有效的去除毛刺的方式。
		
		// 参数一:U  滤波次数，这里进行两次遍历
		const uint8_t U = 2; //30   
		// 参数二:N            每次遍历取后面四个数的平均值
		const uint8_t N = 4; //取连续数长度   16
		uint16_t A_aver = 0, A_count = 0;
		// U次迭代平滑处理
		h = 0;

		for (h = 0; h < U; h++)
		{
			for (i = 0; i < p_num; i++)
			{
				A_aver = A_count = 0;
				if (*(p_data + i) > 0)
				{
					for (j = i; j < i + N && j < p_num; j++)
					{
						if (*(p_data + j) > 0)
						{
							A_aver += *(p_data + j);
							A_count++;
						}
					}
					if (A_count>0)
					{
						A_aver /= A_count;
						*(p_data + i) = A_aver;
					}
				}
			}
		}
		//以上代码是平滑滤波的代码
		
		//接下来进行上升沿和下降沿的查找，峰值点的查找。这点和停止打压部分的峰值点查找几乎没有区别。
		uint16_t down_count = 0, up_count = 0;
		uint8_t d_nn = 0, u_nn = 0;
		uint16_t Down_arr[121][3] = { 0 };
		uint16_t Up_arr[121][3] = { 0 };
		uint16_t down_count_plat = 0, up_count_plat = 0;
		for (i = 0; i < p_num - 1; i++)
		{
			if (d_nn < 120 && u_nn < 120)
			{
				if (p_data[i] >= p_data[i + 1])
				{
					down_count++;
					if (p_data[i] == p_data[i + 1]) //平台期
					{
						down_count_plat++;

					}

					if (i == p_num - 2 && down_count > down_count_plat) // 把最后一个包含进去
					{
						Down_arr[d_nn][0] = down_count;
						Down_arr[d_nn][1] = i;          //下降沿的终点横坐标 
						Down_arr[d_nn][2] = p_data[i];  //下降沿的终点纵坐标
						d_nn++;
						down_count = 0;
						down_count_plat = 0;
					}
				}
				else
				{
					if (down_count > 5 && (down_count > down_count_plat))
					{
						Down_arr[d_nn][0] = down_count;
						Down_arr[d_nn][1] = i;          //下降沿的终点横坐标 
						Down_arr[d_nn][2] = p_data[i];  //下降沿的终点纵坐标
						d_nn++;
						down_count = 0;
						down_count_plat = 0;
					}
					down_count = 0;
					down_count_plat = 0;
				}

				if (p_data[i] <= p_data[i + 1])
				{
					up_count++;
					if (p_data[i] == p_data[i + 1]) //平台期
					{
						up_count_plat++;

					}
					if (i == p_num - 2 && up_count > up_count_plat) // 把最后一个包含进去
					{
						Up_arr[u_nn][0] = up_count;
						Up_arr[u_nn][1] = i;          //上升沿的终点横坐标
						Up_arr[u_nn][2] = p_data[i];  //上升沿终点纵坐标
						u_nn++;
						up_count = 0;
						up_count_plat = 0;
					}
				}
				else
				{
					if (up_count > 5 && (up_count > up_count_plat))
					{
						Up_arr[u_nn][0] = up_count;
						Up_arr[u_nn][1] = i;          //上升沿的终点横坐标
						Up_arr[u_nn][2] = p_data[i];  //上升沿终点纵坐标
						u_nn++;
						up_count = 0;
						up_count_plat = 0;
					}

					up_count = 0;
					up_count_plat = 0;
				}
			}
		}
		uint16_t max_nn = 0;
		max_nn = u_nn > d_nn ? u_nn : d_nn;


		if (max_nn >= 121)  //防止数组越界
		{
			*HR = 0;
			*SBP = 0;
			*BP_CONFIDENCE = 10;
			*DBP = 0;
			*ABP = 0;

			*XLBQ_FLAG = 0;


			tingzhi_ganrao = 0;
			dbp = 0;
			sbp = 0;
			MAX_numt = 0;
			peak_max_id = 1;
			numt = 0;
			avg_max_id = 0;
			avg_max_y = 0;
			f_hr = 0;
			stop_flag = 0;
			peak_now = 0;
			peak_need = 0;
			bp_now = 0;
			bp_max = 0;
			*STOP_FLAG = 1;
			return;
		}
		uint16_t uu = 0;
		uint16_t p_peak[121][3] = { 0 }; //波峰数组：记录波峰的横纵坐标和置信度
		uint16_t p_valley[121][2] = { 0 }; //波谷数组：记录一个波峰的前一个波谷横纵坐标和后一个波谷的横纵坐标
		uint16_t next_down_x = 0, next_down_y = 0;
		uint16_t before_down_x = 0, before_down_y = 0;
		for (i = 0; i < u_nn; i++)
		{
			for (j = 1; j < d_nn; j++)
			{

				if (Down_arr[j][1] < Up_arr[i][1])
				{
					continue;

				}
				else
				{
					next_down_x = Down_arr[j][1];
					next_down_y = Down_arr[j][2];
					before_down_x = Down_arr[j - 1][1];
					before_down_y = Down_arr[j - 1][2];
					break;
				}
			}
			///此处和停止打压部分的寻找峰值点略有区别
			if (i != u_nn - 1)
			{
				if ((abs_int(Up_arr[i][2] - next_down_y) > mjudge_peak_height && abs_int(Up_arr[i][2] - next_down_y) < 4000 && abs_int(Up_arr[i][2] - before_down_y) > mjudge_peak_height && abs_int(Up_arr[i][2] - before_down_y) < 4000) && (abs_int(Up_arr[i][1] - before_down_x) > 10 && abs_int(Up_arr[i][1] - next_down_x) > 10))
				{
					//2.14日修改，把这里1500的阈值进行扩大到4000，其实这里可以没有必要进行最大阈值的限制，不影响以前完整的算法。

					p_peak[uu][0] = Up_arr[i][1];
					p_peak[uu][1] = Up_arr[i][2];
					/*p_valley[uu][0] = before_down_x;*/
					p_valley[uu][0] = before_down_y;
					/*p_valley[uu][2] = next_down_x;*/
					p_valley[uu][1] = next_down_y;
					uu++;
				}
				else if (abs_int(Up_arr[i][2] - next_down_y) > 1500 || (abs_int(Up_arr[i][2] - before_down_y) > 1500))
				{
					//p_peak[uu][0] = Up_arr[i][1];
					//p_peak[uu][1] = Up_arr[i][2];
					//p_valley[uu][0] = before_down_x;
					//p_valley[uu][1] = before_down_y;
					//p_valley[uu][2] = next_down_x;
					//p_valley[uu][3] = next_down_y;
					//uu++;
					BP_CONFIDENCE_PRE = BP_CONFIDENCE_PRE + 0;   //若左右高度过高，视为置信度减少   ， 2.14日修改，这里改成了0，以前是8. 将BP_CONFIDENCE_PRE + 8放在了下文应该需要的地方
				}
			}
			else if ((abs_int(Up_arr[i][1] - before_down_x) > 10 && abs_int(Up_arr[i][1] - next_down_x) > 10))
			{
				p_peak[uu][0] = Up_arr[i][1];  //波峰横坐标
				p_peak[uu][1] = Up_arr[i][2];  //波峰纵坐标
				//p_valley[uu][0] = before_down_x; /////////////////  左波谷横坐标
				p_valley[uu][0] = before_down_y;                  //左波谷纵坐标
				//p_valley[uu][2] = next_down_x; /////////////////右波谷横坐标
				p_valley[uu][1] = next_down_y;                //右波谷纵坐标

				//p_peak[uu][1] = Up_arr[i][2] - p_valley[uu][0];
				uu++;
			}

		}

		//粗略置信度，置信度的初步设置，以脉搏波左高度和右高度的差值和高度作为判断依据
		SUMint = 0;
		h = 0;
		for (i = 0; i < uu; i++)
		{
			if (abs_int((p_peak[i][1] - p_valley[i][0]) - (p_peak[i][1] - p_valley[i][1])) < 200 && p_peak[i][1] - p_valley[i][0] < 1300 && p_peak[i][1] - p_valley[i][1] < 1300) //左高度减去右高度
			{
				p_peak[i][2] = 1;
			}
			else
			{
				p_peak[i][2] = 0;   
			//	tingzhi_ganrao++;   //这里我们要将其视作部分干扰         //2.14日修改，将其删除，放在下文修改的地方
			}

		}

	//2020.2.14更改内容
	//更改原因：对于一些老人的数据，其脉搏波高度非常大，但却是正常波形。所以此处判断置信度时不能只用简单的高度
	//以下为2020.2.14新增，目的是将这些老人比较大的脉搏波也识别为正常波形，尽量使改动不影响以前的算法
	//所以这种情况我们应该将其视为特殊情况，筛选出来，特殊对待。我们设置置信度的概念是为了防止干扰，老人的波形和干扰的区别是，干扰时突然出现的，老人的波形却是逐渐上升的。
	////特殊情况筛选：
		for (i = 0; i < uu; i++)
		{
			if (p_peak[i][2] == 0) //针对置信度为0的波形
			{
				if (i > 4) //小于4个脉搏波肯定有问题，不用修改，会在后面判断时被检测到。
				{
					//判断其连续性
					if (abs_int(p_peak[i][0] - p_peak[i - 1][0]) < abs_int((p_peak[i - 1][0] - p_peak[i - 2][0] + p_peak[i - 2][0] - p_peak[i - 3][0] + p_peak[i - 3][0] - p_peak[i - 4][0]) / 3) + 50)
					{
						p_peak[i][2] = 1;  //若是连续上升或者下降在一定范围，而不是突然出现的，则认为是没有问题，重新置1.
					}
					else
					{
						BP_CONFIDENCE_PRE = BP_CONFIDENCE_PRE + 8;     //这个else才是真正的干扰，将BP_CONFIDENCE_PRE+8 放在这里
						tingzhi_ganrao++;  //上文的放在这里； 
					}
				}
			}
		}
		//以上为2.14日代码修改，应该适用于老年人数据，同时也不影响年轻人的数据。



	//	FILE* fw;
	//errno_t err3;
	//err3 = fopen_s(&fw, "D:\\HM117\\117DATA\\1136Cpeak.txt", "wb");
	//for (i = 0; i < uu; i++)
	//{
	//
	//	fprintf(fw, "%d %d\r\n", p_peak[i][0],p_peak[i][1]);
	//}
	//fclose(fw);

	//去干扰 ,查找有效脉搏波。去除异常波形段
	//首先从形态上定义一个脉搏波是有效的。有效脉搏波的定义为：高度不能太高或者太低在1000左右，不能太宽或者太窄，在100左右
	
		if (uu > 100 || uu <= 6)  //防止死机
		{
			*HR = 0;
			*SBP = 0;
			*BP_CONFIDENCE = 10;
			*DBP = 0;
			*ABP = 0;

			*XLBQ_FLAG = 0;


			tingzhi_ganrao = 0;
			dbp = 0;
			sbp = 0;
			MAX_numt = 0;
			peak_max_id = 1;
			numt = 0;
			avg_max_id = 0;
			avg_max_y = 0;
			f_hr = 0;
			stop_flag = 0;
			peak_now = 0;
			peak_need = 0;
			bp_now = 0;
			bp_max = 0;
			*STOP_FLAG = 1;
			return;
		}
		uint16_t ppp_quality[100][3] = { 0 }; //4列分别为：左高度和右高度和置信度，该数组是用来存放峰值点的信息
		int16_t p_quality[100][1] = { 0 };   //该数组是用来存放各峰值点的置信度信息
		int8_t confidence = 0;
		//uint16_t quality_one = 200, quality_two = 500, quality_three = 1000; //这个不能是固定值，应该是自适应的。
		//uint16_t quality_diff = 250;
		for (i = 0; i < uu; i++)        //数组的幅值
		{
			ppp_quality[i][0] = p_peak[i][1] - p_valley[i][0];    //左高度
			ppp_quality[i][1] = p_peak[i][1] - p_valley[i][1];     //右高度
			ppp_quality[i][2] = ppp_quality[i][1];        //置信度列  暂时先赋值为左高度
		}
		//对ppp_quality进行从大到小排序
		for (i = 0; i < uu; i++)  // 对均值进行冒泡从大到小的排序
		{
			for (j = 0; j < uu - i - 1; j++)
			{
				if (ppp_quality[j][2] < ppp_quality[j+1][2])    //如果前一个元素小于后一个元素
				{
					ITX = 0;        //临时变量  （真正数值）
					ITX = ppp_quality[j][2];
					ppp_quality[j][2] = ppp_quality[j + 1][2]; //大的元素到前一个位置
					ppp_quality[j + 1][2] = ITX;   //小的元素到后一个位置
				}
			}
		}
		//这里主要是判断前六大的波峰的均值是否太大，是前期干扰。（可选）
		ITX = 6;
		SUMint = 0;
		for (i = 1; i < ITX; i++)
		{
			SUMint = SUMint + ppp_quality[i][2];
		}
		SUMint = SUMint / (ITX -1 );
		//2.14日修改，将其删除，这里的前六大波形高度，不应该受到限制。BP_CONFIDENCE_PRE不应该被增加。
		//if (SUMint > 1500)
		//{
		//	BP_CONFIDENCE_PRE = BP_CONFIDENCE_PRE + 20;
		//}
		BP_CONFIDENCE_PRE = BP_CONFIDENCE_PRE + tingzhi_ganrao;
		
		for (i = 0; i < uu; i++)  //这个for循环是用来赋值p_quality置信度数组信息的，这个p_quality很重要
		{
			if (ppp_quality[i][0] < SUMint - 800 || ppp_quality[i][1] < SUMint - 800) //左右高度过小
			{
				confidence = -1;
			}
			else if (ppp_quality[i][0] < SUMint - 500 || ppp_quality[i][1] < SUMint - 500) //左右高度一般
			{
				confidence = 0;
			}
			else if (ppp_quality[i][0] < SUMint + 300 || ppp_quality[i][1] < SUMint + 300) //左右高度较好
			{
				confidence = 1;
			}
			else
			{
				confidence = -1;   //左右高度太大
			}

			if (abs_int(ppp_quality[i][0] - ppp_quality[i][1]) < 250)  //左右谷底点的高度之差
			{
				confidence = confidence + 1;
			}
			else if(abs_int(ppp_quality[i][0] - ppp_quality[i][1]) < 450)  //左右谷底点的高度之差
			{
				confidence = confidence - 1;
			}
			else
			{
				confidence = confidence - 2;
			}
			p_quality[i][0] = confidence;  //置信度赋值到p_quality

		}

	
		for (i = 0; i < uu - 2; i++) //置信度平滑，去异常，对置信度数组进行微调
		{
			if (abs(p_quality[i][0] - p_quality[i + 1][0] > 1))
			{
				if (p_quality[i + 2][0] == p_quality[i][0])
				{
					p_quality[i + 1][0] = (p_quality[i][0] + p_quality[i + 1][0]) / 2;
				}
			}
		}
		uint8_t quality_negtive = 0;  //记录较差置信度的个数
		uint8_t quality_postive = 0;  //记录较好的置信度的个数
		ITX = 0;
		for (i = 0; i < uu - 1; i++) //存在连续置信度较高的波形，增加其置信度
		{
			if (p_quality[i][0] >= 2)
			{
			
				if (p_quality[i + 1][0] >= 2)
				{
					p_quality[i + 1][0] = p_quality[i + 1][0] + 1;  //
					if (abs_int(i - uu / 2) <=1)
					{
						p_quality[i + 1][0]++;
					}
				}
				ITX++;
			}
			if (p_quality[i][0] < 0)
			{
				quality_negtive++;  //算一下置信度小于0的个数
			}
			if (p_quality[i][0] >= 2)
			{
				quality_postive++;  //算一下置信度大于等于2 的个数
			}
		}
		///////////////////////////////////////////////////////////


		//根据有效脉搏波，定义有效波峰段：以6个波峰为窗口，寻找方差最小的6个波峰单位
		//解释：我们以6个波峰为单位窗口，滑动计算，若有7个波峰，则有2个窗口，若8个波峰，则有3个窗口，以此类推。
		const uint8_t peak_window = 6;
		SUMint = 0;
		int16_t peak_Mean[100][2] = { 0 };   //注意这里的类型为int16_t ，是可能存在负数值得
		int16_t peak_Var[100][2] = { 0 };
		uint8_t windows = 0;      //windows是记录我们的窗口数量



		//求数据平均值
		for (i = 0; i < uu - peak_window; i++)
		{
			for (j= 0; j < peak_window; j++)
			{
				SUMint = SUMint + p_peak[i + j][1];
			}
			peak_Mean[i][0] = SUMint / peak_window;        //对数组peak_mean赋值，这个数组内的数字含义是每6个相邻波峰的平均高度值
			peak_Mean[i][1] = i;
			SUMint = 0;                          
			windows++;                                   //同时，记录窗口数量
		}
		if (windows == 0)   //防止死机
		{		
			windows = 1;
			*HR = 0;
			*SBP = 0;
			*BP_CONFIDENCE = 10;
			*DBP = 0;
			*ABP = 0;

			*XLBQ_FLAG = 0;


			tingzhi_ganrao = 0;
			dbp = 0;
			sbp = 0;
			MAX_numt = 0;
			peak_max_id = 1;
			numt = 0;
			avg_max_id = 0;
			avg_max_y = 0;
			f_hr = 0;
			stop_flag = 0;
			peak_now = 0;
			peak_need = 0;
			bp_now = 0;
			bp_max = 0;
			*STOP_FLAG = 1;
			return;

		}
		SUMint = 0;
		//求数据标准差
		for (i = 0; i < uu - peak_window; i++)
		{
			for (j= 0; j < peak_window; j++)
			{
				SUMint = SUMint + (p_peak[i + j][1] - peak_Mean[i][0]) * (p_peak[i + j][1] - peak_Mean[i][0]);
			}
			peak_Var[i][0] = sqrt(SUMint / peak_window);     //对数组peak_var赋值，这个数组内的数字含义是每6个相邻波峰的高度值的标准差
			peak_Var[i][1] = i;
			SUMint = 0;
		}


		//理想的最大峰值点定义：均值偏大，方差偏小.    两者优先看哪个？  优先看均值




		//////均值排序
		for (i = 0; i < windows; i++)  // 对均值进行冒泡从大到小的排序
		{
			for (j= 0; j < windows - i - 1; j++)
			{
				if (peak_Mean[j][0] < peak_Mean[j + 1][0])    //如果前一个元素小于后一个元素
				{
					ITX = 0;        //临时变量  （真正数值）
					ITX = peak_Mean[j][0];
					peak_Mean[j][0] = peak_Mean[j + 1][0]; //大的元素到前一个位置
					peak_Mean[j + 1][0] = ITX;   //小的元素到后一个位置

					ITX = 0;        //临时变量 (下标)
					ITX = peak_Mean[j][1];
					peak_Mean[j][1] = peak_Mean[j + 1][1]; //大的元素到前一个位置
					peak_Mean[j + 1][1] = ITX;   //小的元素到后一个位置
				}
			}
		}
		////方差排序
		for (i = 0; i < windows; i++)  // 对方差 进行冒泡从小到大的排序
		{
			for (j= 0; j < windows - i - 1; j++)
			{
				if (peak_Var[j][0] > peak_Var[j + 1][0])    //如果前一个元素大于后一个元素
				{
					ITX = 0;        //临时变量  （真正数值）
					ITX = peak_Var[j][0];
					peak_Var[j][0] = peak_Var[j + 1][0]; //小的元素到前一个位置
					peak_Var[j + 1][0] = ITX;   //大的元素到后一个位置

					ITX = 0;        //临时变量 (下标)
					ITX = peak_Var[j][1];
					peak_Var[j][1] = peak_Var[j + 1][1]; //小的元素到前一个位置
					peak_Var[j + 1][1] = ITX;   //大的元素到后一个位置
				}
			}
		}

		/////求置信度的平均值
		SUMint = 0;
		float peak_quality[100][2] = { 0 };         //这个数组也是置信度数组，但是与p_quality不同。1.该数组类型为float。2.该数组长度要比其小6个，其含义是每相邻六个波峰计算出一个置信度，遍历所有波峰
		for (i = 0; i < uu - peak_window; i++)
		{
			for (j= 0; j < peak_window; j++)
			{
				SUMint = SUMint + p_quality[i + j][0];  //是在p_quality数组的基础上，每6个波峰取一个平均值，这样，peak_mean , peak_var, peak_quality三个数组长度相同
			}
			peak_quality[i][0] = (float)SUMint / (float)peak_window;
			peak_quality[i][1] = i;
			SUMint = 0;
		}
		//对peak_quality置信度排序
		for (i = 0; i < windows; i++)  // 对均值进行冒泡从大到小的排序
		{
			for (j= 0; j < windows - i - 1; j++)
			{
				if (peak_quality[j][0] < peak_quality[j + 1][0])    //如果前一个元素小于后一个元素
				{
					FTX = 0;        //临时变量  （真正数值）
					FTX = peak_quality[j][0];
					peak_quality[j][0] = peak_quality[j + 1][0]; //大的元素到前一个位置
					peak_quality[j + 1][0] = FTX;   //小的元素到后一个位置

					FTX = 0;        //临时变量 (下标)
					FTX = peak_quality[j][1];
					peak_quality[j][1] = peak_quality[j + 1][1]; //大的元素到前一个位置
					peak_quality[j + 1][1] = FTX;   //小的元素到后一个位置
				}
			}
		}
		
		//下面根据peak_quality数组的数值，对终置信度BP_Confidence进行预设置
		int16_t BP_Confidence = 0;  //设置最终BP结果的置信度，计做终置信度
		/// 根据均值和方差和置信度来循环选择
		if (peak_quality[0][0] < 0.5) //如果最大的置信度不超过1，说明该波形极乱，没有连续6个以上的有效波，判定为干扰波，应设置很低的终置信度。
		{
			BP_Confidence = peak_quality[0][0] * 100;
			BP_Confidence = BP_Confidence - quality_negtive * 1 + quality_postive;

		}
		else if (peak_quality[0][0] < 1)
		{
			BP_Confidence = (peak_quality[0][0] - 0.5) / 0.5 * 35 + 50;
			BP_Confidence = BP_Confidence - quality_negtive * 1 + quality_postive * 0.5;
		}
		else if (peak_quality[0][0] < 2)
		{
			BP_Confidence = (peak_quality[0][0] - 1) / 1 * 9 + 90;
			BP_Confidence = BP_Confidence - quality_negtive * 1 + quality_postive * 0.5;
		}
		else
		{
			BP_Confidence = 99;
			BP_Confidence = BP_Confidence - quality_negtive * 1 + quality_postive * 0.5;
		}
		if (BP_Confidence < 10) { BP_Confidence = 10; }               //终置信度的范围设置
		if (BP_Confidence > 100) { BP_Confidence = 99; }

		////接下来选择最佳的最大峰值点,  输出为最大峰值点的下标
		//////////////////*********选择最大峰值点
		//PEAK_Select(p_peak, peak_quality, peak_Mean, peak_Var, windows, PEAK_OPT, &Return_OPT, &peak_max_id);
		uint8_t mean_id = 0, var_id = 0, true_id = 0;
		true_id = peak_Mean[0][1];//定义：选择平均值还是方差最大的（这里第一次默认是平均值）
		uint8_t sel_flag = 0;//定义：标志位：0以平均值作为选择标准，1以方差作为选择标准，默认平均值0
		//求方差的方差，作为是否增加选择次数的阈值
		uint16_t var_mean = 0, var_var = 0;
		SUMint = 0;
		for (i = 0; i < windows; i++)              //计算方差数组peak_Var的均值
		{
			SUMint = SUMint + peak_Var[i][0];
		}
		var_mean = SUMint / windows;
		SUMint = 0;
		for (i = 0; i < windows; i++)              //计算方差数组peak_Var的方差
		{
			SUMint = SUMint + (peak_Var[i][0] - var_mean) * (peak_Var[i][0] - var_mean);
		}
		var_var = sqrt(SUMint / windows);
		if (var_var > 30) // 如果整体数据很乱，则以方差作为选择标准
		{

			true_id = peak_Var[0][1];
			sel_flag = 1;
		}


		/*uint8_t mean_yuzhi = windows - 6;*/
		/////////////////////////////////////////////
		//// 直接在置信度的前六名中对比均值和方差，直接选择出最优解
		//根据sel_flag  决定均值优先或者方差优先  sel_flag =0均值优先  sel_flag =1方差优先  优先表示加大权重
		float All_quality[6][1] = { 0 };
		FTX = 100;
		if (sel_flag == 0)  //权重设置
		{
			FTX3 = 0.7;
			FTX2 = 0.3;
		}
		else
		{
			FTX3 = 0.3;
			FTX2 = 0.7;
		}

		for (i = 0; i < 6; i++)
		{
			for (j= 0; j < windows; j++)
			{
				if (peak_Var[j][1] == (uint16_t)peak_quality[i][1])
				{
					var_id = j;
				}
				if (peak_Mean[j][1] == (uint16_t)peak_quality[i][1])
				{
					mean_id = j;
				}
			}
			//All_quality[i][0] = peak_quality[i][1];    //原始位置
			//All_quality[i][1] = (float)i;            //当前置信度排序后位置
			//All_quality[i][2] = (float)mean_id;      //当前均值排序后的位置
			//All_quality[i][3] = (float)var_id;       // 当前方差排序后的位置
			All_quality[i][0] = (float)mean_id / 6 * FTX3 + (float)var_id / 6 * FTX2 + (float)i * 0.15; //均值和方差排序后的加权

			if (All_quality[i][0] < FTX)           //计算出置信度的最大峰值点
			{
				FTX = All_quality[i][0];
				true_id = (uint16_t)peak_quality[i][1];
			}
		}


		ITX = 0;  //根据置信度的最大峰值点确定真正的最大峰值点
		for (i = true_id + 1; i < true_id + 6; i++)
		{
			if (p_peak[i][1] > ITX && p_peak[i][2] == 1)
			{
				ITX = p_peak[i][1];
				peak_max_id = i;
			}
		}

	//以下进行心率的求解，基本思想是在最大连续置信度波峰，较好的波峰里面计算其距离的间隔
		utx = 0; //记录连续置信度长度
		uint16_t utx2 = 0; //记录最长连续置信度的长度
		SUMint = 0;
		ITX = 0;
		ITX2 = 0;
		h = 0;
		uint16_t fr_start = 0;//记录连续置信度开始的下标
		for (i = 0; i < uu; i++)
		{
			if (p_peak[i][2] == 1)
			{
				utx++;
			}
			else
			{
				if (utx2 > utx)
				{
					utx2 = utx2;
				}
				else
				{
					utx2 = utx;
					fr_start = i - utx2;
				}
				utx = 0;
			}

			if (i == uu - 1)
			{
				if (utx2 > utx)
				{
					utx2 = utx2;
				}
				else
				{
					utx2 = utx;
					fr_start = i + 1 - utx2;
				}
				utx = 0;
			}
		}
		uint16_t utx3 = 0; //有效的长度
		if (utx2 >= 1 && fr_start >= 0 && fr_start < 100)
		{
			for (i = fr_start; i < fr_start + utx2 - 1; i++)
			{
				if (p_peak[i + 1][0] - p_peak[i][0] >= 85 && p_peak[i + 1][0] - p_peak[i][0] < 150)
				{
					SUMint = SUMint + (p_peak[i + 1][0] - p_peak[i][0]);
					utx3++;
				}
			}
			if (utx3 >= 1)
			{
				ITX = SUMint / utx3;
			}
			else
			{
				ITX = 128 - uu;
			}
		}
		else
		{
			ITX = 128 - uu;
		}

		if (ITX == 0)  //默认间隔
		{
			ITX = 128;
		}
		f_hr = 128 * 60 / ITX;  //心率计算
		if (f_hr <= 0)
		{
			f_hr = 40;
		}
		utx = 0;
		SUMint = 0;
		ITX = 0;
		ITX2 = 0;
	

		/////////判断心律不齐////////////////////
		//基本思想是：选择置信度较高的一段波形，计算该段波形的平均RR间隔（波峰之间的间隔），并检测是否存在偏离平均RR间隔较大的点
		/*uint16_t ADR[50][2] = { 0 };*/
		/*uint16_t ADR_id = 0;*/   
		//用peak_mean代替ADR，以前有个ADR数组，用来存放峰值间隔信息的，有peak_mean数组闲置且后续无用，故用peak_mean表示ADR了。
		for (i = 0; i < uu - 1; i++)
		{
			peak_Mean[i][0] = p_peak[i + 1][0] - p_peak[i][0];
			peak_Mean[i][1] = p_quality[i][0];
			/*ADR_id++;*/

		}
		uint16_t L_qual_id = 100, R_qual_id = 0;  //变量，记录置信度较高的波峰段的起点（L_qual_id）和终点（R_qual_id）
		uint16_t XLBQ = 0;                        //心率不齐的标志位
		uint16_t XLBQ_num = 0;
		/*uint16_t XLBQ_id = 0;*/
		SUMint = 0;
		ITX = 0;
		float RR_BL = 0.0; //RR间期的比例
		for (i = 0; i < uu - peak_window; i++)  //选置信度较高的一段波形决定
		{
			if (peak_quality[i][0] >= 1)
			{
				if (peak_quality[i][1] < L_qual_id)
				{
					L_qual_id = peak_quality[i][1];
				}
				if (peak_quality[i][1] > R_qual_id)
				{
					R_qual_id = peak_quality[i][1];
				}
			}
		}
		if ((L_qual_id == 100 && R_qual_id == 0) || (L_qual_id > R_qual_id) || BP_Confidence < 50)
		{
			XLBQ = 0;     //波形太乱无法判断，默认心率是齐的
		}
		else
		{
			for (i = L_qual_id; i < R_qual_id + 6; i++)   //该for循环用来计算平均RR间隔
			{
				SUMint = SUMint + peak_Mean[i][0];
			}
			ITX = SUMint / (R_qual_id + 6 - L_qual_id);

			if (ITX <= 0)  //防止死机
			{
				*HR = 0;
				*SBP = 0;
				*BP_CONFIDENCE = 10;
				*DBP = 0;
				*ABP = 0;

				*XLBQ_FLAG = 0;


				tingzhi_ganrao = 0;
				dbp = 0;
				sbp = 0;
				MAX_numt = 0;
				peak_max_id = 1;
				numt = 0;
				avg_max_id = 0;
				avg_max_y = 0;
				f_hr = 0;
				stop_flag = 0;
				peak_now = 0;
				peak_need = 0;
				bp_now = 0;
				bp_max = 0;
				*STOP_FLAG = 1;
				return;
			}

			for (i = L_qual_id; i < R_qual_id + 6; i++)
			{
				if (peak_Mean[i][1] >= 0 && i< uu)
				{
					RR_BL = ((float)peak_Mean[i][0] / ITX);
					if ( (RR_BL > 0.7 && RR_BL<1.2) || (RR_BL > 1.7 && RR_BL < 2.2 )|| (RR_BL > 2.7 && RR_BL < 3.2))//有时候漏了一个或几个点，则应该是2倍或n倍关系
					{
						XLBQ = 0;
					}
					else
					{
						XLBQ_num++;
						if (XLBQ_num > 2)
						{
							XLBQ = 1;       //若存在疑似心律不齐，改变标志位
							break;         
						}
						/*XLBQ_id = i;*/
					}
				}

			}
		}
		//////////////////////////////////////

		//////////////筛选后续需要被拟合的峰值点//////
		////基本思想：对于一次测压过程，我们不需要全部波峰点，只需要有效的部分即可。为什么？这和我们后面的利用拟合算法计算血压有关。太长的数据不适合做曲线拟合
		ITX= ((f_hr + 5) / 10) - 2; //含义：至少多少个点再停止搜索，和心率有关
		uint16_t start_Tpeak = 0, end_Tpeak = 0;  //有用峰值点的起点和终点
		ITX2= 0;
		for (i = peak_max_id - 1; i >= 0; i--)  //从最大峰值点向左寻找起点
		{
			if (peak_max_id <= 5) { start_Tpeak = 0; break; }

			if (p_peak[i + 1][1] - p_peak[i][1] > 0 && p_peak[i + 1][1] > p_peak[peak_max_id][1] * 0.70)
			{

				start_Tpeak = i + 1;
				continue;
			}
			else
			{

				if (i > peak_max_id - ITX)    //至少ITX个峰值点再去停止
				{
					start_Tpeak = i + 1;
					continue;
				}
				else
				{
					if (p_peak[i + 1][1] > p_peak[peak_max_id][1] * 0.70 && i > peak_max_id - 6)   //低于最大值点一定值再去停止
					{
						start_Tpeak = i + 1;
						continue;
					}
					else
					{
						start_Tpeak = i + 1;
						ITX2++;
						if (ITX2 >= 2)
						{
							break;
						}
					}
				}
				if (i < peak_max_id - (ITX + 2)) //不能超过(ITX+2)个
				{
					start_Tpeak = i + 1;
					break;
				}
			}
		}
		for (i = peak_max_id; i < uu - 1; i++)  //从最大峰值点向右寻找终点
		{
			if (p_peak[i][1] - p_peak[i + 1][1] > 0 )
			{
				end_Tpeak = i + 1;
				continue;
			}
			else
			{
				if (i + 1 < peak_max_id + ITX  || i + 1 < uu - 2)    //至少ITX个峰值点再去停止
				{
					end_Tpeak = i + 1;
					continue;
				}
				else
				{
					if (p_peak[i + 1][1] > p_peak[peak_max_id][1] * 0.7)   //低于最大值点一定值再去停止
					{
						end_Tpeak = i + 1;
						continue;
					}
					else
					{
						end_Tpeak = i + 1;
						break;
					}

				}
				//if (i > peak_max_id + 7) //不能超过6个
				//{
				//	end_Tpeak = i ;
				//	break;
				//}
			}
		}

		//////////***************////////////////
	

		int16_t T_peak[50][2] = { 0 };
		uint16_t T_peak_x = 0;
		uint8_t T_id = 0;
		if (end_Tpeak - start_Tpeak > 50 || end_Tpeak - start_Tpeak <= 1)  //防止死机
		{
			*HR = 0;
			*SBP = 0;
			*BP_CONFIDENCE = 10;
			*DBP = 0;
			*ABP = 0;

			*XLBQ_FLAG = 0;


			tingzhi_ganrao = 0;
			dbp = 0;
			sbp = 0;
			MAX_numt = 0;
			peak_max_id = 1;
			numt = 0;
			avg_max_id = 0;
			avg_max_y = 0;
			f_hr = 0;
			stop_flag = 0;
			peak_now = 0;
			peak_need = 0;
			bp_now = 0;
			bp_max = 0;
			*STOP_FLAG = 1;
			return;
		}

		ITX  = 0;
		////////我们将上述有用的峰值点放入到新的数组T_peak ， 存放的形式是峰值点的左高度
		for (i = start_Tpeak; i <= end_Tpeak; i++)  
		{
			if (i == end_Tpeak)
			{
				T_peak[T_id][0] = p_peak[i][0];
				T_peak[T_id][1] = p_peak[i][1] - p_valley[i][0];
				T_id++;
			}
			else
			{
				T_peak[T_id][0] = p_peak[i][0];
				if (p_peak[i][1] - p_valley[i][0] > p_peak[i][1] - p_valley[i][1])
				{
					T_peak[T_id][1] = p_peak[i][1] - p_valley[i][0];  //选择左高度存放（有试过右高度，左右平均高度，最终选择左高度）
				}
				else
				{
					//T_peak[T_id][1] = (p_peak[i][1] - p_valley[i][0] + p_peak[i][1] - p_valley[i][1])/2;
					T_peak[T_id][1] = p_peak[i][1] - p_valley[i][0];
				}

				T_id++;
			}


			//在新的T_peak中，重新确定最大峰值点.（peak_max_id是之前绝对高度的最大值，这里的T_peak_x是相对高度（左高度）的最大值，也是我们后续计算的最大值点）
			if (T_peak[T_id - 1][1] > ITX && (i <= peak_max_id + 2) && (i >= peak_max_id - 2))
			{
				ITX = T_peak[T_id - 1][1];
				T_peak_x = i - start_Tpeak;
			}

		}
		if (T_peak_x >= T_id || T_peak_x <= 0) //防止死机
		{
			*HR = 0;
			*SBP = 0;
			*BP_CONFIDENCE = 10;
			*DBP = 0;
			*ABP = 0;

			*XLBQ_FLAG = 0;


			tingzhi_ganrao = 0;
			dbp = 0;
			sbp = 0;
			MAX_numt = 0;
			peak_max_id = 1;
			numt = 0;
			avg_max_id = 0;
			avg_max_y = 0;
			f_hr = 0;
			stop_flag = 0;
			peak_now = 0;
			peak_need = 0;
			bp_now = 0;
			bp_max = 0;
			*STOP_FLAG = 1;
			return;
		}

		
		if (T_id<=1) //防止死机
		{
			*HR = 0;
			*SBP = 0;
			*BP_CONFIDENCE = 10;
			*DBP = 0;
			*ABP = 0;

			*XLBQ_FLAG = 0;


			tingzhi_ganrao = 0;
			dbp = 0;
			sbp = 0;
			MAX_numt = 0;
			peak_max_id = 1;
			numt = 0;
			avg_max_id = 0;
			avg_max_y = 0;
			f_hr = 0;
			stop_flag = 0;
			peak_now = 0;
			peak_need = 0;
			bp_now = 0;
			bp_max = 0;
			*STOP_FLAG = 1;
			return;
		}



		//******接下来进行去除异常的峰值点
		//基本思想是：我们所有峰值点在最大峰值点左侧应该是连续上升的，在右侧应该是连续下降的，若有异常点应该被替换
		uint16_t FF_gap_up = 0, FF_gap_down = 0; //峰值点间隔
		SUMint = 0;
		//此处是计算最大峰值点的左侧的上升部分的  平均上升间隔（即上升高度间隔的平均值，用来作为后续替换异常点时的默认值）
		for (i = 0; i < T_peak_x; i++)  //最大峰值点的左侧：上升部分   
		{
			if (T_peak[i + 1][1] - T_peak[i][1] > 0 && T_peak[i + 1][1] - T_peak[i][1] < 200)
			{
				//FF_dif_up[dif_id_up] = T_peak[i + 1][1] - T_peak[i][1];
				SUMint = SUMint + T_peak[i + 1][1] - T_peak[i][1];
				/*dif_id_up++;*/
			}
		}
		FF_gap_up = SUMint / (T_peak_x)+0.5;  //上升部分平均距离
		if (FF_gap_up <= 30 || FF_gap_up>200)
		{ 
			FF_gap_up = 30; 
		}
		SUMint = 0;
		//此处是计算最大峰值点的右侧的下降部分的  平均下降间隔（即下降高度间隔的平均值，用来作为后续替换异常点时的默认值）
		for (i = T_peak_x; i < T_id ; i++)  //最大峰值点的右侧：下降距离
		{
			if (T_peak[i + 1][1] - T_peak[i][1] < 0 && T_peak[i + 1][1] - T_peak[i][1]> -200)
			{
		/*		FF_dif_down[dif_id_down] = T_peak[i + 1][1] - T_peak[i][1];*/
				SUMint = SUMint + T_peak[i + 1][1] - T_peak[i][1];
				/*dif_id_down++;*/
			}
		}
		FF_gap_down = abs_int(SUMint / (T_id  - T_peak_x) + 0.5);  //下降部分平均距离
		if (FF_gap_down < 30 || FF_gap_up>200)
		{ 
			FF_gap_down = 30;
		}

		//int left_wrong = 0;
		//for (i = 0; i < dif_id_up; i++)//左侧去异
		//{
		//	if (FF_dif_down[i] > FF_gap_up*2 || FF_dif_down[i] < 0)
		//	{
		//		left_wrong++;
		//	}
		//	if (left_wrong == 2 || left_wrong == 4)
		//	{

		//	}
		//}

	////////////////////////////////////////左侧去异/////////////////////////////////////////////////////////
		uint8_t T_count = 0;  //异常点的个数
		//uint16_t T_peak_2[50][2] = { 0 };
		uint16_t next_Tpeak_x = 0;

		//该T_peak_2为peak_mean代替，peak_mean继续为我们使用，将重新赋值
		for (i = T_peak_x - 1; i >= 0; i--)//左侧去异
		{
			peak_Mean[i][1] = T_peak[i][1];//另外一手准备 T_peak_2 作为备用
			if (T_peak[i + 1][1] - T_peak[i][1] < 0)
			{
				if (i == T_peak_x - 1)
				{
					T_peak[i][1] = T_peak[i + 1][1] - FF_gap_up * 0.9; //最大值点的上一个点不存在 i+ 2 所以单独拎出来
				}

				if (T_peak[i + 2][1] - T_peak[i][1] > 0)
				{
					T_peak[i + 1][1] = (T_peak[i + 2][1] + T_peak[i][1]) / 2;
				}
				else  //有两种可能：这个突出的点是干扰   或者  这个突出的点是真正的趋势   应该做两手准备，看哪一种与原趋势最接近
				{
					if (next_Tpeak_x == 0)
					{
						next_Tpeak_x = i;
					}

					T_peak[i][1] = T_peak[i + 1][1] - FF_gap_up * 0.9;
					T_count++;   //做一个计数,叫做异常点的个数


				}
			}
		}

		uint8_t T_count_2 = 0; //第二手准备，情况极为特殊，异常点个数足够多才会触发
		if (T_count >= 2 && T_count > T_peak_x / 4 && T_peak[next_Tpeak_x][1] < T_peak[T_peak_x][1])//再来一次左侧去异，此时从next_peak_x开始向左去异
		{
			for (i = next_Tpeak_x - 1; i >= 0; i--)//左侧去异
			{

				if (peak_Mean[i + 1][1] - peak_Mean[i][1] < 0)
				{
					if (i == next_Tpeak_x - 1)
					{
						peak_Mean[i][1] = peak_Mean[i + 1][1] - FF_gap_up * 0.9; //最大值点的上一个点不存在 i+ 2 所以单独拎出来
					}

					if (peak_Mean[i + 2][1] - peak_Mean[i][1] > 0)
					{
						peak_Mean[i + 1][1] = (peak_Mean[i + 2][1] + peak_Mean[i][1]) / 2;
					}
					else  //
					{
						peak_Mean[i][1] = peak_Mean[i + 1][1] - FF_gap_up * 0.9;
						T_count_2++;
					}
				}
			}

			////T_peak重新赋值
			if (T_count_2 < T_count -2  && T_peak_x - next_Tpeak_x != 0)//如果第二次去异的结果比第一次的好，才更换赋值
			{
				for (i = T_peak_x - 1; i >= 0; i--)//重新赋值
				{
					int temp = (T_peak[T_peak_x][1] - peak_Mean[next_Tpeak_x][1]) / (T_peak_x - next_Tpeak_x);
					if (i > next_Tpeak_x)
					{
						T_peak[i][1] = T_peak[T_peak_x][1] - (T_peak_x - i) * temp;
					}
					else
					{
						T_peak[i][1] = peak_Mean[i][1];
					}
				}
			}
		}
		///////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////右侧去异//////////////////////////////////////////////
		//uint16_t T_peak_3[50][2] = { 0 };
		//改T_peak_3为peak_Var，这里的peak_Var继续为我们使用，并将重新赋值
		uint16_t next_Tpeak_x2 = 0;
		uint8_t T_count3 = 0;
		uint8_t T_count_4 = 0;
		for (i = T_peak_x + 1; i < T_id; i++)//右侧去异 （与左侧一样）
		{
			peak_Var[i][1] = T_peak[i][1];//另外一手准备 T_peak_2 作为备用
			if (T_peak[i - 1][1] - T_peak[i][1] < 0)
			{
				if (i == T_peak_x + 1) // 最大值点的下一个点不存在 i-2 所以单独拎出来
				{
					T_peak[i][1] = T_peak[i - 1][1] - FF_gap_down * 0.9;
				}
				if (T_peak[i - 2][1] - T_peak[i][1] > 0)
				{
					T_peak[i - 1][1] = (T_peak[i - 2][1] + T_peak[i][1]) / 2;
				}
				else
				{
					if (next_Tpeak_x2 == 0)
					{
						next_Tpeak_x2 = i;
					}
					T_peak[i][1] = T_peak[i - 1][1] - FF_gap_down * 0.9;
					T_count3++;
				}
			}
		}

		if (T_count3 >= 2 && T_count3 > ((T_id - T_peak_x) / 2 - 1) && T_peak[next_Tpeak_x2][1] < T_peak[T_peak_x][1])
		{
			for (i = next_Tpeak_x2 + 1; i < T_id; i++)//右侧去异
			{

				if (i == next_Tpeak_x2 + 1) // 最大值点的下一个点不存在 i-2 所以单独拎出来
				{
					peak_Var[i][1] = peak_Var[i - 1][1] - FF_gap_down * 0.9;
				}
				if (peak_Var[i - 2][1] - peak_Var[i][1] > 0)
				{
					peak_Var[i - 1][1] = (peak_Var[i - 2][1] + peak_Var[i][1]) / 2;
				}
				else
				{
					peak_Var[i][1] = peak_Var[i - 1][1] - FF_gap_down * 0.9;
					T_count_4++;
				}
			}

			//重新赋值
			if (T_count_4 < T_count3  && T_peak_x - next_Tpeak_x2 != 0)//如果第二次去异的结果比第一次的好，才更换赋值
			{
				for (i = T_peak_x + 1; i < T_id; i++)//重新赋值
				{
					int temp = (T_peak[T_peak_x][1] - peak_Var[next_Tpeak_x2][1]) / abs_int((T_peak_x - next_Tpeak_x2));
					if (i < next_Tpeak_x2)
					{
						T_peak[i][1] = T_peak[T_peak_x][1] - (i - T_peak_x) * temp;
					}
					else
					{
						T_peak[i][1] = peak_Var[i][1];
					}
				}
			}
		}
		//////////////////////////////////////////////////////////////////////////////
		///此处是在后续实际测试中新加入的部分，表现为有个别情况的波峰高度降不下来，呈平行于x轴的直线，这导致测压结果最后会偏大。称之为斜率过低（small_xielv）
		uint16_t small_xielv = 0;
		if (T_peak_x < end_Tpeak - 2 && end_Tpeak - 2>0) 
		{
			for (i = T_peak_x; i < end_Tpeak - 2; i++)
			{
				if (T_peak[i][1] - T_peak[i + 1][1] > 0 && T_peak[i][1] - T_peak[i + 1][1] < (T_peak[T_peak_x][1] - 500) / 500 * 20 + 20)
				{
					small_xielv++;
				}
				if (T_peak[i][1] - T_peak[i + 1][1] > 0 && T_peak[i][1] - T_peak[i + 1][1] < 20)
				{
					small_xielv++;
				}
			}
		}


		//以下代码为峰值点拟合   ： 
		//***********************三次样条插值拟合
		//三次样条拟合的效果是将各个峰值点尽量以一条平滑的曲线拟合出来。
		//代码不是亲自书写，是在原代码中修改而来的，原代码在interpfun.cpp中
		//修改内容是：原代码每隔一个点插值一次，由于内存问题，改成每隔20个点插值一次，效果上区别不大
		//本版本是为了适用于单片机内存修改过的版本，可视性非常差。
		//算法设计的初试版本，有很多不对的地方，但可视性很好在D:\HM117\HM117_NEW\HM117_NEW\HM117_NEW目录中，可以直接跑，然后在matlab中画图

		uint16_t nihe_len = T_id;
		uint16_t x1[50] = { 0 };
		uint16_t y1[50] = { 0 };
		//x1[0] = T_peak[0][0];
		//y1[0] = T_peak[0][1];
		uint16_t x1_id = 0;
		for (i = 0; i < T_id; i++)
		{
			if (T_peak[i][1] <= 20)
			{
				T_peak[i][1] = 20;
			}
			x1[x1_id] = T_peak[i][0];
			y1[x1_id] = T_peak[i][1]*20;

			if (y1[x1_id] >= 66635)
			{
				y1[x1_id] = 65535;
			}
			x1_id++;
		}
		 numt = (x1[nihe_len - 1] - x1[0])/20;
		 if (numt <= 1)   //防止死机
		 {
			 *HR = 0;
			 *SBP = 0;
			 *BP_CONFIDENCE = 10;
			 *DBP = 0;
			 *ABP = 0;

			 *XLBQ_FLAG = 0;


			 tingzhi_ganrao = 0;
			 dbp = 0;
			 sbp = 0;
			 MAX_numt = 0;
			 peak_max_id = 1;
			 numt = 0;
			 avg_max_id = 0;
			 avg_max_y = 0;
			 f_hr = 0;
			 stop_flag = 0;
			 peak_now = 0;
			 peak_need = 0;
			 bp_now = 0;
			 bp_max = 0;
			 *STOP_FLAG = 1;
			 return;
		 }

		//float* t = (float*)malloc(sizeof(float) * numt);  //为进行插值的点申请空间  //in
		//memset(t, 0.0, sizeof(float) * numt);
		//float* multval = (float*)malloc(sizeof(float) * numt); 				   //out
		//memset(multval, 0.0, sizeof(float)* numt);

		//float t[5000] = { 0 };
		//float multval[5000] = { 0 };

		//for (j= 0; j < numt; j++)
		//{
		//	t[j] = (j + x1[0]);
		//	//printf(" %f\n",t[j]);
		//}

		//interp_multiPoint(x1, y1, nihe_len, t, p_data_float, numt);//x1,y1是插值前的横纵坐标，nihe_len是插值前的长度。																 																	 
		
															   
																   //t,multval是插值后的横纵坐标，numt是插值后的长度。


		//if (t == NULL)
		//	return;
		float s[5];
		int kk, m, lc;
		float u[5];
		FTX2 = 0;
		FTX3 = 0;
		int k = -1;
	
		uint16_t t_x;
		for (h = 0; h < numt; h++)
		{
			/*interp_onePoint(x, y, n, t[kkk], &tempVal);*/
			/*fval[kkk] = tempVal;*/


			t_x = x1[0] + h*20;
			//float s[5];
			//int kk, m, lc;
			//float u[5], p, q;
			//int k = -1;

			s[4] = 0.0f;
			s[0] = 0.0f;
			s[1] = 0.0f;
			s[2] = 0.0f;
			s[3] = 0.0f;

			if (nihe_len < 1)
				return;
			if (nihe_len == 1)
			{
				s[0] = y1[0];
				s[4] = y1[0];
				return;
			}
			if (nihe_len == 2)
			{
				s[0] = y1[0];
				s[1] = (y1[1] - y1[0]) / (x1[1] - x1[0]);
				if (k < 0)
					s[4] = (y1[0] * (t_x - x1[1]) - y1[1] * (t_x - x1[0])) / (x1[0] - x1[1]);
				return;
			}


			if (k < 0 && nihe_len>0 && t_x < x1[0])
			{


				s[4] = y1[0];
				return;
			}


			if (k < 0 && nihe_len>0 && t_x > x1[nihe_len - 1])
			{


				s[4] = y1[nihe_len - 1];
				return;
			}

			if (k < 0)
			{


				if (t_x <= x1[1])
					kk = 0;
				else if (t_x >= x1[nihe_len - 1])
					kk = nihe_len - 2;
				else
				{


					kk = 1;
					m = nihe_len;
					while (((kk - m) != 1) && ((kk - m) != -1))
					{
						lc = (kk + m) / 2;
						if (t_x < x1[lc - 1])
							m = lc;
						else
							kk = lc;


					}
					kk = kk - 1;
				}
			}
			else
				kk = k;


			if (kk > nihe_len - 1)
				kk = nihe_len - 2;


			u[2] = (y1[kk + 1] - y1[kk]) / (x1[kk + 1] - x1[kk]);
			if (nihe_len == 3)
			{
				if (kk == 0)
				{
					u[3] = (y1[2] - y1[1]) / (x1[2] - x1[1]);
					u[4] = 2.0 * u[3] - u[2];
					u[1] = 2.0 * u[2] - u[3];
					u[0] = 2.0 * u[1] - u[2];
				}
				else
				{
					u[1] = (y1[1] - y1[0]) / (x1[1] - x1[0]);
					u[0] = 2.0 * u[1] - u[2];
					u[3] = 2.0 * u[2] - u[1];
					u[4] = 2.0 * u[3] - u[2];
				}


			}
			else
			{
				if (kk <= 1)
				{
					u[3] = (y1[kk + 2] - y1[kk + 1]) / (x1[kk + 2] - x1[kk + 1]);
					if (kk == 1)
					{
						u[1] = (y1[1] - y1[0]) / (x1[1] - x1[0]);
						u[0] = 2.0 * u[1] - u[2];
						if (nihe_len == 4)
							u[4] = 2.0 * u[3] - u[2];
						else
							u[4] = (y1[4] - y1[3]) / (x1[4] - x1[3]);
					}
					else
					{
						u[1] = 2.0 * u[2] - u[3];
						u[0] = 2.0 * u[1] - u[2];
						u[4] = (y1[3] - y1[2]) / (x1[3] - x1[2]);
					}


				}
				else if (kk >= (nihe_len - 3))
				{
					u[1] = (y1[kk] - y1[kk - 1]) / (x1[kk] - x1[kk - 1]);
					if (kk == (nihe_len - 3))
					{
						u[3] = (y1[nihe_len - 1] - y1[nihe_len - 2]) / (x1[nihe_len - 1] - x1[nihe_len - 2]);
						u[4] = 2.0 * u[3] - u[2];
						if (nihe_len == 4)
							u[0] = 2.0 * u[1] - u[2];
						else
							u[0] = (y1[kk - 1] - y1[kk - 2]) / (x1[kk - 1] - x1[kk - 2]);


					}
					else
					{
						u[3] = 2.0 * u[2] - u[1];
						u[4] = 2.0 * u[3] - u[2];
						u[0] = (y1[kk - 1] - y1[kk - 2]) / (x1[kk - 1] - x1[kk - 2]);
					}


				}
				else
				{
					u[1] = (y1[kk] - y1[kk - 1]) / (x1[kk] - x1[kk - 1]);
					u[0] = (y1[kk - 1] - y1[kk - 2]) / (x1[kk - 1] - x1[kk - 2]);
					u[3] = (y1[kk + 2] - y1[kk + 1]) / (x1[kk + 2] - x1[kk + 1]);
					u[4] = (y1[kk + 3] - y1[kk + 2]) / (x1[kk + 3] - x1[kk + 2]);
				}


			}


			s[0] = fabs(u[3] - u[2]);
			s[1] = fabs(u[0] - u[1]);
			if ((s[0] + 1.0 == 1.0) && (s[1] + 1.0 == 1.0))
				FTX2 = (u[1] + u[2]) / 2.0;
			else
				FTX2 = (s[0] * u[1] + s[1] * u[2]) / (s[0] + s[1]);
			s[0] = fabs(u[3] - u[4]);
			s[1] = fabs(u[2] - u[1]);
			if ((s[0] + 1.0 == 1.0) && (s[1] + 1.0 == 1.0))
				FTX3 = (u[2] + u[3]) / 2.0;
			else
				FTX3 = (s[0] * u[2] + s[1] * u[3]) / (s[0] + s[1]);


			s[0] = y1[kk];
			s[1] = FTX2;
			s[3] = x1[kk + 1] - x1[kk];
			s[2] = (3.0 * u[2] - 2.0 * FTX2 - FTX3) / s[3];
			s[3] = (FTX2 + FTX3 - 2.0 * u[2]) / (s[3] * s[3]);
			if (k < 0)
			{
				FTX2 = t_x - x1[kk];
				s[4] = s[0] + s[1] * FTX2 + s[2] * FTX2 * FTX2 + s[3] * FTX2 * FTX2 * FTX2;

			}


			p_data[h] = (uint16_t)s[4];  //三次样条拟合的结果存放在p_data的前numt个，取代了原数据的前numt个数，numt长度相对于原p_num长度缩短了20倍，从numt开始到最后的数已经没用了，相当于被我们浓缩在前numt个数里面了。

		}


		//FILE* fw35;
		//errno_t err35;
		//err35 = fopen_s(&fw35, "D:\\HM117\\117DATA\\1136nihe2.txt", "wb");
		//for (int i = 0; i < p_num; i++)
		//{

		//	fprintf(fw35, "%d\r\n", p_data[i]);
		//}
		//fclose(fw35);

		//***************************************************
		FILE* fw7;
		errno_t err7;
		err7 = fopen_s(&fw7, "D:\\HM117\\117DATA\\1136nihe2.txt", "wb");
		for (i = 0; i < numt; i++)
		{

			fprintf(fw7, " %d %d\r\n", x1[0] + i*20, p_data[i]/20);
		}
		fclose(fw7);

		////以下代码是三次多项式的拟合代码，三次多项式的拟合思想是:描述出一群离散点的大致趋势，是一种相对于三次样条的宏观拟合方式。

		FTX = 0;  //kk
		FTX2 = 0; //m
		FTX3 = 0; //h
		FTX4 = 0; //lc
		//kk = 0;
		//m = 0;
		//h = 0;
		//lc = 0;
		//getCoeff(t ,p_data_float, 3,numt, &FTX, &FTX2, &FTX3, &FTX4);

		////三次多项式拟合，该代码在网上可以找到，这里为适用于单片机，修改了表达方式（删除了vector，改成二维数组），算术流程不变
		 float matFunX[4][4] = { 0 };  //左矩阵
		 float matFunY[4][1] = { 0 };  //右矩阵
		 float  temp[4] = { 0 };
	 
	
		  k=0;
		//左矩阵赋值
		for (i = 0; i <= 3; i++)
		{


			/*temp.clear();*/
			for (j = 0; j <= 3; j++)
			{
				FTX = 0;
				for (k = 0; k < numt; k++)
				{
				
					FTX += pow((x1[0]+k*20), j+i);
				}
				
				temp[j] = FTX;
			}
			matFunX[i][0] = temp[0];
			matFunX[i][1] = temp[1];
			matFunX[i][2] = temp[2];
			matFunX[i][3] = temp[3];

		}
		//printf("matFunX.size=%d\n", matFunX.size());
		//printf("matFunX[3][3]=%f\n", matFunX[3][3]);

		//右矩阵赋值

		for (i = 0; i <= 3; i++)
		{

			FTX = 0;
			for (k = 0; k < numt; k++)
				FTX += (p_data[k] /20)* pow((x1[0]+k*20), i);
			temp[i] = FTX;
			matFunY[i][0] = temp[i];
		}
		/*printf("matFunY.size=%d\n", matFunY.size());*/
		//矩阵行列式变换
		/*float num1, num2, ratio;*/
	
		for (i = 0; i < 4 - 1; i++)
		{
			FTX2 = matFunX[i][i];
			for (j = i + 1; j < 4; j++)
			{
				FTX3 = matFunX[j][i];
				FTX4 = FTX3 / FTX2;
				for (k = 0; k < 4; k++)
					matFunX[j][k] = matFunX[j][k] - matFunX[i][k] * FTX4;
				matFunY[j][0] = matFunY[j][0] - matFunY[i][0] * FTX4;
			}
		}
		//计算拟合曲线的系数
		/*float coeff[4] = { 0 };*/
	
		for (i = 4 - 1; i >= 0; i--)
		{
			if (i == 4 - 1)
				temp[i] = matFunY[i][0] / matFunX[i][i];
			else
			{
				for (j = i + 1; j < 4; j++)
					matFunY[i][0] = matFunY[i][0] - (temp[j]) * matFunX[i][j];
				temp[i] = matFunY[i][0] / matFunX[i][i];
			}
		}
		FTX = temp[0];             //三次多项式拟合的四个系数，也就是我们最终要的结果
		FTX2 = temp[1];
		FTX3 = temp[2];
		FTX4 = temp[3];


		m = 0;//原始 p_data[j] /30
		lc = 0;// 原始avg
		ITX = 0;  
		ITX2 = 0; 
		t_x = x1[0];
		h = 0;



		for (j = 0; j < numt; j++)
		{
			lc = (FTX + FTX2 * (t_x +j*20)+ FTX3 * (t_x + j*20) * (t_x + j*20) + FTX4 * (t_x + j*20) * (t_x + j*20) * (t_x + j*20)); //y=a0+a1*x+a2*x^2+a3*x^4
			//将四个系数带入原数据，计算三次多项式的拟合结果lc。


			m = p_data[j] / 20;  //之前的三次样条拟合结果（放大了20倍的结果）存在p_data的前numt个数组中，这里的m将其减小20倍取出来。
			//h = m;
			h = (m + lc) / 2;    // 去三次样条和三次多项式拟合的平均值
			h = h > 0 ? h : 0;
			p_data[j] = h ;       //最后的结果仍然放在p_data中，但长度只有numt个，这时我们有用的就是这numt个数了，从numt 到 p_num的原数据没有用了。其实numt就是p_num减小20倍左右的数。
			/*t[j] = t[j];*/
	
			if (avg_max_y <= p_data[j])//顺便求拟合曲线的最大值
			{
				avg_max_y = p_data[j];
				avg_max_id = j*20 + x1[0];
			}
	
		}
		
		//此处是在实际测试中改动的部分，主要问题是面对某些高压不准的情况，表现出的情况是原始的最高点与拟合后的最高点差距较大。所以我们针对这种情况重新设定拟合的最高值点。
		uint16_t avg_max_y_dbp = avg_max_y;
		if (T_peak[T_peak_x][1] - avg_max_y > 100)
		{
			avg_max_y = (T_peak[T_peak_x][1] * 7) / 10 + (avg_max_y * 3) / 10;
		}
		else if (T_peak[T_peak_x][1] - avg_max_y > 75)
		{
			avg_max_y = (T_peak[T_peak_x][1] * 6) / 10 + (avg_max_y * 4) / 10;
		}
		else if (T_peak[T_peak_x][1] - avg_max_y > 50)
		{

			avg_max_y = (T_peak[T_peak_x][1] * 5) / 10 + (avg_max_y * 5) / 10;
		}
		//for (j = 0; j < numt; j++)
		//{
		//	
		//	avg[j] = (avg[j] + p_data[j] /30) / 2;

		//	if (avg_max_y < avg[j])//求avg的 原位置，顺便求拟合曲线的最大值
		//	{
		//		avg_max_y = avg[j];
		//		avg_max_id =j;
		//	}
		//}


	//接下来开始计算收缩压和舒张压。
		uint16_t  mbp_predict_avg = 0;
		mbp_predict_avg = p_bp[ avg_max_id/10];   //根据最大值点对应的压力值即是平均压
	
		float KK_sbp = 0, KK_dbp = 0; //波动区间      //根据幅度系数法，在不同的平均压阶段，设定不同的阈值范围（设定值是由大量数据总结而来）
		if (mbp_predict_avg <= 65)   //80   60-62
		{
			KK_sbp = 0.58;
			KK_dbp = 0.62;
		}
		else if (mbp_predict_avg <= 75 && mbp_predict_avg > 65)  //90 72-74
		{
			KK_sbp = 0.58;
			KK_dbp = 0.620;
		}
		else if (mbp_predict_avg <= 80 && mbp_predict_avg > 75)  //100  77-79
		{
			KK_sbp = 0.59;
			KK_dbp = 0.6513;
		}
		else if (mbp_predict_avg <= 90 && mbp_predict_avg > 80)  //110  83-86
		{
			KK_sbp = 0.60;
			KK_dbp = 0.6728;  //6728
		}
		else if (mbp_predict_avg < 100 && mbp_predict_avg > 90)  //120  95-98
		{
			KK_sbp = 0.63;
			KK_dbp = 0.66;  //68
		}
		else if (mbp_predict_avg <= 105 && mbp_predict_avg >= 100)  //130  101-103
		{
			KK_sbp = 0.63;
			KK_dbp = 0.658;
		}
		else if (mbp_predict_avg <= 110 && mbp_predict_avg > 105)  //140  107-108
		{
			KK_sbp = 0.63;
			KK_dbp = 0.67;
		}
		else if (mbp_predict_avg <= 120 && mbp_predict_avg > 110)  //150  116-119
		{
			KK_sbp = 0.63;
			KK_dbp = 0.62;
		}
		else if (mbp_predict_avg <= 125 && mbp_predict_avg > 120)  //160  121-123
		{
			KK_sbp = 0.63;
			KK_dbp = 0.62;
		}
		else if (mbp_predict_avg <= 135 && mbp_predict_avg > 125)  //170 131-132
		{
			KK_sbp = 0.64;
			KK_dbp = 0.63;
		}
		else if (mbp_predict_avg <= 150 && mbp_predict_avg > 135)  //180  147-148
		{
			KK_sbp = 0.65;
			KK_dbp = 0.61;
		}
		else if (mbp_predict_avg < 160 && mbp_predict_avg > 150)  //200  153-155
		{
			KK_sbp = 0.65;
			KK_dbp = 0.625;
		}
		else
		{
			KK_sbp = 0.65; //0.44
			KK_dbp = 0.62; //0.622
		}

		//(T_peak[T_id - 1][1] + T_peak[T_id - 1][1]) / (T_peak[T_id - 1][1] + T_peak[T_id - 1][1])
		//printf(sbp);
		//printf(dbp);
		//int Ganrao_Flag_Left = 0, Ganrao_Flag_Right = 0; // 干扰标志位
		if (avg_max_id - x1[0] > 0)   //防止死机
		{
			MAX_numt = avg_max_id - x1[0];
		}
		else
		{
			*HR = 0;
			*SBP = 0;
			*BP_CONFIDENCE = 10;
			*DBP = 0;
			*ABP = 0;

			*XLBQ_FLAG = 0;


			tingzhi_ganrao = 0;
			dbp = 0;
			sbp = 0;
			MAX_numt = 0;
			peak_max_id = 1;
			numt = 0;
			avg_max_id = 0;
			avg_max_y = 0;
			f_hr = 0;
			stop_flag = 0;
			peak_now = 0;
			peak_need = 0;
			bp_now = 0;
			bp_max = 0;
			*STOP_FLAG = 1;
			return;
		}
		//MAX_numt是缩短数据后的真正最大值点的横坐标
        MAX_numt  =  MAX_numt /20;      //代码里的乘或者除20，都是缩短20倍数据长度的原因。
                

        //从最大值点向右根据幅度系数查找收缩压        
		for (i = MAX_numt; i < numt; i++) //
		{
			if (p_data[i] <= avg_max_y * KK_sbp)
			{
				sbp = p_bp[(i * 20 + x1[0]) / 10];
				break;
			}
		}
		//从最大值点向左根据幅度系数查找舒张压  
		for (i = MAX_numt; i >= 0; i--)
		{
			if (p_data[i] <= avg_max_y_dbp * KK_dbp)
			{
				/*int xx = i;*/
				dbp = p_bp[(i * 20 + x1[0]) / 10 + 1];
				break;
			}
		}

		FILE* fw34;
		errno_t err34;
		err34 = fopen_s(&fw34, "D:\\HM117\\117DATA\\1136CCC.txt", "wb");
		for (int i = 0; i < p_num; i++)
		{

			fprintf(fw34, "%d\r\n", p_data[i]);
		}
		fclose(fw34);


		//(T_peak[T_id - 1][1] + T_peak[T_id - 1][1]) / (T_peak[T_id - 1][1] + T_peak[T_id - 1][1])
		//printf(sbp);
		//printf(dbp);
		//int Ganrao_Flag_Left = 0, Ganrao_Flag_Right = 0; // 干扰标志位
		/*FTX = 0;*/
		FTX2 = 0;
		FTX3 = 0;
		//FTX2 = (avg_max_y - T_peak[T_id - 1][1]) / (T_peak[T_id - 1][0] - FTX );
		FTX = avg_max_id;
		if ((numt - 1) * 20 + x1[0] - FTX > 0) //分母大于0
		{
			FTX2 = (avg_max_y - p_data[numt - 1]) / ((numt - 1) * 20 + x1[0] - FTX); //右侧斜率
		}
		else
		{
			*HR = 0;
			*SBP = 0;
			*BP_CONFIDENCE = 10;
			*DBP = 0;
			*ABP = 0;

			*XLBQ_FLAG = 0;


			tingzhi_ganrao = 0;
			dbp = 0;
			sbp = 0;
			MAX_numt = 0;
			peak_max_id = 1;
			numt = 0;
			avg_max_id = 0;
			avg_max_y = 0;
			f_hr = 0;
			stop_flag = 0;
			peak_now = 0;
			peak_need = 0;
			bp_now = 0;
			bp_max = 0;
			*STOP_FLAG = 1;
			return;
		}
		FTX3 = (avg_max_y - p_data[0]) / (FTX - x1[0]);  //左侧斜率
		FTX4 = 100;

		ITX = 0; ITX2 = 0;
		//下面的代码是针对上面搜索高压值和低压值搜索不到的情况，先分别算出上升和下降的斜率，然后在原拟合曲线上进行延长，再去找高低压。
		if (numt - 1 - MAX_numt >= 5)  //高压右侧斜率
		{
			FTX = (p_data[numt - 6] - p_data[numt - 1]) / FTX4;
			FTX = FTX > 0.6 ? FTX : 0.6;
			FTX2 = FTX2 > FTX ? FTX2 : FTX;
		}
		if (MAX_numt >= 5)        //低压左侧斜率
		{ 
			FTX = (p_data[5] - p_data[0]) / FTX4;
			FTX = FTX > 0.3 ? FTX : 0.3;
			FTX3 = FTX3 > FTX ? FTX3 : FTX;
		}



		//FTX3 = (avg_max_y -p_data[numt-1]) / (FTX - T_peak[0][0]);


		ITX = 0, ITX2 = 0;
		FTX4 = abs_int(p_bp[p_num_bp - 1] - p_bp[0]);
		FTX4 = FTX4 / p_num;


		//需要延长曲线才能计算血压的情况
		if (sbp == 0 && FTX2 != 0)   //高压部分
		{
			//abs_int(p_data[(numt - 1)] - avg_max_y * KK_sbp) 
			ITX2 = abs_int(p_data[(numt - 1)] - avg_max_y * KK_sbp) / FTX2;
			if (ITX2 > 500)   //如果需要向右延长的曲线太长，则说明有问题，应该减小置信度
			{
				BP_Confidence = BP_Confidence - 20;
				sbp = p_bp[p_num_bp - 1] * 0.9;
			}
			else
			{
				sbp = p_bp[(T_peak[T_id - 1][0] / 10) - 1] + ITX2 * FTX4;
			}
		
		}
		if (dbp == 0 && FTX3 != 0)  //低压部分
		{
			ITX = abs_int(p_data[0] - avg_max_y_dbp * KK_dbp) / FTX3;
			if (ITX > T_peak[0][0])   //如果向左延长曲线过长，则可能出现负值，需要较小置信度
			{
				dbp = (3 * mbp_predict_avg - sbp) / 2;
				BP_Confidence = BP_Confidence - 15;
			}
			else
			{
				dbp = p_bp[(T_peak[0][0] / 10) + 1] - ITX * FTX4;
			}
			
		}

		if ((T_peak_x < (T_id/5) && T_id>10) && dbp>10)    //置信度的微调
		{

			BP_Confidence = BP_Confidence - 20;
		}
		if (((T_id - T_peak_x) < T_id / 5 ) && sbp > 10)//置信度的微调
		{

			BP_Confidence = BP_Confidence - 20;
		}
                
        sbp = sbp -2;   //血压值的微调
		dbp = dbp - 1;  //血压值的微调
                
                
		//血压值的范围设定
		if (sbp < 30 )
		{
			sbp = 30;	
		}
		if (sbp >255 )
		{
			sbp = 255;
		}
		if (dbp < 20)
		{
			dbp = 20;
		}
		if (dbp >200)
		{
			dbp = 200;
		}
		uint16_t temp_bp = 0;
		if (sbp <= dbp) //置信度的微调
		{
			temp_bp = sbp;
			sbp = dbp;
			dbp = temp_bp;
			BP_Confidence =30;
		}
		if (mbp_predict_avg >= sbp || mbp_predict_avg <= dbp)//置信度的微调，在函数外部，低于70会提示干扰过大。
		{
			mbp_predict_avg = sbp * 0.66 + dbp * 0.33;
			BP_Confidence = BP_Confidence - 20;
		}
		if (f_hr < 40)
		{
			f_hr = 40;
		}
	
	
	
		//置信度的的合成
		BP_Confidence = BP_Confidence - T_count*2 - T_count3 - BP_CONFIDENCE_PRE;
		if ( sbp - dbp < 20)
		{
			BP_Confidence = BP_Confidence - 50;
		}
		if (sbp > 90 &&  sbp - dbp >80)
		{
			BP_Confidence = BP_Confidence - 50;
		}
		if (numt-MAX_numt <= 5 || MAX_numt <= 5)
		{
			BP_Confidence = BP_Confidence - 50;
		}


		if (BP_Confidence < 10)
		{
			BP_Confidence = 10 ;
		}
	
		if (small_xielv > 2)//血压值的微调
		{
			sbp = sbp - (small_xielv - 2) * 1;
		}


		*HR = f_hr;
		*SBP = sbp;
		*BP_CONFIDENCE = BP_Confidence;
		*DBP = dbp;
		*ABP = mbp_predict_avg;
	
		*XLBQ_FLAG = XLBQ;
		if (*LOOSE_CUFF == 1)
		{
			*HR = 0;
			*SBP = 0;
			*BP_CONFIDENCE = 0;
			*DBP = 0;
		}

		tingzhi_ganrao = 0;
		dbp = 0;
		sbp = 0;
		MAX_numt = 0;
		peak_max_id = 1;
		numt = 0;
		avg_max_id = 0;
		avg_max_y = 0;
        f_hr =0;
		stop_flag = 0;
		peak_now = 0;
		peak_need = 0;
		bp_now = 0;
		bp_max = 0;
		*STOP_FLAG = 1;


	}



	
}

int abs_int(int a)
{
	if (a >= 0)
		return a;
	else
		return a * (-1);
}

