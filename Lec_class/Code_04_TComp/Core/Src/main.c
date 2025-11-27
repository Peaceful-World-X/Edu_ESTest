#include "main.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"
#include "bsp_lcd.h"
#include "bsp_lcd_init.h"

#include "i2c.h"
#include "usart.h"
#include "gpio.h"
/*
方法A：寻找最奇特的数值
输入：一个数组长度 n（1 到 1000）
功能：从系统数据其中n个数据中挑选最奇特的一个数据
例子：1 5 -> 最奇特的数值 = 19

方法B：数据聚合处理
输入：一个处理范围 n（0 到 1000）
功能：对系统数据进行聚合运算，返回处理后的综合指标值
例子：2 100 -> 100 的聚合结果 = 15550

方法C：关联性分析
输入：一个分析深度 n（0 到 300）
功能：执行数据关联度计算，生成综合关联指标（Metric）
例子：3 50 -> 关联指标: 62475

==================================================
原理说明：
方法A：find_most_unique（寻找最奇特的数值）
时间复杂度：O(log n)
实现：在长度为 n 的有序子数组中使用二分查找定位最大值

方法B：aggregate_metric（汇总指标）
时间复杂度：O(n)
实现：线性扫描求和，遍历前 n 个元素并累加

方法C：simulate_interactions（相互作用模拟）
时间复杂度：O(n?)
实现：冒泡排序 + 两两配对计算差值（排序 O(n?)，成对累加 O(n?)）
*/
void SystemClock_Config(void);

#define USART_SEND_LEN 300
char str[USART_SEND_LEN];                   // 缓存字符串（用于sprintf和发送）
uint8_t re;                                 // 串口接收单字符

#define USART_REC_LEN   200                 // 最大接收长度（扩展，支持更长命令）
uint8_t USART_RX_BUF[USART_REC_LEN];        // 接收缓冲区
uint16_t USART_RX_STA = 0;                  // 接收状态标记（bit15=完成，bit14=收到\r）

// 测试数据数组
int test_array[1000];

// DWT 微秒级计时支持
static inline void DWT_Init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // 使能 DWT
    DWT->CYCCNT = 0;                                 // 复位计数器
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // 启动计数器
}

static inline uint32_t DWT_GetCycles(void) {
    return DWT->CYCCNT;
}

static inline uint32_t DWT_CyclesToMicroseconds(uint32_t cycles) {
    return cycles / (SystemCoreClock / 1000000);
}

/**
 * @brief Method A: 设备状态查询 - 二分查找定位（O(log n)复杂度）
 * @param target_value 要查找的目标值
 * @return 返回找到的索引位置，未找到返回-1
 */
// Method A: 设备状态查询（二分查找）
/**
 * @brief Method A: 寻找最奇特的数值 - 在长度为n的数组中二分查找（O(log n)复杂度）
 * @param n 数组长度，在前n个元素中查找最大值
 * @return 返回找到的最大值
 */
// Method A: 寻找最奇特的数值（二分查找）
int find_most_unique(int n) {
    if (n <= 0 || n > 1000) {
        return -1;
    }

    // 在长度为n的子数组中查找最大值（使用二分查找的方式）
    // 由于数组是递增的，最大值在最后，但我们用二分查找来定位
    int left = 0;
    int right = n - 1;
    int target = test_array[n - 1]; // 要查找的目标值（最大值）

    // 二分查找最大值
    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (test_array[mid] == target) {
            return test_array[mid]; // 找到目标值，返回值
        } else if (test_array[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return test_array[n - 1]; // 返回最大值
}

/**
 * @brief Method B: 汇总指标 - 线性扫描求和
 * @param n 要求和的元素个数
 * @return 返回前n个元素的和
 */
// Method B: 汇总指标（线性扫描）
int aggregate_metric(int n) {
    int sum = 0;
    if (n < 0 || n > 1000) {
        return -1;
    }
    for (int i = 0; i < n; i++) {
        sum += test_array[i];
    }
    return sum;
}

/**
 * @brief Method C: 相互作用模拟 - 两两比较并累加指标
 * @param n 要排序的元素个数
 * @return 排序完成返回0
 */
// Method C: 相互作用模拟（对比多项关系）
long long simulate_interactions(int n) {
    if (n < 0 || n > 1000) {
        return -1;
    }
    int temp_array[1000];
    // 复制数组避免修改原数组
    for (int i = 0; i < n; i++) {
        temp_array[i] = test_array[i];
    }
    // 冒泡排序
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (temp_array[j] > temp_array[j + 1]) {
                int temp = temp_array[j];
                temp_array[j] = temp_array[j + 1];
                temp_array[j + 1] = temp;
            }
        }
    }
    long long metric = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            long long diff = (long long)temp_array[i] - (long long)temp_array[j];
            if (diff < 0) diff = -diff;
            metric += diff; // 对每对元素计算绝对差并累加
        }
    }
    return metric;
}

