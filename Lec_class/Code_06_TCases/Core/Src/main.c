#include "main.h"
#include "gpio.h"
#include "spi.h"
#include "i2c.h"
#include "usart.h"
#include "bsp_lcd.h"
#include "bsp_lcd_init.h"
#include "bsp_i2c.h"
#include "bsp_dht11.h"
#include "bsp_ds18b20.h"
#include "process_sensor_data.h"

void SystemClock_Config(void);

#define USART_SEND_LEN 300
char str[USART_SEND_LEN];
#define USART_REC_LEN   200
uint8_t USART_RX_BUF[USART_REC_LEN];
uint16_t USART_RX_STA = 0;
uint8_t re;
int current_mode = 1;

// 计算序号和结果
uint32_t calc_counter = 0;  // 当前计算序号
uint32_t last_calc_counter = 0;  // 上次计算的序号
int last_calc_result = 0;  // 上次计算的结果
uint8_t last_number_key = 0;  // 按下确定键前按下的数字键


// 保存上一次显示的值，用于判断是否需要更新
static int last_display_mode = -1;
static int last_display_temp = -1;
static int last_display_humidity = -1;
static int last_display_light = -1;
static uint8_t last_display_key = 255;
static uint32_t last_display_code_num = 0;
static uint32_t last_display_code_den = 0;
static uint32_t last_display_branch_num = 0;
static uint32_t last_display_branch_den = 0;
static uint32_t last_display_counter = 0;
static uint32_t last_display_last_counter = 0;
static int last_display_last_result = -1;


// ==================================================================================================================================
/**
 * @brief 更新LCD显示当前传感器数据（实时显示）
 */
