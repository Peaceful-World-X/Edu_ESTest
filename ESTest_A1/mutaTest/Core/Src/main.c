#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "spi.h"
#include "tim.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "BSP_lcd_init.h"
#include "BSP_lcd.h"
#include "BSP_pic.h"
#include "BSP_i2c.h"
#include "BSP_ds18b20.h"
#include "BSP_dht11.h"
#include "BSP_filter.h"

#define MAX_ARRAY_SIZE 100
typedef struct TestData{
    float input1[MAX_ARRAY_SIZE];
	int16_t len1;
    float input2[MAX_ARRAY_SIZE];
	int16_t len2;
	float alpha;       		
}TData;
/* USER CODE END Includes */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
_Bool parse_string_to_struct(const char * str, TData* data);

TData data;
float_t Tem;          
float_t Lig;           
float_t Tem_Res;       
float_t Lig_Res;       
MovingAverageFilter Hum_Data;
KalmanFilter Tem_Data;
MedianFilter Lig_Data;
_Bool Flag_Tem=0;       
_Bool Flag_Lig=0;       
_Bool Flag_Uart_dis=1;
_Bool Flag_source=1;

__IO uint32_t Tick_sys[10]; 
__IO uint32_t Tick_LCD;
__IO uint32_t Tick_KEY;
__IO uint32_t UPT;

char str[50];  
uint8_t key_val, key_down=0, key_old;
uint8_t Tick_1s=0, Tick_300ms=0;
uint8_t LED=0;         
_Bool Flag_filter=0;   
uint8_t EEPROM_write[1]={0xAA};
uint8_t EEPROM_read[1]={0};

#define total_time   1000   
uint16_t idle_time=0;     
float_t cpu_usage;

