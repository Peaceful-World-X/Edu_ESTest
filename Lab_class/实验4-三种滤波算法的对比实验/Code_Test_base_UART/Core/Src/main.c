#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "i2c-lcd.h"
#include "filter.h"

void SystemClock_Config(void);

#define MAX_INPUT_SIZE 100

char str[50];       //缓存字符
uint8_t re;         //串口接收字符
#define USART_REC_LEN   200     //定义最大接收字节数 200
uint8_t USART_RX_BUF[USART_REC_LEN];      //接收缓冲,最大USART_REC_LEN个字节.
uint16_t USART_RX_STA=0;       //接收状态标记
//bit15，	接收完成标志
//bit14，	接收到0x0d 回车是把光标置于本行行首 换行\n是把光标置于下一行的同一列
//bit13~0，	接收到的有效字节数目

// =======================================测试定义==================================================
TData data;
int result;
float real_data;

// 分别调用三个滤波器
void apply_filters() {
    // 输入数据数组，包含20个测量值和两个显著高峰
    float input_data[] = {1.0, 1.1, 1.2, 1.3, 1.4, 10.0, 1.5, 1.6, 1.7, 1.8, 
                          2.0, 2.1, -20.0, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8};
    int len = sizeof(input_data) / sizeof(input_data[0]);

    // 滑动窗口大小
    int window_size = N;

    // 创建三个数组存放滤波后的结果
    float moving_average_result[len];
    float ewma_result[len];
    float median_result[len];

    // 对输入数据进行移动平均滤波
    for (int i = 0; i < len; i++) {
        if (i < window_size - 1) {
            // 滤波窗口还无法满窗，直接使用原始数据
            moving_average_result[i] = input_data[i];
        } else {
            // 从当前窗口计算移动平均值
            moving_average_result[i] = moving_average_filter(&input_data[i - window_size + 1], window_size);
        }
    }

    // 对输入数据进行指数加权移动平均滤波
    float current_ewma = input_data[0];
    ewma_result[0] = current_ewma;  // 初始化
    for (int i = 1; i < len; i++) {
        current_ewma = 0.3 * input_data[i] + (1 - 0.3) * current_ewma;
        ewma_result[i] = current_ewma;
    }

    // 对输入数据进行中值滤波
    for (int i = 0; i < len; i++) {
        if (i < window_size - 1) {
            // 滤波窗口还无法满窗，直接使用原始数据
            median_result[i] = input_data[i];
        } else {
            // 从当前窗口计算中值
            median_result[i] = median_filter(&input_data[i - window_size + 1], window_size);
        }
    }

    // 输出移动平均滤波结果
    printf("移动平均滤波结果:\n");
    for (int i = 0; i < len; i++) {
        printf("%.2f ", moving_average_result[i]);
    }
    printf("\n");

    // 输出指数加权移动平均滤波结果
    printf("指数加权移动平均滤波结果:\n");
    for (int i = 0; i < len; i++) {
        printf("%.2f ", ewma_result[i]);
    }
    printf("\n");

    // 输出中值滤波结果
    printf("中值滤波结果:\n");
    for (int i = 0; i < len; i++) {
        printf("%.2f ", median_result[i]);
    }
    printf("\n");
}

// =======================================测试定义==================================================
int main(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_I2C1_Init();
	MX_USART1_UART_Init();

	uint32_t tick = HAL_GetTick();
	HAL_UART_Receive_IT(&huart1, (uint8_t *)(&re), 1);

	printf("连接成功！\n");
	lcd_init();
	lcd_clear();
	HAL_Delay(500);
	
	apply_filters();
	
	
	
	while (1)
	{

	}
}



//*串口接收中断回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{      
    if((USART_RX_STA & 0x8000)==0){             //接收未完成
        if(USART_RX_STA & 0x4000){              //接收到了
            if(re==0x0a) USART_RX_STA|=0x8000;  //又接收到了\n 接收完成了！
            else USART_RX_STA=0;                //接收错误,重新开始
        }else{	
            if(re==0x0d) USART_RX_STA|=0x4000;  //接收到了
            else{
                USART_RX_BUF[USART_RX_STA&0X3FFF]=re ;//保存数据
                USART_RX_STA++;
                if(USART_RX_STA>(USART_REC_LEN-1)) USART_RX_STA=0;//接收数据溢出,重新开始接收	  
                }
            }
    }
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&re, 1);
}

// =======================================下面的代码不用管==================================================
/******************************************************************************************/
/* 加入以下代码, 支持printf函数, 而不需要选择use MicroLIB */

#if 1
#if (__ARMCC_VERSION >= 6010050)                    /* 使用AC6编译器时 */
__asm(".global __use_no_semihosting\n\t");          /* 声明不使用半主机模式 */
__asm(".global __ARM_use_no_argv \n\t");            /* AC6下需要声明main函数为无参数格式，否则部分例程可能出现半主机模式 */

#else
/* 使用AC5编译器时, 要在这里定义__FILE 和 不使用半主机模式 */
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
    /* Whatever you require here. If the only file you are using is */
    /* standard output using printf() for debugging, no file handling */
    /* is required. */
};

#endif

/* 不使用半主机模式，至少需要重定义_ttywrch\_sys_exit\_sys_command_string函数,以同时兼容AC6和AC5模式 */
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

/* 定义_sys_exit()以避免使用半主机模式 */
void _sys_exit(int x)
{
    x = x;
}

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}

/* FILE 在 stdio.h里面定义. */
FILE __stdout;

/* 重定义fputc函数, printf函数最终会通过调用fputc输出字符串到串口 
其中串口可根据实际使用情况调整 */
int fputc(int ch, FILE *f)
{
    while ((USART1->SR & 0X40) == 0);             /* 等待上一个字符发送完成 */

    USART1->DR = (uint8_t)ch;                     /* 将要发送的字符 ch 写入到DR寄存器 */
    return ch;
}
#endif
/***********************************************END*******************************************/


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

/* USER CODE BEGIN 4 */


void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}
