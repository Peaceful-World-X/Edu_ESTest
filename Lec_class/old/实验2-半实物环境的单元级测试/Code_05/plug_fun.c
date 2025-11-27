#include <stdio.h>
#include <stdlib.h>
#include "plug_fun.h"
#include "filter.h"
#define N 5  // 移动平均滤波和中值滤波窗口大小

// 1. 移动平均滤波实现
int ma_line1 = 0, ma_line2 = 0, ma_line3 = 0;
float moving_average_filter_pl(float input[], int len) {
    ma_line1=1;  // 记录进入函数
    float sum = 0;
    for (int i = 0; i < len; i++) {
        ma_line2=1;  // 记录for循环每次执行
        sum += input[i];
    }
    ma_line3=1;  // 记录返回语句执行
    return sum / len;
}

// 2. 指数加权移动平均滤波（EWMA）实现
int ewma_line1 = 0, ewma_line2 = 0, ewma_line3 = 0;
float exponential_weighted_moving_average_pl(float input[], int len, float alpha) {
    ewma_line1=1;  // 记录进入函数
    float ewma = input[0];
    for (int i = 1; i < len; i++) {
        ewma_line2=1;  // 记录for循环每次执行
        ewma = alpha * input[i] + (1 - alpha) * ewma;
    }
    ewma_line3=1;  // 记录返回语句执行
    return ewma;
}

// 3. 中值滤波实现
int mf_line1 = 0, mf_line2 = 0, mf_line3 = 0, mf_line4 = 0, mf_line5 = 0;
int compare_pl(const void *a, const void *b) {
    return (*(float *)a - *(float *)b);
}
float median_filter_pl(float input[], int len) {
    mf_line1=1;  // 记录进入函数
    float sorted[len];
    for (int i = 0; i < len; i++) {
        mf_line2=1;  // 记录for循环每次执行
        sorted[i] = input[i];
    }
    qsort(sorted, len, sizeof(float), compare_pl);
    mf_line3=1;  // 记录qsort执行
    if (len % 2 == 0) {
        mf_line4=1;  // 记录偶数长度时的返回
        return (sorted[len / 2 - 1] + sorted[len / 2]) / 2.0;
    } else {
        mf_line5=1;  // 记录奇数长度时的返回
        return sorted[len / 2];
    }
}

// 测试语句覆盖率
void test_Coverage() {

    float data1[] = {1.0, 2.0, 3.0};
    float data2[] = {1.0, 2.0, 3.0, 4.0};

    // 静态插装：手动插入计数器
    printf("--------Static instrumentation results--------\n");
    // 测试用例1：奇数个数据
    printf("Test Case 1:\n");
    printf("Moving Average: %.2f\n", moving_average_filter_pl(data1, 3));
    printf("EWMA: %.2f\n", exponential_weighted_moving_average_pl(data1, 3, 0.5));
    printf("Median: %.2f\n", median_filter_pl(data1, 3));
    // 测试用例2：偶数个数据
    printf("\nTest Case 2:\n");
    printf("Moving Average: %.2f\n", moving_average_filter_pl(data2, 4));
    printf("EWMA: %.2f\n", exponential_weighted_moving_average_pl(data2, 4, 0.3));
    printf("Median: %.2f\n", median_filter_pl(data2, 4));
    // 输出覆盖率报告
    printf("\nCoverage Report:\n");
    printf("Moving Average Filter:%.2f%%\n", (ma_line1+ma_line2+ma_line3)/3.0*100);
    printf("EWMA Filter:%.2f%%\n", (ewma_line1+ewma_line2+ewma_line3)/3.0*100);
    printf("Median Filter:%.2f%%\n", (mf_line1+mf_line2+mf_line3+mf_line4+mf_line5)/5.0*100);

    // 动态插装：使用 gcvg 工具获取覆盖率
    printf("\n--------Dynamic installation results--------\n");
    // 测试用例1：奇数个数据
    printf("Test Case 1:\n");
    printf("Moving Average: %.2f\n", moving_average_filter(data1, 3));
    printf("EWMA: %.2f\n", exponential_weighted_moving_average(data1, 3, 0.5));
    printf("Median: %.2f\n", median_filter(data1, 3));
    // 测试用例2：偶数个数据
    printf("\nTest Case 2:\n");
    printf("Moving Average: %.2f\n", moving_average_filter(data2, 4));
    printf("EWMA: %.2f\n", exponential_weighted_moving_average(data2, 4, 0.3));
    printf("Median: %.2f\n", median_filter(data2, 4));
}
