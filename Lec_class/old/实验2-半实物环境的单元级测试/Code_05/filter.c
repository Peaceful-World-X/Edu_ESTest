#include <stdlib.h>
#include "filter.h"
#define N 5  // 移动平均滤波和中值滤波窗口大小

// 1. 移动平均滤波实现
/*
 * 函数名称: moving_average_filter
 * 功能说明: 实现对输入数组的移动平均滤波，计算输入数据的平均值。
 * 原理说明: 将窗口内所有数值相加后取平均，适合平滑处理和去除高频噪声。
 * 参数:
 *   - input[]: 输入数据数组
 *   - len: 数据数组的长度
 * 返回值: 计算后的移动平均值
 */
float moving_average_filter(float input[], int len) {
    float sum = 0;
    for (int i = 0; i < len; i++) {
        sum += input[i];
    }
    return sum / len;
}

// 2. 指数加权移动平均滤波（EWMA）实现
/*
 * 函数名称: exponential_weighted_moving_average
 * 功能说明: 对输入数据进行指数加权移动平均滤波，赋予新数据更高的权重。
 * 原理说明: 通过系数 alpha 对新旧数据加权求和，alpha 越大越重视新数据。
 * 参数:
 *   - input[]: 输入数据数组
 *   - len: 数据数组的长度
 *   - alpha: 权重因子，取值范围为 0 到 1
 * 返回值: 计算后的指数加权移动平均值
 */
float exponential_weighted_moving_average(float input[], int len, float alpha) {
    float ewma = input[0];  // 初始化为第一个输入值
    for (int i = 1; i < len; i++) {
        ewma = alpha * input[i] + (1 - alpha) * ewma;
    }
    return ewma;
}

// 3. 中值滤波实现
/*
 * 函数名称: median_filter
 * 功能说明: 对输入数据进行中值滤波，去除数据中的脉冲噪声。
 * 原理说明: 将窗口内的数据排序，取中间值作为滤波后的结果。
 * 参数:
 *   - input[]: 输入数据数组
 *   - len: 数据数组的长度
 * 返回值: 计算后的中值滤波结果
 */
int compare(const void *a, const void *b) {
    return (*(float *)a - *(float *)b);
}
float median_filter(float input[], int len) {
    float sorted[N];
    for (int i = 0; i < len; i++) {
        sorted[i] = input[i];
    }
    qsort(sorted, len, sizeof(float), compare);
    if(len % 2 == 0) {
        return (sorted[len / 2 - 1] + sorted[len / 2]) / 2.0;
    } else {
        return sorted[len / 2];
    }
}
