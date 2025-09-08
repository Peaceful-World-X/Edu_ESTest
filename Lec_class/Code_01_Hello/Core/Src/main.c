#include "main.h"           // 主头文件，通常包含STM32 HAL库基础定义和项目配置
#include "gpio.h"           // GPIO外设初始化函数声明头文件
#include "spi.h"            // SPI外设初始化函数声明头文件
#include "usart.h"          // USART（串口）外设初始化函数声明头文件

#include "bsp_lcd.h"        // LCD屏底层驱动函数声明（如显示字符、填充等）
#include "bsp_lcd_init.h"   // LCD屏初始化相关函数声明


// 函数声明：系统时钟配置函数（通常在main.c或system_stm32xxx.c中实现）
void SystemClock_Config(void);

// 定义一个字符数组，作为字符串缓存区，最大可存储49个字符+1个结束符
char str[50];   


// 主函数，程序入口
int main(void)
{
  HAL_Init();              // 初始化HAL库（包括滴答定时器、中断优先级分组等）
  SystemClock_Config();    // 配置系统时钟（如HSE/HSI、PLL、系统频率等）
  MX_GPIO_Init();          // 初始化项目中用到的GPIO引脚（如LED、按键、LCD控制线等）
    
  MX_USART1_UART_Init();   // 初始化USART1串口（用于串口通信，如打印调试信息）
  MX_SPI1_Init();          // 初始化SPI1接口（用于驱动LCD屏等SPI设备）
  LCD_Init();              // 初始化LCD显示屏（包括复位、发送初始化命令序列等）
  LCD_Fill(0,0,LCD_W,LCD_H,WHITE);  // 将整个LCD屏幕填充为白色（清屏操作）
                                    // 参数：起始X、起始Y、宽度、高度、颜色

  // 主循环：无限循环，持续运行程序
  while(1)
  {
     // 格式化字符串：将"Hello Tester!"写入str缓冲区
     sprintf(str,"Hello Tester!");

     // 在LCD屏幕上显示字符串
     // 参数：X坐标(8*0=0)，Y坐标(0)，字符串指针，前景色(RED)，背景色(WHITE)，字体大小(16)，模式(0)
     LCD_ShowString(8*0 ,0,(u8*)str,RED,WHITE,16,0);
      
     // 格式化字符串并添加换行符，用于串口输出
     sprintf(str,"Hello Tester!\r\n");

     // 通过USART1发送字符串到串口（如电脑串口助手）
     // 参数：串口句柄，数据指针，数据长度，超时时间（毫秒）
     HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 15);
      
     // 延时1000毫秒（1秒），控制刷新频率
     HAL_Delay(1000);
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
}
