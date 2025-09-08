#ifndef __FILTER_H
#define __FILTER_H


float moving_average_filter(float input[], int len);
float exponential_weighted_moving_average(float input[], int len, float alpha);
float median_filter(float input[], int len);


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <float.h>
#include <assert.h>
#include <math.h>

#define N 5
typedef struct TestData{
    int id;         // 测试用例序号
    int num;        // 输入数据长度
    float input[6]; // 输入数据
    float result;   // 断言输出,假设无效或者报错返回 0
}TData;
typedef struct TestData_alpha{
    int id;         // 测试用例序号
    int num;        // 输入数据长度
    float input[6]; // 输入数据
    float result;   // 断言输出,假设无效或者报错返回 0
    float alpha;    // 系数 alpha
}TData_alpha;


#endif

