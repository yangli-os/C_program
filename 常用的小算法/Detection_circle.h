#ifndef __DETECT_CIRCLE_H__
#define __DETECT_CIRCLE_H__

class DetectCircle
{
	public:
		float data_c_dimention2[256][256] = { 0 };

		int Detect_circle(float* data_c, int len_data_c);
};


#endif //#ifndef __DETECT_CIRCLE_H__