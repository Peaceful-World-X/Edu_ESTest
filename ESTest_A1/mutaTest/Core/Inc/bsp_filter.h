#ifndef __BSP_FILTER_H
#define __BSP_FILTER_H

#include "main.h"
#include <stdio.h>

#define WINDOW_SIZE 50

// 移动平均滤波器
typedef struct {
    float window[WINDOW_SIZE];
    int count;
}MovingAverageFilter;

// 中值滤波器
typedef struct {
    float window[WINDOW_SIZE];
    int count;
}MedianFilter;

// 卡尔曼滤波器
typedef struct {
    float x;  // 状态估计
    float P;  // 误差协方差
    float Q;  // 过程噪声协方差
    float R;  // 测量噪声协方差
    float K;  // 卡尔曼增益
    float F;  // 状态转移矩阵
    float H;  // 观测矩阵
}KalmanFilter;

void KLF_init(KalmanFilter *kf, float x, float p, float q, float r, float f, float h);
void KLF(KalmanFilter *kf, float measurement);

void  MDF_Init(MedianFilter *filter);
float MDF(MedianFilter *filter, float new_measurement);

void  MAF_Init(MovingAverageFilter *filter);
float MAF(MovingAverageFilter *filter, float new_measurement);

float_t F_Result(float_t data);
double L_Result(uint16_t data);

float moving_average_filter(float input[],int len);
float exponential_weighted_moving_average(float* input,int len,float alpha);

#endif