char str[50];      
uint8_t re;        
#define USART_REC_LEN   200    
uint8_t USART_RX_BUF[USART_REC_LEN];    
uint16_t USART_RX_STA=0;     
uint8_t num_test[1];


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();
	
	/* Configure the system clock */
	SystemClock_Config();
	
	/* USER CODE BEGIN SysInit */
	MX_SPI1_Init();
	LCD_Init();
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
	LCD_ShowString(0 ,18*0,(u8*)"System Init...",BLACK,WHITE,16,0);
	
	/* USER CODE END SysInit */
	
	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_I2C1_Init();
	MX_USART1_UART_Init();
	MX_I2C2_Init();
	MX_SPI1_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_TIM4_Init();
	MAF_Init(&Hum_Data);
	MDF_Init(&Lig_Data);
	KLF_init(&Tem_Data, 25, 1.0, 0.9, 0.1, 1.0, 1.0);
	HAL_UART_Receive_IT(&huart1, (uint8_t *)(&re), 1);
	printf("连接成功！\n");
	DS18B20_Init();
	BH1750_Init();
	EEPROM_Init();
	HAL_TIM_Base_Start_IT(&htim2);        
	HAL_TIM_Base_Start_IT(&htim3);        
	HAL_TIM_Base_Start_IT(&htim4);      
	LCD_ShowString(0 ,18*1,(u8*)"System Init OK!",RED,WHITE,16,0);
	HAL_Delay(500);
	Write_24c(EEPROM_write, 10, 1);
	HAL_Delay(50);
	Read_24c(EEPROM_read, 10, 1);
	while(EEPROM_read[0] != EEPROM_write[0]){
	  Read_24c(EEPROM_read, 10, 1);
	  LCD_ShowString(0 ,18*2,(u8*)"EEPROM Error!",RED,WHITE,16,0);
	}
	LCD_ShowString(0 ,18*2,(u8*)"EEPROM OK!     ",RED,WHITE,16,0);
	Read_24c(num_test, 3, 1);	
	num_test[0]++; HAL_Delay(50);
	Write_24c(num_test, 3, 1);
	while(!(Flag_Tem && Flag_Lig)){
	  if(!Flag_Tem){
		  Tem = ds18b20_read()/16.0;
		  if(Tem>10)   {
			Flag_Tem = 1;
			LCD_ShowString(0 ,18*3,(u8*)"DS18B20 OK!",RED,WHITE,16,0);  
		  }else Flag_Tem = 0;
	  }
	  if(!Flag_Lig){
		  Lig = GY30_Read_Data();
		  if(Lig<10000)   {
			Flag_Lig = 1;
			LCD_ShowString(0 ,18*4,(u8*)"BH1750 OK!",RED,WHITE,16,0);  
		  }else Flag_Lig = 0;       
	  }
	}
	LCD_ShowString(0 ,18*6,(u8*)"Entering ...",RED,WHITE,16,0);
	HAL_Delay(1000);
	if(num_test[0]>10) num_test[0]++;
	Write_24c(num_test, 3, 1);
	/* Infinite loop */
	Tick_LCD = Tick_KEY = HAL_GetTick();
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
	while (1)
	{
		if(HAL_GetTick()-Tick_KEY > 100){
			Tick_KEY = HAL_GetTick();
			key_val = keyboard_scan();
			key_down= key_val & (key_old ^ key_val);
			key_old = key_val;
			
			if(key_down) printf("按键 %d 按下！\n",key_down);
			switch(key_down){
				case 1:             
					Flag_filter ^= 1;
				break;
				case 2:                      
					Flag_Lig ^= 1;
					Flag_Tem ^= 1;
				break;
				case 3:                        
					Flag_Uart_dis ^= 1;
				break;
				case 4:                     
					Flag_source ^=1;
				break;
				default:break;}
		}
		if(HAL_GetTick()-Tick_LCD > 800){
			Tick_LCD = HAL_GetTick();
			UPT = uwTick/1000;if(UPT>65) UPT=60;
			sprintf(str,"UPT:%ds",UPT);LCD_ShowString(8*0 ,0,(u8*)str,RED,WHITE,16,0);
			(Flag_filter==1)?sprintf(str,"F:Yes"):sprintf(str,"F:No ");
			LCD_ShowString(8*10 ,0,(u8*)str,RED,WHITE,16,0);
			sprintf(str,"CPU:%04.1f%%",cpu_usage);
			LCD_ShowString(0 ,18*1,(u8*)str,RED,WHITE,16,0);
			sprintf(str,"N:%d",num_test[0]);
			LCD_ShowString(8*10 ,18*1,(u8*)str,RED,WHITE,16,0);
			sprintf(str,"LED:%d",LED);
			LCD_ShowString(0 ,18*2,(u8*)str,RED,WHITE,16,0);
			(Flag_source==0)?sprintf(str,"S:Uart"):sprintf(str,"S:Mod ");
			LCD_ShowString(8*10 ,18*2,(u8*)str,RED,WHITE,16,0);
			sprintf(str,"T:%5.2fC", Tem);	
			LCD_ShowString(0  ,18*3,(u8*)str,BLUE,WHITE,16,0);
			sprintf(str,"T_Res:%5.2fC", Tem_Res);
			LCD_ShowString(0  ,18*4,(u8*)str,RED,WHITE,16,0);
			if(Flag_Lig) sprintf(str,"L:%.0fLx     ", Lig);
			else         {sprintf(str,"L:Error"); Lig=0;}
			LCD_ShowString(0  ,18*5,(u8*)str,BLUE,WHITE,16,0);
			sprintf(str,"L_Res:%5.2fLx     ", Lig_Res);
			LCD_ShowString(0  ,18*6,(u8*)str,RED,WHITE,16,0);
			if(Flag_Uart_dis){
				sprintf(str,"T:%5.2fC L:%.0fLx\r\n",Tem,Lig);
				HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 50);
			}
		}
		if(USART_RX_STA & 0x8000){  
			if(USART_RX_BUF[0] == '{'){	
				if(parse_string_to_struct((char *)USART_RX_BUF, &data)){
					printf("\n\ninput1: ");
					for (int i = 0; i < data.len1; i++) { 
						printf("%.2f ", data.input1[i]);
					}
					printf("\nLength of input1: %d\n", data.len1);

					printf("input2: ");
					for (int i = 0; i < data.len2; i++) { 
						printf("%.2f ", data.input2[i]);
					}
					printf("\nLength of input2: %d\n", data.len2);
					printf("alpha: %.2f\n", data.alpha);
					
					if(data.len1<5 || data.len2<5){
						printf("数组长度太短（len>=5）\n");
					}else{
						Tem = moving_average_filter(data.input1, data.len1);
						Lig = exponential_weighted_moving_average(data.input2, data.len1 ,data.alpha);
						printf("\n【T:%05.2fC       L:%05.2fLx】\n",Tem,Lig);
						Tem_Res = F_Result(Tem);
						Lig_Res = L_Result(Lig);
						printf("【T_Res:%05.2fC   L_Res:%05.2fLx】\n",Tem_Res,Lig_Res);
					}
				}
				else printf("Invalid input string format.\n");
			}
			else printf("Invalid input string format.\n");
			USART_RX_STA=0;
			memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
		}
		__WFI();
	}
}

