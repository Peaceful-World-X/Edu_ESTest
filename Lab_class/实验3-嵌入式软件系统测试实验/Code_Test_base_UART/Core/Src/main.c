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
const char* filter_names[4] = {"unknown_filter", "moving_average_filter", "exponential_weighted_moving_average", "median_filter"};
void parse_string_to_struct(const char * str, TData* data) {

    const char *input_start;
    const char *input_end;
    int input_count = 0;
    
    // 第一步：匹配 data->id 和 data->num
    sscanf(str, "{ %d , %d ,", &data->id, &data->num);
    
    // 第二步：定位输入数组部分的开始和结束位置
    input_start = strchr(str+1, '{');
    input_end = strchr(input_start, '}');
	    
    // 第三步：循环匹配 data->input
    const char *current = input_start + 1; // 跳过 '{'
    while (current < input_end && input_count < MAX_INPUT_SIZE) {
        if (sscanf(current, " %f ,", &data->input[input_count]) == 1 ||
            sscanf(current, " %f ", &data->input[input_count]) == 1) {
            input_count++;
        }
        
        // 移动到下一个逗号或结束位置
        current = strchr(current, ',');
        if (current) {
            current++; // 跳过逗号
        } else {
            break;
        }
    }

    // 第四步：匹配 data->alpha, data->result, data->dex
    sscanf(input_end + 1, " , %f , %f , %d }", &data->alpha, &data->result, &data->dex);
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

	sprintf(str,"连接成功！\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);
	lcd_init();
	lcd_clear();
	HAL_Delay(500);
	
	while (1)
	{

		if(USART_RX_STA & 0x8000)       //串口接收到数据
		{
			if(USART_RX_BUF[0] == '{' && USART_RX_BUF[strlen((char*)USART_RX_BUF) - 1] == '}'){
				
				parse_string_to_struct((char *)USART_RX_BUF, &data);
				printf("Case-%d:%s\n",data.id, USART_RX_BUF);
				
				switch(data.dex){
					case 1:
						real_data = moving_average_filter(data.input, data.num);
						result = fabs(real_data - data.result)<1e-6 ? 1 : 0;
						if(result)	printf("Case-%d:PASS\n", data.id);
						else{
							printf("Case-%d:FAIL\n", data.id);
							printf("Case-%d result:%f\n", data.id, real_data);
						}		
					break;
					case 2:
						real_data = exponential_weighted_moving_average(data.input, data.num, data.alpha);
						result = fabs(real_data- data.result)<1e-6 ? 1 : 0;
						if(result)	printf("Case-%d:PASS\n", data.id);
						else{
							printf("Case-%d:FAIL\n", data.id);
							printf("Case-%d result:%f\n", data.id, real_data);
						}		
					break;
					case 3:
						real_data = median_filter(data.input, data.num);
						result = fabs(real_data - data.result)<1e-6 ? 1 : 0;
						if(result)	printf("Case-%d:PASS\n", data.id);
						else{
							printf("Case-%d:FAIL\n", data.id);
							printf("Case-%d result:%f\n", data.id, real_data);
						}		
					break;
					default:
						printf("滤波函数选择序号出错！\n");
						break;
				}
			}else{
				printf("Invalid input string format.\n");
			}
//			printf("测试用例ID: %d\n", data.id);
//			printf("输入数据长度: %d\n", data.num);
//			printf("输入数据数组: [");
//			for (int i = 0; i < data.num; ++i) {
//				if(i == data.num-1) 
//					 printf("%.1f"  , data.input[i]);
//				else printf("%.1f, ", data.input[i]);
//			}printf("]\n");
//			printf("权重参数: %.2f\n", data.alpha);
//			printf("断言结果: %.2f\n", data.result);
//			const char* filter_name = (data.dex >= 1 && data.dex <= 3) ? filter_names[data.dex] : filter_names[0];
//			printf("滤波函数类别: %s\n", filter_name);  
			// 清空缓存区
			USART_RX_STA=0;
			memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
		}
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