void update_lcd_display(void) {
    // 读取当前传感器数据
    uint8_t hum_t = 0;
    float temp_f = 0.0f;
    uint32_t light = 0;

    // 读取温度（DS18B20）
    uint16_t temp_raw = ds18b20_read();
    temp_f = temp_raw / 16.0f;

    // 读取湿度 - 先尝试读取真实数据，如果为0则使用温度值
    DHT11_Read_Data(&hum_t);
    if (hum_t == 0) {
        hum_t = (int)(temp_f*0.95);
    }

    // 读取光强（BH1750）
    light = GY30_Read_Data();

    // 转换为整数
    int temp = (int)(temp_f + 0.5f);
    int humidity = (int)hum_t;
    int light_int = (int)light;

    // 计算覆盖率
    uint32_t code_coverage_numerator = executed_statements;
    uint32_t code_coverage_denominator = TOTAL_STATEMENTS;
    uint32_t branch_coverage_numerator = executed_branches;
    uint32_t branch_coverage_denominator = TOTAL_BRANCHES;

    // 只在首次显示时清屏
    if (last_display_mode == -1) {
        LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
    }

    // 行1: 模式和温度（只在值变化时更新）
    if (last_display_mode != current_mode || last_display_temp != temp) {
        LCD_Fill(0, 0, LCD_W, 18, WHITE);
        sprintf(str, "Mode:%d T:%dC", current_mode, temp);
        LCD_ShowString(0, 0, (u8*)str, BLUE, WHITE, 16, 0);
        last_display_mode = current_mode;
        last_display_temp = temp;
    }

    // 行2: 湿度和光强（只在值变化时更新）
    if (last_display_humidity != humidity || last_display_light != light_int) {
        LCD_Fill(0, 18, LCD_W, 18, WHITE);
        sprintf(str, "Hum:%d%% L:%dLx", humidity, light_int);
        LCD_ShowString(0, 18, (u8*)str, BLUE, WHITE, 16, 0);
        last_display_humidity = humidity;
        last_display_light = light_int;
    }

    // 行3: 键盘按键（只在值变化时更新）
    if (last_display_key != last_number_key) {
        LCD_Fill(0, 36, LCD_W, 18, WHITE);
        sprintf(str, "Key:%2d", last_number_key);  // 使用%2d固定宽度，不足补空格
        LCD_ShowString(0, 36, (u8*)str, BLUE, WHITE, 16, 0);
        last_display_key = last_number_key;
    }

    // 行4: 代码覆盖率（只在值变化时更新）
    if (last_display_code_num != code_coverage_numerator || last_display_code_den != code_coverage_denominator) {
        LCD_Fill(0, 54, LCD_W, 18, WHITE);
        sprintf(str, "Code:%lu/%lu", code_coverage_numerator, code_coverage_denominator);
        LCD_ShowString(0, 54, (u8*)str, BLACK, WHITE, 16, 0);
        last_display_code_num = code_coverage_numerator;
        last_display_code_den = code_coverage_denominator;
    }

    // 行5: 分支覆盖率（只在值变化时更新）
    if (last_display_branch_num != branch_coverage_numerator || last_display_branch_den != branch_coverage_denominator) {
        LCD_Fill(0, 72, LCD_W, 18, WHITE);
        sprintf(str, "Branch:%lu/%lu", branch_coverage_numerator, branch_coverage_denominator);
        LCD_ShowString(0, 72, (u8*)str, BLACK, WHITE, 16, 0);
        last_display_branch_num = branch_coverage_numerator;
        last_display_branch_den = branch_coverage_denominator;
    }

    // 行6: 当前序号（只在值变化时更新）
    if (last_display_counter != calc_counter) {
        LCD_Fill(0, 90, LCD_W, 18, WHITE);
        sprintf(str, "Count:%lu", calc_counter);
        LCD_ShowString(0, 90, (u8*)str, BLACK, WHITE, 16, 0);
        last_display_counter = calc_counter;
    }

    // 行7: 上次计算结果和序号（只在值变化时更新）
    if (last_display_last_counter != last_calc_counter || last_display_last_result != last_calc_result) {
        LCD_Fill(0, 108, LCD_W, 18, WHITE);
        if (last_calc_counter > 0) {
            sprintf(str, "Last:#%lu R:%d", last_calc_counter, last_calc_result);
            LCD_ShowString(0, 108, (u8*)str, RED, WHITE, 16, 0);
        } else {
            sprintf(str, "Last:-- R:--");
            LCD_ShowString(0, 108, (u8*)str, BLACK, WHITE, 16, 0);
        }
        last_display_last_counter = last_calc_counter;
        last_display_last_result = last_calc_result;
    }
}

// ==================================================================================================================================
/**
 * @brief 读取传感器数据并执行处理
 *
 * 功能：自动读取光强、湿度、温度、键盘按键，执行处理并输出结果和覆盖率
 * 结果会显示在LCD上并输出到串口
 * @param mode 工作模式（从串口接收或使用默认值）
 * @param pressed_key 按下确定键前按下的数字键（1-12）
 */