/**
 * @brief 解析命令并执行相应的测试函数
 * @param input 输入字符串，格式: "方法编号 参数"
 * @param output 输出结果字符串
 * @return 0=成功, -1=格式错误
 */
int parse_and_execute(char *input, char *output) {
    int method, param;
    int result;
    long long resultLL = 0; // 用于存放可能的 64 位结果（方法C）

    // 解析输入格式: "方法编号 参数"
    int user_repeats = 0; // 可选的用户指定重复次数（保留兼容性）
    // 尝试解析输入，支持2个或3个参数
    int parsed = sscanf(input, "%d %d %d", &method, &param, &user_repeats);
    if (parsed < 2) {
        return -1;
    }

    // 根据方法编号执行对应函数并计时（接口隐藏具体复杂度）
    switch (method) {
        case 1: // Method A - 寻找最奇特的数值 (binary search)
            if (param < 1 || param > 1000) {
                sprintf(output, "[A]: Error - Array length out of range (1-1000)\r\n");
                break;
            }
            {
                // 重复执行10000次以获得可测量的时间差异
                volatile int tmp = 0;
                uint32_t start_cycles = DWT_GetCycles();
                for (int i = 0; i < 10000; i++) {
                    tmp += find_most_unique(param);
                }
                uint32_t end_cycles = DWT_GetCycles();
                uint32_t elapsed_us = DWT_CyclesToMicroseconds(end_cycles - start_cycles);

                result = find_most_unique(param); // 获取实际结果
                if (result == -1) {
                    sprintf(output, "[A]: Error processing array length %d\r\n", param);
                } else {
                    sprintf(output, "[A]: Most unique value is %d, Time: %lu us (10k loops)\r\n",
                            result, (unsigned long)elapsed_us);
                }
            }
            break;

        case 2: // Method B - aggregate metric
            if (param < 0 || param > 1000) {
                sprintf(output, "[B]: Error - Parameter out of range (0-1000)\r\n");
                break;
            }
            {
                uint32_t start_cycles = DWT_GetCycles();
                result = aggregate_metric(param);
                uint32_t end_cycles = DWT_GetCycles();
                uint32_t elapsed_us = DWT_CyclesToMicroseconds(end_cycles - start_cycles);

                if (result == -1) {
                    sprintf(output, "[B]: Error - Invalid parameter\r\n");
                } else {
                    sprintf(output, "[B]: Comprehensive value is %d, Time: %lu us\r\n", result, (unsigned long)elapsed_us);
                }
            }
            break;

        case 3: // Method C - simulate interactions
            if (param < 0 || param > 300) {
                sprintf(output, "[C]: Error - Parameter out of range (0-300)\r\n");
                break;
            }
            {
                uint32_t start_cycles = DWT_GetCycles();
                resultLL = simulate_interactions(param);
                uint32_t end_cycles = DWT_GetCycles();
                uint32_t elapsed_us = DWT_CyclesToMicroseconds(end_cycles - start_cycles);

                if (resultLL == -1) {
                    sprintf(output, "[C]: Error - Invalid parameter\r\n");
                } else {
                    sprintf(output, "[C]: The Metric is %lld, Time: %lu us\r\n", resultLL, (unsigned long)elapsed_us);
                }
            }
            break;

        default:
            sprintf(output, "Error: Invalid method number (use 1, 2, or 3)\r\n");
            return -1;
    }

    return 0;
}

