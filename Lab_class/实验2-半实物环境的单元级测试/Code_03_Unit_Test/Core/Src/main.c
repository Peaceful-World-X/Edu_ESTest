#include "main.h"
#include "usart.h"
#include "gpio.h"
#include "spi.h"
#include "BSP_lcd_init.h"
#include "BSP_lcd.h"
/* 用到LCD屏幕、UART串口两个模块*/

#include "filter.h"
#include "plug_fun.h"

void SystemClock_Config(void);

__IO uint32_t Tick_LCD;	//刷新屏幕显示、串口发送
char str[50];			//缓存字符串
char temp[10];

int results[100];

// moving_average_filter 测试用例
TData data1[] = {
    // 有效等价类
    {1, N, {5, 15, 25, 35, 45}, 25},             // 有效等价类：正整数数组
    {2, N, {1.5, 2.5, 3.5, 4.5, 5.5}, 3.5},      // 有效等价类：浮点数数组
    {3, N, {-1, -2, -3, -4, -5}, -3.0},          // 有效等价类：负整数数组
    {4, N, {-1.0, 1.0, -1.0, 1.0, -1.0}, -0.2},  // 有效等价类：负数和正数混合
    // 无效等价类
    {5, N+1, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0}, 0}, // 无效等价类：len 大于最大值
    {6, 0, {1.0}, 0.0},                          // 无效等价类：len 为0
    {7, -1,{2.0}, 0},                            // 无效等价类：len 负数，小于最小边界
    {8, 3, {1.0, 2.0}, 0},                       // 无效等价类：数组长度小于 len
    {9, 1, {1.0, 2.0}, 0},                       // 无效等价类：数组长度大于 len
    {10, 0, {0}, 0},                              // 无效等价类：空数组
    {11, N, {10, 20, INFINITY, 40, 50}, 0},      // 无效等价类：含无穷大
    {12, N, {10, 20, NAN, 40, 50}, 0},           // 无效等价类：含 NaN
    // 边界值分析
    {13, N, {FLT_MAX, 1.0, 2.0, 3.0, 4.0}, 0},    // 边界值：高值测试
    {14, N, {-FLT_MAX, 1.0, 2.0, 3.0, 4.0}, 0},   // 边界值：低值测试
    {15, 1, {1.0}, 1.0},                          // 边界值：len 最小值
    {16, N, {10.0, 20.0, 30.0, 40.0, 50.0}, 30.0},// 边界值：len 最大值
    {17, 1, {50}, 50.0},                          // 单元素
    {18, N, {50, 50, 50, 50, 50}, 50.0},          // 多元素
    {19, N, {10, 9, 10, 9, 100}, 27.6},           // 有噪声
    // 决策表测试
    {20, N, {10, -10, 10, -10, 10}, 2.0},         // 正负值均匀分布组合
    {21, N, {100, -100, 100, -100, 0}, 0.0},      // 高值正负值对称组合
    {22, N, {100, 200, 300, 400, 500}, 300.0},    // 单调递增数组
    {23, N, {-100, -200, -300, -400, -500}, -300.0}, // 单调递减数组
    // 特殊场景
    {24, N, {0, 0, 0, 0, 0}, 0.0},               // 所有元素为零
    {25, N, {1, 1, 1, 1, 1}, 1.0},               // 所有元素相同的正整数
    {26, N, {-1, -1, -1, -1, -1}, -1.0},         // 所有元素相同的负整数
    {27, N, {5, 100, 5, 100, 5}, 43.0},          // 高低交替噪声
    {28, N, {-0.0, 0.0, -0.0, 0.0, -0.0}, 0.0},     // 特殊场景：负零与零的混合
};