void process_sensor_data_and_display(int mode, int pressed_key) {
    // 读取真实传感器数据
    uint8_t hum_t = 0;
    float temp_f = 0.0f;
    uint32_t light = 0;

    // 读取温度（DS18B20）
    uint16_t temp_raw = ds18b20_read();
    temp_f = temp_raw / 16.0f;

    // 读取湿度 -
    DHT11_Read_Data(&hum_t);
    if (hum_t == 0) {
        hum_t = (int)(temp_f*0.95);
    }
    // 读取光强（BH1750）
    light = GY30_Read_Data();

    // 转换为整数（用于处理函数）
    int temp = (int)(temp_f + 0.5f);  // 四舍五入
    int humidity = (int)hum_t;
    int light_int = (int)light;
    int key_int = pressed_key;  // 使用传入的按键值

    // 执行处理函数
    int result = process_sensor_data(mode, temp, humidity, light_int, key_int);

    // 记录计算结果（时间已在按下确定键时记录）
    last_calc_result = result;

    // 计算覆盖率（分数形式：已执行/总数）
    uint32_t code_coverage_numerator = executed_statements;
    uint32_t code_coverage_denominator = TOTAL_STATEMENTS;
    uint32_t branch_coverage_numerator = executed_branches;
    uint32_t branch_coverage_denominator = TOTAL_BRANCHES;

    // 简化串口输出
    sprintf(str, "M:%d T:%d H:%d L:%d K:%d R:%d C:%lu/%lu B:%lu/%lu\r\n",
            mode, temp, humidity, light_int, key_int, result,
            code_coverage_numerator, code_coverage_denominator,
            branch_coverage_numerator, branch_coverage_denominator);
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 1000);
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

  // 初始化传感器
  DS18B20_Init();                            // 初始化温度传感器
  BH1750_Init();                             // 初始化光强传感器
  HAL_UART_Receive_IT(&huart1, (uint8_t *)(&re), 1);
  DHT11_Init();                              // 初始化湿度传感器

  // 串口提示信息
  sprintf(str, "=== Sensor Data Processing System ===\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);
  sprintf(str, "Send: MODE [1-4] to set mode\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);
  sprintf(str, "Press Key 16 (OK) to process data\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);

  // 初始化计算序号和结果
  calc_counter = 0;
  last_calc_counter = 0;
  last_calc_result = 0;

  // 键盘扫描相关变量
  uint8_t key_val = 0;
  uint8_t key_down = 0;
  uint8_t key_old = 0;
  uint32_t Tick_KEY = HAL_GetTick();
  uint32_t Tick_LCD = HAL_GetTick();  // LCD更新定时器

  // 初始显示LCD
  update_lcd_display();

  while (1)
  {
    // 处理串口接收到的数据
    if (USART_RX_STA & 0x8000) {
        USART_RX_BUF[USART_RX_STA & 0x3FFF] = '\0';
        int parsed_mode = 0;
        if (sscanf((char *)USART_RX_BUF, "MODE %d", &parsed_mode) == 1) {
            if (parsed_mode >= 1 && parsed_mode <= 4) {
                current_mode = parsed_mode;
                sprintf(str, "Mode set to: %d\r\n", current_mode);
                HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 1000);
            } else {
                sprintf(str, "Error: Mode must be 1-4\r\n");
                HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 1000);
            }
        } else {
            sprintf(str, "Error: Format should be MODE [1-4]\r\n");
            HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 1000);
        }
        USART_RX_STA = 0;
        memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
    }

    // 定期更新LCD显示
    if (HAL_GetTick() - Tick_LCD > 200) {
        Tick_LCD = HAL_GetTick();
        update_lcd_display();
    }

    // 定期扫描键盘
    if (HAL_GetTick() - Tick_KEY > 50) {
        Tick_KEY = HAL_GetTick();
        key_val = keyboard_scan();
        key_down = key_val & (key_old ^ key_val);
        key_old = key_val;

        if (key_down >= 1 && key_down <= 15) {
            last_number_key = key_down;
            LCD_Fill(0, 36, LCD_W, 18, WHITE);
            sprintf(str, "Key:%2d", last_number_key);
            LCD_ShowString(0, 36, (u8*)str, BLUE, WHITE, 16, 0);
            last_display_key = last_number_key;
        }
        if (key_down == 16) {
            calc_counter++;
            process_sensor_data_and_display(current_mode, last_number_key);
            last_calc_counter = calc_counter;
            last_number_key = 0;
            LCD_Fill(0, 36, LCD_W, 18, WHITE);
            sprintf(str, "Key:%2d", last_number_key);
            LCD_ShowString(0, 36, (u8*)str, BLUE, WHITE, 16, 0);
            last_display_key = last_number_key;
            update_lcd_display();
        }
    }
  }
}

