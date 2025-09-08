#include "main.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"
#include "bsp_lcd.h"
#include "bsp_lcd_init.h"

#include "i2c.h"
#include "usart.h"
#include "gpio.h"

void SystemClock_Config(void);

char str[50];                               // 缓存字符串（用于sprintf和发送）
uint8_t re;                                 // 串口接收单字符
#define USART_REC_LEN   50                  // 最大接收长度
uint8_t USART_RX_BUF[USART_REC_LEN];        // 接收缓冲区
uint16_t USART_RX_STA = 0;                  // 接收状态标记（bit15=完成，bit14=收到\r）

/**
 * @brief 解析 "m+n" 格式字符串并计算结果
 * @param input 输入字符串
 * @param output 输出结果字符串
 * @return 0=成功, -1=格式错误
 */
int parse_and_calculate(char *input, char *output) {
    int m, n;
    char op;
    // 尝试解析 "m+n" 格式（严格匹配一个加号，左右为整数）
    if (sscanf(input, "%d%c%d", &m, &op, &n) != 3 || op != '+') {
        return -1;                          // 格式错误
    }
    // 格式正确，生成结果字符串
    sprintf(output, "%d+%d=%d", m, n, m + n);
    return 0;
}

int main(void)
{
  HAL_Init();                               // 初始化HAL库
  SystemClock_Config();                     // 配置系统时钟
  MX_GPIO_Init();                           // 初始化GPIO
  MX_I2C1_Init();                           // 初始化I2C（用于LCD）
  MX_USART1_UART_Init();                    // 初始化串口1
  MX_SPI1_Init();                           // 初始化 SPI1（通常用于驱动 LCD）
  LCD_Init();                               // 初始化 LCD 屏幕控制器
  LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);      // 清全屏，用白色填充

  HAL_UART_Receive_IT(&huart1, (uint8_t *)(&re), 1); // 开启串口接收中断（每次收1字节）

  // 发送连接成功提示
  sprintf(str, "Connection OK!\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);
  sprintf(str, "Enter m+n byUART!");
  LCD_ShowString(0, 0, (u8*)str, RED, WHITE, 16, 0); 	// 在屏幕第一行显示提示语

  while (1)
  {
    if (USART_RX_STA & 0x8000)              // 检测是否接收完成（bit15置1）
    {
        // 添加字符串结束符（确保安全）
        USART_RX_BUF[USART_RX_STA & 0x3FFF] = '\0';

        // 准备响应字符串
        if (parse_and_calculate((char *)USART_RX_BUF, str) == 0) {
            // 计算成功，返回结果
            strcat(str, "\r\n");            // 添加换行
            HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 10000);
        } else {
            // 格式错误，返回错误提示
            sprintf(str, "Error: Please input like '1+1'\r\n");
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

/********************************    以下代码可忽略  *********************************/
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