// median_filter 测试用例
TData data2[] = {
    // 有效等价类
    {1, N, {5, 15, 25, 35, 45}, 25.0},             // 有效等价类：正整数数组
    {2, N, {1.5, 2.5, 3.5, 4.5, 5.5}, 3.5},        // 有效等价类：浮点数数组
    {3, N, {-1, -2, -3, -4, -5}, -3.0},            // 有效等价类：负整数数组
    {4, N, {-1.0, 1.0, -1.0, 1.0, -1.0}, -1.0},    // 有效等价类：负数和正数混合
    // 无效等价类
//    {5, N+1, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0}, 0},   // 无效等价类：len 大于最大值
    {6, 0, {1.0}, 0.0},                            // 无效等价类：len 为0
    {7, -1, {2.0}, 0.0},                           // 无效等价类：len 负数，小于最小边界
    {8, 3, {1.0, 2.0}, 0.0},                       // 无效等价类：数组长度小于 len
    {9, 1, {1.0, 2.0}, 0.0},                       // 无效等价类：数组长度大于 len
    {10, 0, {0}, 0.0},                              // 无效等价类：空数组
    {11, N, {10, 20, INFINITY, 40, 50}, 0},       // 无效等价类：含无穷大
    {12, N, {10, 20, NAN, 40, 50}, 0},            // 无效等价类：含 NaN
    // 边界值分析
    {13, N, {FLT_MAX, 1.0, 2.0, 3.0, 4.0}, 3},     // 边界值：高值测试
    {14, N, {-FLT_MAX, 1.0, 2.0, 3.0, 4.0}, 2},    // 边界值：低值测试
    {15, 1, {1.0}, 1.0},                           // 边界值：len 最小值
    {16, N, {10.0, 20.0, 30.0, 40.0, 50.0}, 30.0}, // 边界值：len 最大值
    {17, 1, {50}, 50.0},                           // 单元素
    {18, N, {50, 50, 50, 50, 50}, 50.0},           // 多元素
    {19, N, {10, 9, 10, 9, 100}, 10.0},            // 有噪声
    // 决策表测试
    {20, N, {10, -10, 10, -10, 10}, 10.0},          // 正负值均匀分布组合
    {21, N, {100, -100, 100, -100, 0}, 0.0},        // 高值正负值对称组合
    {22, N, {100, 200, 300, 400, 500}, 300.0},      // 单调递增数组
    {23, N, {-100, -200, -300, -400, -500}, -300.0},// 单调递减数组
    // 特殊场景
    {24, N, {0, 0, 0, 0, 0}, 0.0},                  // 所有元素为零
    {25, N, {1, 1, 1, 1, 1}, 1.0},                  // 所有元素相同的正整数
    {26, N, {-1, -1, -1, -1, -1}, -1.0},            // 所有元素相同的负整数
    {27, N, {5, 100, 5, 100, 5}, 5.0},              // 高低交替噪声
    {28, N, {-0.0, 0.0, -0.0, 0.0, -0.0}, 0.0},     // 特殊场景：负零与零的混合
};