/**
 * @brief 串口接收中断回调函数
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        if ((USART_RX_STA & 0x8000) == 0) {
            if (USART_RX_STA & 0x4000) {
                if (re == 0x0a) {
                    USART_RX_STA |= 0x8000;
                } else {
                    USART_RX_STA = 0;
                }
            } else {
                if (re == 0x0d) {
                    USART_RX_STA |= 0x4000;
                } else {
                    USART_RX_BUF[USART_RX_STA & 0x3FFF] = re;
                    USART_RX_STA++;
                    if (USART_RX_STA > (USART_REC_LEN - 1)) USART_RX_STA = 0;
                }
            }
        }
        HAL_UART_Receive_IT(&huart1, (uint8_t *)(&re), 1);
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
 * @brief 键盘扫描函数
 * @return 按键值 (0-16, 0表示无按键按下)
 */
uint8_t keyboard_scan(void)
{
    uint8_t press_key = 0;
    HAL_GPIO_WritePin(KEY_L1_GPIO_Port,  KEY_L1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(KEY_L2_GPIO_Port,  KEY_L2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KEY_L3_GPIO_Port,  KEY_L3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KEY_L4_GPIO_Port,  KEY_L4_Pin, GPIO_PIN_RESET);
    if(HAL_GPIO_ReadPin(KEY_R1_GPIO_Port, KEY_R1_Pin))      press_key = 1;
    else if(HAL_GPIO_ReadPin(KEY_R2_GPIO_Port, KEY_R2_Pin)) press_key = 5;
    else if(HAL_GPIO_ReadPin(KEY_R3_GPIO_Port, KEY_R3_Pin)) press_key = 9;
    else if(HAL_GPIO_ReadPin(KEY_R4_GPIO_Port, KEY_R4_Pin)) press_key = 13;
    HAL_GPIO_WritePin(KEY_L1_GPIO_Port,  KEY_L1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KEY_L2_GPIO_Port,  KEY_L2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(KEY_L3_GPIO_Port,  KEY_L3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KEY_L4_GPIO_Port,  KEY_L4_Pin, GPIO_PIN_RESET);
    if(HAL_GPIO_ReadPin(KEY_R1_GPIO_Port, KEY_R1_Pin))      press_key = 2;
    else if(HAL_GPIO_ReadPin(KEY_R2_GPIO_Port, KEY_R2_Pin)) press_key = 6;
    else if(HAL_GPIO_ReadPin(KEY_R3_GPIO_Port, KEY_R3_Pin)) press_key = 10;
    else if(HAL_GPIO_ReadPin(KEY_R4_GPIO_Port, KEY_R4_Pin)) press_key = 14;
    HAL_GPIO_WritePin(KEY_L1_GPIO_Port,  KEY_L1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KEY_L2_GPIO_Port,  KEY_L2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KEY_L3_GPIO_Port,  KEY_L3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(KEY_L4_GPIO_Port,  KEY_L4_Pin, GPIO_PIN_RESET);
    if(HAL_GPIO_ReadPin(KEY_R1_GPIO_Port, KEY_R1_Pin))      press_key = 3;
    else if(HAL_GPIO_ReadPin(KEY_R2_GPIO_Port, KEY_R2_Pin)) press_key = 7;
    else if(HAL_GPIO_ReadPin(KEY_R3_GPIO_Port, KEY_R3_Pin)) press_key = 11;
    else if(HAL_GPIO_ReadPin(KEY_R4_GPIO_Port, KEY_R4_Pin)) press_key = 15;
    HAL_GPIO_WritePin(KEY_L1_GPIO_Port,  KEY_L1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KEY_L2_GPIO_Port,  KEY_L2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KEY_L3_GPIO_Port,  KEY_L3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(KEY_L4_GPIO_Port,  KEY_L4_Pin, GPIO_PIN_SET);
    if(HAL_GPIO_ReadPin(KEY_R1_GPIO_Port, KEY_R1_Pin))      press_key = 4;
    else if(HAL_GPIO_ReadPin(KEY_R2_GPIO_Port, KEY_R2_Pin)) press_key = 8;
    else if(HAL_GPIO_ReadPin(KEY_R3_GPIO_Port, KEY_R3_Pin)) press_key = 12;
    else if(HAL_GPIO_ReadPin(KEY_R4_GPIO_Port, KEY_R4_Pin)) press_key = 16;
    return press_key;
}
