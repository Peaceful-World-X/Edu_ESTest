#include "bsp_filter.h"

// 初始化卡尔曼滤波器
void KLF_init(KalmanFilter *kf, float x, float p, float q, float r, float f, float h) {
    kf->x = x;
    kf->P = p;
    kf->Q = q;
    kf->R = r;
    kf->F = f;
    kf->H = h;
}

void KLF(KalmanFilter *kf, float measurement) {
    kf->x = kf->F * kf->x;  // 预测状态
    kf->P = kf->F * kf->P * kf->F + kf->Q;  // 预测误差协方差
    kf->K = kf->P * kf->H / (kf->H * kf->P * kf->H + kf->R);  // 计算卡尔曼增益
    kf->x = kf->x + kf->K * (measurement - kf->H * kf->x);  // 更新估计
    kf->P = (1 - kf->K * kf->H) * kf->P;  // 更新误差协方差
}

/***********************************************************************************************/
// 初始化中值滤波器
void MDF_Init(MedianFilter *filter) {
    for (int i = 0; i < WINDOW_SIZE; i++) {
        filter->window[i] = 0.0;
    }
    filter->count = 0;
}

// 中值滤波器
float MDF(MedianFilter *filter, float new_measurement) {
    if (filter->count < WINDOW_SIZE) {
        filter->window[filter->count++] = new_measurement;
    } else {
        for (int i = 1; i < WINDOW_SIZE; i++) {
            filter->window[i - 1] = filter->window[i];
        }
        filter->window[WINDOW_SIZE - 1] = new_measurement;
    }

    float sorted_window[WINDOW_SIZE];
    for (int i = 0; i < filter->count; i++) {
        sorted_window[i] = filter->window[i];
    }
    
    // 插入排序
    float *window = sorted_window;
    int count = filter->count;
    for (int i = 1; i < count; i++) {
        float key = window[i];
        int j = i - 1;
        while (j >= 0 && window[j] > key) {
            window[j + 1] = window[j];
            j = j - 1;
        }
        window[j + 1] = key;
    }
    if (count % 2 == 0) {
        return (window[count / 2 - 1] + window[count / 2]) / 2.0;
    } else {
        return window[count / 2];
    }
}

/***********************************************************************************************/
// 初始化移动平均滤波器
void MAF_Init(MovingAverageFilter *filter) {
    for (int i = 0; i < WINDOW_SIZE; i++) {
        filter->window[i] = 0.0;
    }
    filter->count = 0;
}

// 移动平均滤波器
float MAF(MovingAverageFilter *filter, float new_measurement) {
    if(filter->count < WINDOW_SIZE) {
        filter->window[filter->count++] = new_measurement;
    }else{
        for (int i = 1; i < WINDOW_SIZE; i++) {
            filter->window[i - 1] = filter->window[i];
        }
        filter->window[WINDOW_SIZE - 1] = new_measurement;
    }

    float sum = 0.0;
    for (int i = 0; i < filter->count; i++) {
        sum += filter->window[i];
    }
    return sum / filter->count;
}
/***********************************************************************************************/
// 1. 移动平均滤波实现
/*
 * 函数名称: moving_average_filter
 * 功能说明: 实现对输入数组的移动平均滤波，计算输入数据的平均值。
 * 原理说明: 将窗口内所有数值相加后取平均，适合平滑处理和去除高频噪声。
 * 参数:
 *   - input[]: 输入数据数组
 * 返回值: 计算后的移动平均值
 */
float moving_average_filter(float input[],int len)
{
    float sum = 0;
	for (int i = 1; i < len; i++) {
		sum += input[i];
	}
    return sum*1.0 / len;
}

// 2. 指数加权移动平均滤波（EWMA）实现
/*
 * 函数名称: exponential_weighted_moving_average
 * 功能说明: 对输入数据进行指数加权移动平均滤波，赋予新数据更高的权重。
 * 原理说明: 通过系数 alpha 对新旧数据加权求和，alpha 越大越重视新数据。
 * 参数:
 *   - input[]: 输入数据数组
 *   - alpha: 权重因子，取值范围为 0 到 1
 * 返回值: 计算后的指数加权移动平均值
 */
float exponential_weighted_moving_average(float* input,int len,float alpha) {
	if(alpha<=0)	alpha=0;
	if(alpha>=1.2)	alpha=1;
    float ewma = input[0]; 
    for (int i = 1; i < len; i++) {
        ewma = alpha * input[i] + (1 - alpha) * ewma;
    }
    return ewma;
}


/***********************************************************************************************/
float_t F_Result(float_t data)
{
	if(data<20)
		return data*data-5;
	else if(data>30)
		return 2*data+10;
	else
		return (int)data/2;
}

double L_Result(uint16_t data)
{
	if(data<201)
		return data*0.85;
	else if(data>=800)
		return data*data*1.0/1000.0;
	else
		return data-5;
}
