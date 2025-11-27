#ifndef __FILTER_H
#define __FILTER_H


float moving_average_filter(float input[], int len);
float exponential_weighted_moving_average(float input[], int len, float alpha);
float median_filter(float input[], int len);


#endif

