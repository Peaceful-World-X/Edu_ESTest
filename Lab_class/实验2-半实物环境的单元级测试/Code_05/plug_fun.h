#ifndef __plug_fun_H
#define __plug_fun_H


float moving_average_filter_pl(float input[], int len);
float exponential_weighted_moving_average_pl(float input[], int len, float alpha);
float median_filter_pl(float input[], int len);

void test_Coverage();

#endif

