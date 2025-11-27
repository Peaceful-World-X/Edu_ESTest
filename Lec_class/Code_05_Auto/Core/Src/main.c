#include "main.h"
#include "gpio.h"
#include "spi.h"
#include "i2c.h"
#include "usart.h"
#include "bsp_lcd.h"
#include "bsp_lcd_init.h"

void SystemClock_Config(void);
/*
 * ============================================================================
 * 温度传感器数据处理系统 - 代码覆盖率测试项目
 * ============================================================================
 *
 * 功能说明：
 * 本程序实现一个温度传感器数据处理系统，包含多种工作模式和复杂的分支逻辑，
 * 专为代码覆盖率测试设计。测试者需要设计测试用例来覆盖所有分支。
 *
 * 测试目标：
 * - 使用等价类划分、边界值分析等黑盒测试方法设计测试用例
 * - 手动插桩统计代码覆盖率（语句覆盖、分支覆盖、路径覆盖）
 * - 分析未覆盖分支，补充测试用例
 * - 测试函数的时间特性（执行时间、性能分析）
 *
 * 插桩说明：
 * - 全局变量 branch_coverage[50] 已声明，用于记录每个分支执行次数
 * - 测试者需要在关键分支处手动添加：branch_coverage[i]++;
 * - 建议在每个if/else、switch/case、循环等分支入口处插桩
 *
 * 命令使用：
 * TEST [mode] [temp] [humidity] [time]   - 执行传感器数据处理
 * COVERAGE                               - 显示覆盖率统计报告
 * RESET                                  - 重置覆盖率计数器
 *
 * 提示：时间性能测试由测试者自己在parse_and_execute函数中编写代码实现
 */

#define USART_SEND_LEN 300
char str[USART_SEND_LEN];                   // 缓存字符串（用于sprintf和发送）
uint8_t re;                                 // 串口接收单字符
#define USART_REC_LEN   200                 // 最大接收长度（扩展，支持更长命令）
uint8_t USART_RX_BUF[USART_REC_LEN];        // 接收缓冲区
uint16_t USART_RX_STA = 0;                  // 接收状态标记（bit15=完成，bit14=收到\r）

// 代码覆盖率统计变量（用于插桩）
uint32_t branch_coverage[50] = {0};         // 记录每个分支的执行次数

// 微秒/毫秒级计时函数声明
static inline void DWT_Init(void);
static inline uint32_t Get_us(void);
static inline uint32_t Get_ms(void);

// ==================================================================================================================================
/**
 * ============================================================================
 * 代码覆盖率测试专用函数：温度传感器数据处理系统
 * ============================================================================
 * @brief 温度传感器数据处理与告警系统（多分支结构，适合代码覆盖率测试）
 *
 * 输入格式：TEST mode temp humidity time
 * @param mode      工作模式 (1=正常监测, 2=节能模式, 3=高精度模式, 4=校准模式)
 * @param temp      温度值 (-50 到 150，单位：℃)
 * @param humidity  湿度值 (0 到 100，单位：%)
 * @param time      时间戳 (0-86400，单位：秒，表示一天内的秒数)
 * @return 返回处理结果代码
 *
 * 返回值说明：
 *  100+ : 正常状态码
 *  200+ : 警告状态码
 *  300+ : 错误状态码
 *  400+ : 严重告警状态码
 *
 * ============================================================================
 * 测试用例设计指南（黑盒测试方法）
 * ============================================================================
 *
 * 1. 等价类划分测试用例
 * ----------------------
 * mode参数等价类：【提示：有效等价类(1,2,3,4) vs 无效等价类(≤0, ≥5)】
 * temp参数等价类：【提示：有效范围[-50,150] vs 无效范围(<-50, >150)；内部分5个温度级别】
 * humidity参数等价类：【提示：有效范围[0,100] vs 无效范围(<0, >100)；内部分3个湿度级别】
 * time参数等价类：【提示：有效范围[0,86400] vs 无效范围(<0, >86400)；内部分日间/夜间】
 *
 * 2. 边界值分析测试用例
 * ----------------------
 * temp边界值：-51(无效), -50(最小), -49(最小+1), -11(临界-1), -10(临界), -9(临界+1),
 *             9(临界-1), 10(临界), 11(临界+1), 34(临界-1), 35(临界), 36(临界+1),
 *             49(临界-1), 50(临界), 51(临界+1), 149(最大-1), 150(最大), 151(无效)
 * humidity边界值：-1(无效), 0(最小), 1(最小+1), 29(临界-1), 30(临界), 31(临界+1),
 *                 69(临界-1), 70(临界), 71(临界+1), 99(最大-1), 100(最大), 101(无效)
 * time边界值：-1(无效), 0(最小), 1(最小+1), 21599(临界-1), 21600(临界), 21601(临界+1),
 *             64799(临界-1), 64800(临界), 64801(临界+1), 86399(最大-1), 86400(最大), 86401(无效)
 * mode边界值：0(无效), 1(最小), 4(最大), 5(无效)
 *
 * 3. 组合条件测试用例（判定覆盖/条件覆盖）
 * ------------------------------------------
 * 【提示：需要测试复合条件的各种组合，确保每个逻辑运算符(AND/OR)的所有分支都被覆盖】
 * 高温+高湿组合：temp_level>=2 AND humidity_level==2
 * 夜间+极端温度组合：is_daytime==0 AND temp_level==3
 * 节能模式嵌套条件：is_daytime==0 AND temp_level==0
 * 高精度模式双边界：temp∈[15,30] AND humidity∈[40,60]
 * 校准模式双边界：temp∈[23,27] AND humidity∈[45,55]
 *
 * 4. 路径覆盖测试用例
 * 【提示：从函数入口到出口的完整执行路径，需要设计用例覆盖所有可能的执行路径】
 * 【提示：重点关注switch-case的4个分支，以及每个case内部的嵌套if-else结构】
 *
 * 5. 推荐的最小测试用例集（覆盖所有分支）
 * 【提示：以下是覆盖主要分支的建议测试用例，实际测试时可根据需要补充】
 * 【提示：以上仅为基础用例，完整的分支覆盖还需要补充更多边界值和组合条件测试】
 */