/******************************************************************************************/
_Bool parse_string_to_struct(const char * str, TData* data) {
	const char *input_start;
    const char *input_end;
    int input_count1 = 0, input_count2 = 0;
    memset(data, 0, sizeof(TData));
    input_start = strchr(str, '{'); 
	input_start++; 
    if(!input_start) {return 0;} 
    input_start++; 
    input_end = strchr(input_start, '}'); 
    if (!input_end) {return 0; }
    const char *current = input_start;
	while (current < input_end && input_count1 < MAX_ARRAY_SIZE) {
		if (sscanf(current, " %f", &data->input1[input_count1]) == 1) {
			input_count1++;
		}
		current = strchr(current, ','); 
		if (current) {
			current++; 
		} else {
			break;
		}
    }
	data->len1 = input_count1;

    input_start = strchr(input_end + 1, '{');
    if(!input_start) {return 0;} 
    input_start++; 

    input_end = strchr(input_start, '}'); 
    if (!input_end) {return 0; }

    current = input_start;
	while (current < input_end && input_count2 < MAX_ARRAY_SIZE) {
		if (sscanf(current, " %f", &data->input2[input_count2]) == 1) {
			input_count2++;
		}
		current = strchr(current, ','); 
		if (current) {
			current++; 
		} else {
			break;
		}
    }
	data->len2 = input_count2;
    if (sscanf(input_end + 1, " , %f }", &data->alpha) != 1) {
        return 0; 
    }

    return 1; 
}

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
	else if(HAL_GPIO_ReadPin(KEY_R4_GPIO_Port, KEY_R4_Pin)) press_key=15;
	return press_key;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == &htim4){        
        idle_time++;
    }
    if(htim == &htim3){       
        if(LED==2)  HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);  
        if(++Tick_300ms == 3){  
            Tick_300ms = 0;

			if(Flag_source){
				if(Flag_Tem){
					Tem = ds18b20_read()/16.0;
					if(Flag_filter){
						KLF(&Tem_Data, Tem);
						Tem = Tem_Data.x;
					}  
				}
				Tem_Res = F_Result(Tem);
				if(Flag_Lig){
					Lig = GY30_Read_Data();
					if(Flag_filter){
						Lig = MDF(&Lig_Data, Lig);
					} 
				}
				Lig_Res = L_Result(Lig);
			}

            if(!(Flag_Lig&&Flag_Tem))     LED = 1;
            else if(Lig>1000 || Tem>27)   LED = 2;
            else LED =0;
        }
    }
    if(htim == &htim2){        
        if(LED==1)  HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin); 
        if(++Tick_1s == 2){    
            Tick_1s = 0;
            cpu_usage = 100 - (idle_time * 100.0 / total_time);  
            idle_time = 0;
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{      
    if((USART_RX_STA & 0x8000)==0){            
        if(USART_RX_STA & 0x4000){              
            if(re==0x0a) USART_RX_STA|=0x8000;  
            else USART_RX_STA=0;              
        }else{	
            if(re==0x0d) USART_RX_STA|=0x4000; 
            else{
                USART_RX_BUF[USART_RX_STA&0X3FFF]=re ;
                USART_RX_STA++;
                if(USART_RX_STA>(USART_REC_LEN-1)) USART_RX_STA=0;
                }
            }
    }
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&re, 1);
}


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