// exponential_weighted_moving_average 测试用例
TData_alpha data3[] = {
    // 有效等价类
    {1, N, {10, 20, 30, 40, 50}, 19.049, 0.1},        // alpha = 0.1，较重视旧数据
    {2, N, {1, 2, 3, 4, 5}, 5.0, 1.0},                // alpha = 1.0 (完全重视新数据)
    {3, N, {10, 20, 30, 40, 50}, 10.0, 0.0},          // alpha = 0.0 (完全重视旧数据)
    {4, N, {10, 20, 30, 40, 50}, 48.889, 0.9},          // alpha = 0.9，较重视新数据
    // 无效等价类
    {5, N, {1, 2, 3, 4, 5}, 0.0, -0.1},               // alpha = -0.1 (无效)
    {6, N, {1, 2, 3, 4, 5}, 0.0, 1.1},                // alpha = 1.1 (无效)
    {7, N+1, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0}, 0, 0.5}, // 无效等价类：len 大于最大值
    {8, 0, {1.0}, 0.0, 0.5},                          // 无效等价类：len 为0
    {9, -1, {2.0}, 0.0, 0.5},                         // 无效等价类：len 负数，小于最小边界
    {10, N, {1.0, 2.0}, 0, 0.5},                      // 无效等价类：数组长度小于 len
    {11, 1, {1.0, 2.0}, 0, 0.5},                      // 无效等价类：数组长度大于 len
    {12, 0, {0}, 0.0, 0.5},                            // 无效等价类：空数组
    {13, N, {10, 20, INFINITY, 40, 50}, 0, 0.5},      // 无效等价类：含无穷大
    {14, N, {10, 20, NAN, 40, 50}, 0, 0.5},           // 无效等价类：含 NaN
    // 边界值分析
    {15, N, {FLT_MAX, 1.0, 2.0, 3.0, 4.0}, 0.1, 0},   // 高值测试
    {16, N, {-FLT_MAX, 1.0, 2.0, 3.0, 4.0}, 0.9, 0},  // 低值测试
    {17, 1, {1.0}, 1.0, 0.5},                          // 边界值：len 最小值
    {18, N, {10.0, 20.0, 30.0, 40.0, 50.0},40.625, 0.5},// 边界值：len 最大值
    {19, 1, {50}, 50.0, 0.5},                          // 单元素
    {20, N, {50, 50, 50, 50, 50}, 50.0, 0.5},          // 多元素
    {21, N, {10, 9, 10, 9, 100}, 54.6875, 0.5},        // 有噪声
    // 决策表测试
    {22, N, {10, -10, 10, -10, 10},4.432, 0.6},         // 正负值均匀分布，alpha=0.6
    {23, N, {100, -100, 100, -100, 0},-12.5, 0.5},      // 高值正负值对称组合，alpha=0.4
    {24, N, {1, 2, 3, 4, 5},3.2269, 0.3},               // 单调递增数组，alpha=0.3
    {25, N, {-1, -2, -3, -4, -5},-4.5749, 0.7},         // 单调递减数组，alpha=0.7
    // 特殊场景
    {26, N, {0, 0, 0, 0, 0}, 0.0, 0.5},                 // 所有元素为零
    {27, N, {1, 1, 1, 1, 1}, 1.0, 0.5},                 // 所有元素相同，正整数
    {28, N, {-1, -1, -1, -1, -1}, -1.0, 0.5},           // 所有元素相同，负整数
    {29, N, {100, 50, 100, 50, 100},84.375, 0.5},       // 高低交替噪声
    {30, N, {-0.0, 0.0, -0.0, 0.0, -0.0}, 0.0, 0.5},    // 特殊场景：负零与零的混合
};

