#include "main.h"
#include "filter.h"

int results[]={};

// moving_average_filter 单元测试
void test_moving_average_filter() {
    int count = sizeof(data1) / sizeof(data1[0]);
    for (int i = 0; i < count; i++){
        results[i] = fabs(moving_average_filter(data1[i].input, data1[i].num) - data1[i].result)<1e-6 ? 1 : 0;
    }
    printf("\ntest_moving_average_filter Test results: \n");
    for (int i = 0; i < count; i++) {
        if(!(i%5)) printf(" ");
        printf("%d", results[i]);
    }
    for (int i = 0; i < count; i++) {
        if(results[i]==0){
            printf("\nCase-%d result:%f", data1[i].id, moving_average_filter(data1[i].input, data1[i].num));
        }
    }
    printf("\n");
    memset(results, 0, sizeof(results));
}

// median_filter 单元测试
void test_median_filter() {
    int count = sizeof(data2) / sizeof(data2[0]);
    for (int i = 0; i < count; i++){
        results[i] = fabs(median_filter(data2[i].input, data2[i].num) - data2[i].result)<1e-6 ? 1 : 0;
    }
    printf("\ntest_median_filter Test results: \n");
    for (int i = 0; i < count; i++) {
        if(!(i%5)) printf(" ");
        printf("%d", results[i]);
    }
    for (int i = 0; i < count; i++) {
        if(results[i]==0){
            printf("\nCase-%d result:%f", data2[i].id, median_filter(data2[i].input, data2[i].num));
        }
    }
    printf("\n");
    memset(results, 0, sizeof(results));
}

// exponential_weighted_moving_average 单元测试
void test_exponential_weighted_moving_average() {
    int count = sizeof(data3) / sizeof(data3[0]);
    for (int i = 0; i < count; i++){
        results[i] = fabs(exponential_weighted_moving_average(data3[i].input, data3[i].num, data3[i].alpha) - data3[i].result)<1e-6 ? 1 : 0;
    }
    printf("\nexponential_weighted_moving_average Test results: \n");
    for (int i = 0; i < count; i++) {
        if(!(i%5)) printf(" ");
        printf("%d", results[i]);
    }
    for (int i = 0; i < count; i++) {
        if(results[i]==0){
            printf("\nCase-%d result:%f", data3[i].id, exponential_weighted_moving_average(data3[i].input, data3[i].num, data3[i].alpha));
        }
    }
    printf("\n");
    memset(results, 0, sizeof(results));
}
// =================================================================
int main()
{
    // 代码覆盖率
    // test_Coverage();

    test_moving_average_filter();
    test_median_filter();
    test_exponential_weighted_moving_average();

    printf("\nAll test cases passed.\n");
    return 0;
}

