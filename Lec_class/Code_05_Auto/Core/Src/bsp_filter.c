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

// 卡尔曼滤波器
void KLF(KalmanFilter *kf, float measurement) {
    // 预测步骤
    kf->x = kf->F * kf->x;  // 预测状态
    kf->P = kf->F * kf->P * kf->F + kf->Q;  // 预测误差协方差
    // 更新步骤
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