int process_sensor_data(int mode, int temp, int humidity, int time) {
    int result_code = 0;
    int is_daytime = 0;
    int temp_level = 0;
    int humidity_level = 0;

    // 参数有效性检查
    if (temp < -50 || temp > 150) {
        return 301;
    }

    if (humidity < 0 || humidity > 100) {
        return 302;
    }

    if (time < 0 || time > 86400) {
        return 303;
    }

    // 时间段判断
    if (time >= 21600 && time <= 64800) {
        is_daytime = 1;
    } else {
        is_daytime = 0;
    }

    // 温度等级分类
    if (temp < -10) {
        temp_level = 3;
    } else if (temp < 10) {
        temp_level = 1;
    } else if (temp <= 35) {
        temp_level = 0;
    } else if (temp <= 50) {
        temp_level = 2;
    } else {
        temp_level = 3;
    }

    // 湿度等级分类
    if (humidity < 30) {
        humidity_level = 1;
    } else if (humidity <= 70) {
        humidity_level = 0;
    } else {
        humidity_level = 2;
    }

    // 工作模式处理
    switch (mode) {
        case 1:
            result_code = 100;

            if (temp_level == 3) {
                result_code = 401;
            } else if (temp_level == 2) {
                result_code = 201;
            } else if (temp_level == 1) {
                result_code = 202;
            }

            if (temp_level == 0) {
                if (humidity_level == 2) {
                    result_code = 203;
                } else if (humidity_level == 1) {
                    result_code = 204;
                }
            }

            if (temp_level >= 2 && humidity_level == 2) {
                result_code = 402;
            }
            break;

        case 2:
            result_code = 110;

            if (is_daytime == 0) {
                if (temp_level == 0) {
                    result_code = 111;
                } else {
                    result_code = 211;
                }
            } else {
                result_code = 212;
            }
            break;

        case 3:
            result_code = 120;

            if (temp >= 15 && temp <= 30) {
                if (humidity >= 40 && humidity <= 60) {
                    result_code = 121;
                } else {
                    result_code = 221;
                }
            } else {
                result_code = 222;
            }
            break;

        case 4:
            result_code = 130;

            if (temp >= 23 && temp <= 27) {
                if (humidity >= 45 && humidity <= 55) {
                    result_code = 131;
                } else {
                    result_code = 231;
                }
            } else {
                result_code = 232;
            }
            break;

        default:
            return 304;
    }

    // 特殊组合检测
    if (is_daytime == 0 && temp_level == 3) {
        result_code = 403;
    }

    return result_code;
}

/**
 * @brief 解析命令并执行测试函数
 * @param input 输入字符串，格式: "TEST mode temp humidity time"
 * @param output 输出结果字符串
 * @return 0=成功, -1=格式错误
 *
 * 提示：如需测试时间性能，可在此函数中添加计时代码：
 */
int parse_and_execute(char *input, char *output) {
    int mode, temp, humidity, time;
    int parsed = sscanf(input, "TEST %d %d %d %d", &mode, &temp, &humidity, &time);

    if (parsed != 4) {
        sprintf(output, "Error: Format should be TEST [mode] [temp] [humidity] [time]\r\n");
        return -1;
    }

    int result = process_sensor_data(mode, temp, humidity, time);
    sprintf(output, "Result: Code=%d\r\n", result);
    return 0;
}

