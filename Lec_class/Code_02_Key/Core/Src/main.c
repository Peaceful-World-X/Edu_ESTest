#include "main.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"
#include "bsp_lcd.h"
#include "bsp_lcd_init.h"


void SystemClock_Config(void);
uint8_t keyboard_scan(void);

char str[50];    									  //缓存字符串
uint8_t key_val;                                      // 存储当前扫描到的按键值
uint8_t key_old = 0;                                  // 存储上一次按键值，用于边沿检测
uint8_t key_down;         							  // 存储按下瞬间的按键值	
__IO uint32_t Tick_KEY = 0;                           // 记录上次按键扫描时间，用于去抖

int main(void)
{
  HAL_Init();                                           // 初始化 HAL 库（硬件抽象层）
  SystemClock_Config();                                 // 配置系统主频和时钟树
  MX_GPIO_Init();                                       // 初始化按键、LED 等 GPIO 引脚
  MX_USART1_UART_Init();                                // 初始化串口 USART1（用于调试输出）
  MX_SPI1_Init();                                       // 初始化 SPI1（通常用于驱动 LCD）
  LCD_Init();                                           // 初始化 LCD 屏幕控制器
  LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);                  // 清全屏，用白色填充
	
  char prompt[] = "Press any key!";                     // 定义开机提示语
  LCD_ShowString(0, 0, (u8*)prompt, RED, WHITE, 16, 0); // 在屏幕第一行显示提示语

  while(1)                                              // 主循环
  {
    if(HAL_GetTick() - Tick_KEY > 100){                 // 每100ms扫描一次（防抖）
        Tick_KEY = HAL_GetTick();                       // 更新时间戳
        key_val = keyboard_scan();                      // 获取当前按键
        key_down = key_val & (key_old ^ key_val);       // 边沿检测：刚按下的键
        key_old = key_val;                              // 更新历史值

        if (key_down >= 1 && key_down <= 16) {          // 如果是有效按键（1~16）
            LCD_Fill(0, 0, LCD_W, 18, WHITE);           // 清除第一行防残影
            sprintf(str, "Pressed Key: %d", key_down);  // 生成显示字符串
            LCD_ShowString(0, 0, (u8*)str, RED, WHITE, 16, 0); // 显示按键号
            strcat(str, "\r\n");                        // 添加换行符
            HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 15); // 串口发送
        }
    }
  }
}

// 扫描获取当前按键
uint8_t keyboard_scan(void)
{ 
    uint8_t press_key=0;
	HAL_GPIO_WritePin(KEY_L1_GPIO_Port,  KEY_L1_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(KEY_L2_GPIO_Port,  KEY_L2_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(KEY_L3_GPIO_Port,  KEY_L3_Pin,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KEY_L4_GPIO_Port,  KEY_L4_Pin,GPIO_PIN_RESET);
	if(HAL_GPIO_ReadPin(KEY_R1_GPIO_Port, KEY_R1_Pin))      press_key=1;
	else if(HAL_GPIO_ReadPin(KEY_R2_GPIO_Port, KEY_R2_Pin)) press_key=5;
	else if(HAL_GPIO_ReadPin(KEY_R3_GPIO_Port, KEY_R3_Pin)) press_key=9;
	else if(HAL_GPIO_ReadPin(KEY_R4_GPIO_Port, KEY_R4_Pin)) press_key=13;
	HAL_GPIO_WritePin(KEY_L1_GPIO_Port,  KEY_L1_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(KEY_L2_GPIO_Port,  KEY_L2_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(KEY_L3_GPIO_Port,  KEY_L3_Pin,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KEY_L4_GPIO_Port,  KEY_L4_Pin,GPIO_PIN_RESET);
	if(HAL_GPIO_ReadPin(KEY_R1_GPIO_Port, KEY_R1_Pin))      press_key=2;
	else if(HAL_GPIO_ReadPin(KEY_R2_GPIO_Port, KEY_R2_Pin)) press_key=6;
	else if(HAL_GPIO_ReadPin(KEY_R3_GPIO_Port, KEY_R3_Pin)) press_key=10;
	else if(HAL_GPIO_ReadPin(KEY_R4_GPIO_Port, KEY_R4_Pin)) press_key=14;
	HAL_GPIO_WritePin(KEY_L1_GPIO_Port,  KEY_L1_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(KEY_L2_GPIO_Port,  KEY_L2_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(KEY_L3_GPIO_Port,  KEY_L3_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(KEY_L4_GPIO_Port,  KEY_L4_Pin,GPIO_PIN_RESET);
	if(HAL_GPIO_ReadPin(KEY_R1_GPIO_Port, KEY_R1_Pin))      press_key=3;
	else if(HAL_GPIO_ReadPin(KEY_R2_GPIO_Port, KEY_R2_Pin)) press_key=7;
	else if(HAL_GPIO_ReadPin(KEY_R3_GPIO_Port, KEY_R3_Pin)) press_key=11;
	else if(HAL_GPIO_ReadPin(KEY_R4_GPIO_Port, KEY_R4_Pin)) press_key=15;
	HAL_GPIO_WritePin(KEY_L1_GPIO_Port,  KEY_L1_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(KEY_L2_GPIO_Port,  KEY_L2_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(KEY_L3_GPIO_Port,  KEY_L3_Pin,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KEY_L4_GPIO_Port,  KEY_L4_Pin,GPIO_PIN_SET);
	if(HAL_GPIO_ReadPin(KEY_R1_GPIO_Port, KEY_R1_Pin))      press_key=4;
	else if(HAL_GPIO_ReadPin(KEY_R2_GPIO_Port, KEY_R2_Pin)) press_key=8;
	else if(HAL_GPIO_ReadPin(KEY_R3_GPIO_Port, KEY_R3_Pin)) press_key=12;
	else if(HAL_GPIO_ReadPin(KEY_R4_GPIO_Port, KEY_R4_Pin)) press_key=16;
	return press_key;
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