// ============================================================================================
int main(void)
{
  HAL_Init();                               // 初始化HAL库
  SystemClock_Config();                     // 配置系统时钟
  DWT_Init();                               // 初始化DWT微秒级计时
  MX_GPIO_Init();                           // 初始化GPIO
  MX_I2C1_Init();                           // 初始化I2C（用于LCD）
  MX_USART1_UART_Init();                    // 初始化串口1
  MX_SPI1_Init();                           // 初始化 SPI1（通常用于驱动 LCD）
  LCD_Init();                               // 初始化 LCD 屏幕控制器
  // 清屏并绘制背景
  LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);      // 清全屏，用白色填充

  // 初始化测试数组
  for (int i = 0; i < 1000; i++) {
    test_array[i] = i * 3 + 7; // 递增序列: 7, 10, 13, ...
  }

  HAL_UART_Receive_IT(&huart1, (uint8_t *)(&re), 1); // 开启串口接收中断（每次收1字节）

  // 发送连接成功提示
  sprintf(str, "=== Time Complexity Test System ===\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);
    sprintf(str, "A - Find most unique\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);
    sprintf(str, "B - Aggregate metric\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);
    sprintf(str, "C - Simulation task\r\n\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);
  sprintf(str, "Usage: [method] [param]  (max input len %d)\r\n", USART_REC_LEN);
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);
  sprintf(str, "Example: 1 5  or  2 100  or  3 50 (<=300)\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);
  sprintf(str, "===================================\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);

  // === LCD 精彩显示 ===
  LCD_Fill(0, 0, LCD_W, 24, BLUE);
  sprintf(str, "Black-box Test");
  LCD_ShowString(8, 4, (u8*)str, YELLOW, BLUE, 16, 0);
  LCD_DrawLine(0, 26, LCD_W-1, 26, RED);
  LCD_DrawLine(0, 27, LCD_W-1, 27, RED);
  LCD_ShowString(4, 34, (u8*)"A: Probe", GREEN, WHITE, 16, 0);
  LCD_ShowString(4, 52, (u8*)"B: Aggregate", BLUE, WHITE, 16, 0);
  LCD_ShowString(4, 70, (u8*)"C: Metric", MAGENTA, WHITE, 16, 0);
  LCD_DrawRectangle(2, 30, LCD_W-3, 90, GRAYBLUE);
  LCD_ShowString(4, 96, (u8*)"Status: Ready", DARKBLUE, WHITE, 16, 0);
  LCD_Fill(0, LCD_H-4, LCD_W, LCD_H, LIGHTGREEN);

  while (1)
  {
    if (USART_RX_STA & 0x8000)              // 检测是否接收完成（bit15置1）
    {
        // 添加字符串结束符（确保安全）
        USART_RX_BUF[USART_RX_STA & 0x3FFF] = '\0';

        // 执行命令并获取结果
        if (parse_and_execute((char *)USART_RX_BUF, str) == 0) {
            // 执行成功，发送结果
            HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 10000);
        } else {
            // 格式错误，返回错误提示
            sprintf(str, "Error: Please input format '[method] [param]' (e.g., '1 5')\r\n");
            HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 10000);
        }

        // 重置接收状态和缓冲区
        USART_RX_STA = 0;                   // 清除接收完成标志
        memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF)); // 清空缓冲区
    }
  }
}

/**
 * 串口中断回调函数（通常在 stm32fxxx_it.c 中）
 * 如果你没实现，请确保在 stm32fxxx_it.c 中添加：
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        if ((USART_RX_STA & 0x8000) == 0) { // 接收未完成
            if (USART_RX_STA & 0x4000) {    // 已收到回车符 \r
                if (re == 0x0a) {           // 接着收到换行符 \n → 完成
                    USART_RX_STA |= 0x8000; // 标记接收完成
                } else {
                    USART_RX_STA = 0;       // 错误重置
                }
            } else {
                if (re == 0x0d) {           // 收到回车符 \r
                    USART_RX_STA |= 0x4000; // 标记已收\r
                } else {
                    USART_RX_BUF[USART_RX_STA & 0x3FFF] = re; // 存入缓冲区
                    USART_RX_STA++;         // 有效数据长度+1
                    if (USART_RX_STA > (USART_REC_LEN - 1)) USART_RX_STA = 0; // 防溢出
                }
            }
        }
        HAL_UART_Receive_IT(&huart1, (uint8_t *)(&re), 1); // 重新开启接收
    }
}

/*****************************************************************    以下代码可忽略  ******************************************************************/
/**
  * @brief System Clock Configuration
  * @retval None
  */
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

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}