/**
 * @brief 打印代码覆盖率统计报告
 * @param output 输出缓冲区
 *
 * 说明：测试者需要根据插桩数量修改 total_branches 的值
 */
void print_coverage_report(char *output) {
    int total_branches = 50; // TODO: 根据实际插桩数量修改此值
    int covered = 0;

    // 统计已覆盖的分支数
    for (int i = 0; i < total_branches; i++) {
        if (branch_coverage[i] > 0) {
            covered++;
        }
    }

    // 计算覆盖率
    float coverage_rate = (float)covered / total_branches * 100.0f;

    sprintf(output, "\r\n=== Code Coverage Report ===\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t *)output, strlen(output), 1000);

    sprintf(output, "Total Branches: %d\r\n", total_branches);
    HAL_UART_Transmit(&huart1, (uint8_t *)output, strlen(output), 1000);

    sprintf(output, "Covered Branches: %d\r\n", covered);
    HAL_UART_Transmit(&huart1, (uint8_t *)output, strlen(output), 1000);

    sprintf(output, "Uncovered: %d\r\n", total_branches - covered);
    HAL_UART_Transmit(&huart1, (uint8_t *)output, strlen(output), 1000);

    sprintf(output, "Coverage Rate: %.2f%%\r\n", coverage_rate);
    HAL_UART_Transmit(&huart1, (uint8_t *)output, strlen(output), 1000);
}


// ==================================================================================================================================
int main(void)
{
  HAL_Init();                               // 初始化HAL库
  SystemClock_Config();                     // 配置系统时钟
  MX_GPIO_Init();                           // 初始化GPIO
  MX_I2C1_Init();                           // 初始化I2C（用于LCD）
  MX_USART1_UART_Init();                    // 初始化串口1
  MX_SPI1_Init();                           // 初始化SPI1（用于驱动LCD）
  LCD_Init();                               // 初始化LCD屏幕控制器
  LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);      // 清屏

  HAL_UART_Receive_IT(&huart1, (uint8_t *)(&re), 1); // 开启串口接收中断

  // 串口提示信息
  sprintf(str, "=== Code Coverage Test System ===\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);
  sprintf(str, "Commands: TEST/COVERAGE/RESET\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);
  sprintf(str, "Format: TEST [mode] [temp] [humidity] [time]\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);

  // LCD静态显示
  LCD_ShowString(10, 10, (u8*)"Coverage Test", BLUE, WHITE, 16, 0);
  LCD_ShowString(10, 30, (u8*)"System Ready", BLACK, WHITE, 16, 0);

  while (1)
  {
    if (USART_RX_STA & 0x8000)              // 检测是否接收完成
    {
        USART_RX_BUF[USART_RX_STA & 0x3FFF] = '\0';

        if (strncmp((char *)USART_RX_BUF, "COVERAGE", 8) == 0) {
            print_coverage_report(str);
            HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 10000);
        } else if (strncmp((char *)USART_RX_BUF, "RESET", 5) == 0) {
            memset(branch_coverage, 0, sizeof(branch_coverage));
            sprintf(str, "[RESET]: Coverage counters reset\r\n");
            HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 10000);
        } else {
            if (parse_and_execute((char *)USART_RX_BUF, str) == 0) {
                HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 10000);
            } else {
                sprintf(str, "Error: Invalid command\r\n");
                HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 10000);
            }
        }

        USART_RX_STA = 0;
        memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
    }
  }
}

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

/**
 * @brief DWT计时器初始化
 */
static inline void DWT_Init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // 使能 DWT
    DWT->CYCCNT = 0;                                 // 复位计数器
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // 启动计数器
}

/**
 * @brief 将DWT周期数转换为微秒
 * @return 微秒数
 */
static inline uint32_t DWT_CyclesToMicroseconds(void) {
    return DWT->CYCCNT / (SystemCoreClock / 1000000);
}

/**
 * @brief 获取微秒级时间戳
 * @return 当前微秒计数
 *
 * 使用示例：
 *   uint32_t start = Get_us();
 *   process_sensor_data(1, 25, 60, 43200);
 *   uint32_t elapsed = Get_us() - start;
 *   sprintf(str, "Time: %u us\r\n", elapsed);
 */
static inline uint32_t Get_us(void) {
    return DWT_CyclesToMicroseconds();
}

/**
 * @brief 获取毫秒级时间戳
 * @return 当前毫秒计数
 *
 * 使用示例：
 *   uint32_t start = Get_ms();
 *   process_sensor_data(1, 25, 60, 43200);
 *   uint32_t elapsed = Get_ms() - start;
 *   sprintf(str, "Time: %u ms\r\n", elapsed);
 */
static inline uint32_t Get_ms(void) {
    return HAL_GetTick();
}