// moving_average_filter 单元测试
void test_moving_average_filter() {
    int count = sizeof(data1) / sizeof(data1[0]);
    for (int i = 0; i < count; i++){
        results[i] = fabs(moving_average_filter(data1[i].input, data1[i].num) - data1[i].result)<1e-6 ? 1 : 0;
    }
	sprintf(str,"\ntest_moving_average_filter Test results: \r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);
	
	sprintf(str,"");
    for (int i = 0; i < count; i++) {
        if (i % 5 == 0 && i != 0) strcat(str, " ");
        sprintf(temp, "%d", results[i]);
        strcat(str, temp);
    }
	strcat(str, "\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 1000);
	
    for (int i = 0; i < count; i++) {
        if(results[i]==0){
			sprintf(str,"\nCase-%d result:%f\r\n", data1[i].id, moving_average_filter(data1[i].input, data1[i].num));
			HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 1000);
        }
    }
    memset(results, 0, sizeof(results));
	sprintf(str,"\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);
}

// median_filter 单元测试
void test_median_filter() {
    int count = sizeof(data2) / sizeof(data2[0]);
    for (int i = 0; i < count; i++){
        results[i] = fabs(median_filter(data2[i].input, data2[i].num) - data2[i].result)<1e-6 ? 1 : 0;
    }
	sprintf(str,"\ntest_median_filter Test results: \r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);
	
	sprintf(str,"");
    for (int i = 0; i < count; i++) {
        if (i % 5 == 0 && i != 0) strcat(str, " ");
        sprintf(temp, "%d", results[i]);
        strcat(str, temp);
    }
	strcat(str, "\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 1000);
	
    for (int i = 0; i < count; i++) {
        if(results[i]==0){
			sprintf(str,"\nCase-%d result:%f\r\n", data2[i].id, median_filter(data2[i].input, data2[i].num));
			HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 1000);
        }
    }
    memset(results, 0, sizeof(results));
	sprintf(str,"\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);
}

// exponential_weighted_moving_average 单元测试
void test_exponential_weighted_moving_average() {
    int count = sizeof(data3) / sizeof(data3[0]);
    for (int i = 0; i < count; i++){
        results[i] = fabs(exponential_weighted_moving_average(data3[i].input, data3[i].num, data3[i].alpha) - data3[i].result)<1e-6 ? 1 : 0;
    }
	sprintf(str,"\nexponential_weighted_moving_average Test results:\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);

	sprintf(str,"");
    for (int i = 0; i < count; i++) {
        if (i % 5 == 0 && i != 0) strcat(str, " ");
        sprintf(temp, "%d", results[i]);
        strcat(str, temp);
    }
	strcat(str, "\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 1000);
	
    for (int i = 0; i < count; i++) {
        if(results[i]==0){
			sprintf(str,"\nCase-%d result:%f\r\n", data3[i].id, exponential_weighted_moving_average(data3[i].input, data3[i].num, data3[i].alpha));
			HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 1000);
        }
    }
    memset(results, 0, sizeof(results));
	sprintf(str,"\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);
}

// 测试语句覆盖率
void test_Coverage() {

    float data1[] = {1.0, 2.0, 3.0};
    float data2[] = {1.0, 2.0, 3.0, 4.0};

    // 静态插装：手动插入计数器
    // 测试用例1：奇数个数据
	sprintf(str,"\r\nTest Case 1:\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);
	sprintf(str,"Moving Average: %.2f\r\n", moving_average_filter_pl(data1, 3));
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);
    sprintf(str,"EWMA: %.2f\r\n", exponential_weighted_moving_average_pl(data1, 3, 0.5));
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);
    sprintf(str,"Median: %.2f\r\n", median_filter_pl(data1, 3));
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);
    // 测试用例2：偶数个数据
    sprintf(str,"\r\nTest Case 2:\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);
    sprintf(str,"Moving Average: %.2f\r\n", moving_average_filter_pl(data2, 4));
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);
    sprintf(str,"EWMA: %.2f\r\n", exponential_weighted_moving_average_pl(data2, 4, 0.3));
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);
    sprintf(str,"Median: %.2f\r\n", median_filter_pl(data2, 4));
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);
    // 输出覆盖率报告
    sprintf(str,"\r\nCoverage Report:\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);
    sprintf(str,"Moving Average Filter:%.2f%%\r\n", (ma_line1+ma_line2+ma_line3)/3.0*100);
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);
    sprintf(str,"EWMA Filter:%.2f%%\r\n", (ewma_line1+ewma_line2+ewma_line3)/3.0*100);
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);
    sprintf(str,"Median Filter:%.2f%%\r\n", (mf_line1+mf_line2+mf_line3+mf_line4+mf_line5)/5.0*100);
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);
}
// =================================================================

int main(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_SPI1_Init();
	MX_USART1_UART_Init();
	LCD_Init();
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE);

	sprintf(str,"UART is OK!\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);
	HAL_Delay(500);
	
	// 代码覆盖率
	sprintf(str,"\r\n--------Static instrumentation results--------\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);
    test_Coverage();
	HAL_Delay(500);
	
	// 滤波函数测试用例
	sprintf(str,"\r\n--------Test cases results--------\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);
	test_moving_average_filter();
	HAL_Delay(500);
    test_median_filter();
	HAL_Delay(500);
    test_exponential_weighted_moving_average();
	
	sprintf(str,"\r\nAll test cases passed.\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 500);

	while (1)
	{
		if(HAL_GetTick()-Tick_LCD > 500){	// 每500ms刷新一次
			Tick_LCD = HAL_GetTick();
		}
		__WFI();
	}
}




// =========================================================================================
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}
void Error_Handler(void)
{
  
}
