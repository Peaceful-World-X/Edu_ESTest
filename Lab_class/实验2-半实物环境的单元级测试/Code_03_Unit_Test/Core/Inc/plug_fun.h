#ifndef __plug_fun_H
#define __plug_fun_H

extern int ma_line1, ma_line2, ma_line3;
extern int ewma_line1, ewma_line2, ewma_line3;
extern int mf_line1, mf_line2, mf_line3, mf_line4, mf_line5;

float moving_average_filter_pl(float input[], int len);
float exponential_weighted_moving_average_pl(float input[], int len, float alpha);
float median_filter_pl(float input[], int len);


#endif

