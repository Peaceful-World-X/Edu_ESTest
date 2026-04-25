/* USER CODE BEGIN Header */
/**
STM32F103C8 主流高性能系列，Arm Cortex-M3 MCU，具有64KB Flash，20KB SRAM，72MHz CPU，

      光强 Lig 中值滤波         0~65535 lx
      湿度 Hum 移动平均滤波     5%RH―95%RH
      温度 Tem 卡尔曼滤波       －10～＋85℃
      
      Program Size: Code=18640 RO-data=9100 RW-data=60 ZI-data=1740  
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "BSP_lcd_init.h"
#include "BSP_lcd.h"
#include "BSP_pic.h"
#include "BSP_i2c.h"
#include "BSP_ds18b20.h"
#include "BSP_dht11.h"
#include "BSP_filter.h"

//USART1_TX     PA10
//USART1_RX     PA9

// EEPROM_SCL   PB6
// EEPROM_SDA   PB7

// BH1750_SCL   PB10
// BH1750_SDA   PB11

// DHT11      PB0
// DS18B20    PB1
// LED        PC13

// KEY_L1     PB3
// KEY_L2     PB4
// KEY_L3     PB8
// KEY_L4     PB9
// KEY_R1     PA0
// KEY_R2     PC15
// KEY_R3     PC14
// KEY_R4     PA11 

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t Hum_t;
float_t Hum;            //记录湿度
float_t Tem;            //记录温度
uint16_t Lig;           //记录光强
MovingAverageFilter Hum_Data;
KalmanFilter Tem_Data;
MedianFilter Lig_Data;
_Bool Flag_Hum=0;       //设备正常否
_Bool Flag_Tem=0;       //设备正常否
_Bool Flag_Lig=0;       //设备正常否

__IO uint32_t Tick_sys[10];  // 初始化时长;自检时长;采集三个时长
__IO uint32_t Tick_LCD;
__IO uint32_t Tick_KEY;
__IO uint32_t Tick_UART;

char str[96];    //缓存字符串
uint8_t key_val, key_down=0, key_old;
uint8_t Tick_1s=0, Tick_300ms=0;
uint8_t LED=0;          //LED状态  0-灭 1-500ms闪烁 2-100ms闪烁
_Bool Flag_filter=0;    //是否滤波
uint8_t EEPROM_write[1]={0xAA};
uint8_t EEPROM_read[1]={0};

#define total_time   1000   // 总时间（基于定时器的周期）
uint16_t idle_time=0;       // 累计 CPU 空闲时间
float_t cpu_usage;

#define USART_REC_LEN   200             //定义最大接收字节数 200
uint8_t USART_RX_BUF[USART_REC_LEN];    //接收缓冲,最大USART_REC_LEN个字节
uint8_t re;                             //串口接收字符
uint16_t USART_RX_STA=0;                //接收状态标记
uint8_t num_test[1];

#define UART1_TX_BUF_SIZE 512
static uint8_t uart1_tx_buf[UART1_TX_BUF_SIZE];
static volatile uint16_t uart1_tx_head = 0;
static volatile uint16_t uart1_tx_tail = 0;
static volatile uint8_t uart1_tx_busy = 0;

static void UART1_Send(const uint8_t *data, uint16_t len);

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  Tick_sys[0] = HAL_GetTick();                  // 初始化开始------------------------------------------
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
  /* USER CODE BEGIN 2 */
  
  // 数据滤波配置
  MAF_Init(&Hum_Data);
  MDF_Init(&Lig_Data);
  KLF_init(&Tem_Data, 25, 1.0, 0.9, 0.1, 1.0, 1.0);

    HAL_UART_Receive_IT(&huart1, (uint8_t *)(&re), 1);
  sprintf(str,"串口连接成功！\r\n");
    UART1_Send((const uint8_t *)str, (uint16_t)strlen(str));
  DS18B20_Init();
  BH1750_Init();
  EEPROM_Init();
  DHT11_Init();
  Tick_sys[0] = HAL_GetTick() - Tick_sys[0];    // 初始化完成-----------------------------------------
  LCD_ShowString(0 ,18*1,(u8*)"System Init OK!",RED,WHITE,16,0);
  
  LCD_ShowString(0 ,18*2,(u8*)"Mod Selftest...",BLACK,WHITE,16,0);
  Tick_sys[1] = HAL_GetTick();                  // 模块自检开始-----------------------------------------
  Write_24c(EEPROM_write, 10, 1);
  HAL_Delay(50);
  Read_24c(EEPROM_read, 10, 1);
  while(EEPROM_read[0] != EEPROM_write[0]){
      Read_24c(EEPROM_read, 10, 1);
      LCD_ShowString(0 ,18*3,(u8*)"EEPROM Error!",RED,WHITE,16,0);
  }
  LCD_ShowString(0 ,18*3,(u8*)"EEPROM OK!",RED,WHITE,16,0);


  while(!(Flag_Hum && Flag_Tem && Flag_Lig)){
      if(!Flag_Hum){
          DHT11_Read_Data(&Hum_t);
          if(Hum_t>0)   {
            Flag_Hum = 1;
            LCD_ShowString(0 ,18*6,(u8*)"DHT11 OK!",RED,WHITE,16,0);  
          }else Flag_Hum = 0;       
      }
      if(!Flag_Tem){
          Tem = ds18b20_read()/16.0;
          if(Tem>10)   {
            Flag_Tem = 1;
            LCD_ShowString(0 ,18*4,(u8*)"DS18B20 OK!",RED,WHITE,16,0);  
          }else Flag_Tem = 0;
      }
      if(!Flag_Lig){
          Lig = GY30_Read_Data();
          if(Lig<10000)   {
            Flag_Lig = 1;
            LCD_ShowString(0 ,18*5,(u8*)"BH1750 OK!",RED,WHITE,16,0);  
          }else Flag_Lig = 0;       
      }
  }

  Read_24c(num_test, 11, 1);
//  num_test[0]=0;
  num_test[0] ^= 1;
  Write_24c(num_test, 11, 1);

  if(num_test[0]%2){
    LCD_ShowString(12*8 ,18*5,(u8*)"TEST",WHITE,RED,16,0);
  }else{
    LCD_ShowString(12*8 ,18*5,(u8*)"USER",WHITE,RED,16,0);
  }

  Tick_sys[1] = HAL_GetTick() - Tick_sys[1];    // 模块自检完成-----------------------------------------
  LCD_ShowString(0 ,18*7,(u8*)"Selftest OK!",RED,WHITE,16,0);
    sprintf(str,"Boot Time -> Init:%lums Selftest:%lums\r\n", Tick_sys[0], Tick_sys[1]);
    UART1_Send((const uint8_t *)str, (uint16_t)strlen(str));
  HAL_Delay(1500);
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  
  HAL_TIM_Base_Start_IT(&htim2);        // 500ms定时器 
  HAL_TIM_Base_Start_IT(&htim3);        // 100ms定时器
  HAL_TIM_Base_Start_IT(&htim4);        // 1ms定时器
    Tick_LCD = Tick_KEY = Tick_UART = HAL_GetTick();
  LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
  
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    if(HAL_GetTick()-Tick_KEY > 100){
        Tick_KEY = HAL_GetTick();
        key_val = keyboard_scan();
        key_down= key_val & (key_old ^ key_val);
        key_old = key_val;
        
        
        switch(key_down){
            case 1:                         // 是否滤波
                Flag_filter ^= 1;
            break;
            case 2:
                Flag_Lig ^= 1;
            break;
            case 3:
                Flag_Tem ^= 1;
            break;
            case 4:
                Flag_Hum ^= 1;
            break;
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            break;
            case 10:
            break;
            case 14:
            break;
            case 15:

            break; 
            case 11:

            break;
            case 12:

            break;
            case 13:

            break;
            case 16:
            break;
            default:break;}
    }

    if(HAL_GetTick()-Tick_LCD > 300){
        Tick_LCD = HAL_GetTick();
        uint32_t startTime = Tick_LCD;
        
        if(num_test[0]%2){
            sprintf(str,"UPT:%ds",uwTick/1000);
            LCD_ShowString(8*0 ,0,(u8*)str,RED,WHITE,16,0);
            (Flag_filter==1)?sprintf(str,"F:Yes"):sprintf(str,"F:No ");
            LCD_ShowString(8*10 ,0,(u8*)str,RED,WHITE,16,0);
            sprintf(str,"T:%2d-%2d-%2d",Tick_sys[0], Tick_sys[1], Tick_sys[5]);
            LCD_ShowString(0 ,18*1,(u8*)str,RED,WHITE,16,0);
            sprintf(str,"T:%2d-%2d-%2d",Tick_sys[2], Tick_sys[3], Tick_sys[4]);
            LCD_ShowString(0 ,18*2,(u8*)str,RED,WHITE,16,0);
            
            sprintf(str,"CPU:%05.2f%%",cpu_usage);
            LCD_ShowString(0 ,18*3,(u8*)str,RED,WHITE,16,0);
            
            if(Flag_Hum) sprintf(str,"H:%4.1f%%", Hum);
            else        {sprintf(str,"H:Error"); Hum=0;}
            LCD_ShowString(0  ,18*5,(u8*)str,BLUE,WHITE,16,0);
            
            if(Flag_Tem) sprintf(str,"T:%5.2fC", Tem);
            else         {sprintf(str,"T:Error"); Tem=0;}
            LCD_ShowString(8*8,18*5,(u8*)str,BLUE,WHITE,16,0);
        }else{
            if(Flag_Hum) sprintf(str,"H:%4.1f%%", Hum);
            else        {sprintf(str,"H:Error"); Hum=0;}
            LCD_ShowString(0  ,18*1,(u8*)str,BLUE,WHITE,16,0);
            
            if(Flag_Tem) sprintf(str,"T:%5.2fC", Tem);
            else         {sprintf(str,"T:Error"); Tem=0;}
            LCD_ShowString(0  ,18*3,(u8*)str,BLUE,WHITE,16,0);  
        }
        
        if(Flag_Lig) sprintf(str,"L:%dLx", Lig);
        else         {sprintf(str,"L:Error"); Lig=0;}
        if(Lig<100&&Lig!=0) LCD_Fill(8*8, 18*6,LCD_W,80,WHITE);
        else if(Lig<1000)   LCD_Fill(8*9, 18*6,LCD_W,80,WHITE);
        else if(Lig<10000)  LCD_Fill(8*10,18*6,LCD_W,80,WHITE);
        LCD_ShowString(0  ,18*6,(u8*)str,BLUE,WHITE,16,0);

        Tick_sys[5] = HAL_GetTick() - startTime;
    }

    if(HAL_GetTick()-Tick_UART > 500){
        Tick_UART = HAL_GetTick();
        sprintf(str,"H:%4.1f%% T:%5.2fC L:%dLx F:%d Ctrl:%lums CPU:%05.2f%%\r\n",
            Hum, Tem, Lig, Flag_filter, Tick_sys[5], cpu_usage);
        UART1_Send((const uint8_t *)str, (uint16_t)strlen(str));
    }
   if(USART_RX_STA & 0x8000){
       _Bool *pFlag = NULL;
       char sensor = 0;
       char value = 0;

       if ((USART_RX_BUF[1] == '=') && (USART_RX_BUF[3] == '\0') &&
           ((USART_RX_BUF[2] == '0') || (USART_RX_BUF[2] == '1'))) {
           sensor = (char)USART_RX_BUF[0];
           value = (char)USART_RX_BUF[2];

           if (sensor == 'H') pFlag = &Flag_Hum;
           else if (sensor == 'T') pFlag = &Flag_Tem;
           else if (sensor == 'L') pFlag = &Flag_Lig;
           else if (sensor == 'F') pFlag = &Flag_filter;

           if (pFlag != NULL) {
               *pFlag = (value == '1') ? 1 : 0;
               sprintf(str, "CMD OK: %c=%c\r\n", sensor, value);
               UART1_Send((const uint8_t *)str, (uint16_t)strlen(str));
           } else {
               sprintf(str, "CMD ERR: use H/T/L/F=0/1\r\n");
               UART1_Send((const uint8_t *)str, (uint16_t)strlen(str));
           }
       } else {
           sprintf(str, "CMD ERR: use H/T/L/F=0/1\r\n");
           UART1_Send((const uint8_t *)str, (uint16_t)strlen(str));
       }

       USART_RX_STA=0;
       memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
   }
   __WFI();
  }
  /* USER CODE END 3 */
}

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

/* USER CODE BEGIN 4 */

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

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == &htim4){         //1ms循环
        idle_time++;
    }
    if(htim == &htim3){         //100ms循环
        if(LED==2)  HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);  //翻转LED灯的状态
        if(++Tick_300ms == 3){  //300ms循环
            Tick_300ms = 0;
            
            Tick_sys[2] = HAL_GetTick();
            if(Flag_Hum){
                DHT11_Read_Data(&Hum_t);
                Hum = Hum_t;
                if(Flag_filter){
                    Hum = MAF(&Hum_Data, Hum_t);
                }   
            }
            Tick_sys[2] = HAL_GetTick() - Tick_sys[2]; 
            
            Tick_sys[3] = HAL_GetTick();
            if(Flag_Tem){
                Tem = ds18b20_read()/16.0;
                if(Flag_filter){
                    KLF(&Tem_Data, Tem);
                    Tem = Tem_Data.x;
                }  
            }
            Tick_sys[3] = HAL_GetTick() - Tick_sys[3]; 
            
            Tick_sys[4] = HAL_GetTick();
            if(Flag_Lig){
                Lig = GY30_Read_Data();
                if(Flag_filter){
                    Lig = MDF(&Lig_Data, Lig);
                } 
            }
            Tick_sys[4] = HAL_GetTick() - Tick_sys[4];

            if(!(Flag_Lig&&Flag_Tem&&Flag_Hum))     LED = 1;
            else if(Hum>70 || Lig>1000 || Tem>27)   LED = 2;
            else LED =0;            
        }
    }
    if(htim == &htim2){         //500ms循环
        if(LED==1)  HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);  //翻转LED灯的状态
        if(++Tick_1s == 2){     //1s循环
            Tick_1s = 0;
            cpu_usage = 100 - (idle_time * 100.0 / total_time);   // 计算 CPU 占用率
            idle_time = 0;
        }
    }
}
// 串口接收中断回调函数
//bit15，	接收完成标志
//bit14，	接收到0x0d 回车\r是把光标置于本行行首 换行\n是把光标置于下一行的同一列
//bit13~0，	接收到的有效字节数目
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{      
   if((USART_RX_STA & 0x8000)==0){             //接收未完成
       if(USART_RX_STA & 0x4000){              //接收到了\r
           if(re==0x0a) USART_RX_STA|=0x8000;  //又接收到了\n 接收完成了！
           else USART_RX_STA=0;                //接收错误,重新开始
       }else{	
           if(re==0x0d) USART_RX_STA|=0x4000;  //接收到了\r
           else{
               USART_RX_BUF[USART_RX_STA&0X3FFF]=re ;//保存数据
               USART_RX_STA++;
               if(USART_RX_STA>(USART_REC_LEN-1)) USART_RX_STA=0;//接收数据溢出,重新开始接收	  
               }
           }
   }
   HAL_UART_Receive_IT(&huart1, (uint8_t *)&re, 1);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t need_start = 0;
    uint16_t tx_len = 0;
    uint16_t tx_tail = 0;

    if (huart != &huart1) {
        return;
    }

    __disable_irq();
    uart1_tx_tail = (uart1_tx_tail + huart->TxXferSize) % UART1_TX_BUF_SIZE;
    if (uart1_tx_head != uart1_tx_tail) {
        tx_tail = uart1_tx_tail;
        if (uart1_tx_head > uart1_tx_tail) {
            tx_len = uart1_tx_head - uart1_tx_tail;
        } else {
            tx_len = UART1_TX_BUF_SIZE - uart1_tx_tail;
        }
        need_start = 1;
    } else {
        uart1_tx_busy = 0;
    }
    __enable_irq();

    if (need_start) {
        if (HAL_UART_Transmit_IT(&huart1, &uart1_tx_buf[tx_tail], tx_len) != HAL_OK) {
            __disable_irq();
            uart1_tx_busy = 0;
            __enable_irq();
        }
    }
}

static void UART1_Send(const uint8_t *data, uint16_t len)
{
    uint16_t used;
    uint16_t free_space;
    uint16_t first_chunk;
    uint8_t need_start = 0;
    uint16_t tx_len = 0;
    uint16_t tx_tail = 0;

    if ((data == NULL) || (len == 0)) {
        return;
    }

    __disable_irq();
    used = (uart1_tx_head >= uart1_tx_tail) ?
           (uart1_tx_head - uart1_tx_tail) :
           (UART1_TX_BUF_SIZE - (uart1_tx_tail - uart1_tx_head));
    free_space = (UART1_TX_BUF_SIZE - 1U) - used;
    if (len > free_space) {
        len = free_space;
    }

    if (len > 0U) {
        first_chunk = UART1_TX_BUF_SIZE - uart1_tx_head;
        if (first_chunk > len) {
            first_chunk = len;
        }
        memcpy(&uart1_tx_buf[uart1_tx_head], data, first_chunk);
        uart1_tx_head = (uart1_tx_head + first_chunk) % UART1_TX_BUF_SIZE;

        if (len > first_chunk) {
            memcpy(&uart1_tx_buf[uart1_tx_head], data + first_chunk, len - first_chunk);
            uart1_tx_head = (uart1_tx_head + (len - first_chunk)) % UART1_TX_BUF_SIZE;
        }
    }

    if ((!uart1_tx_busy) && (uart1_tx_head != uart1_tx_tail)) {
        uart1_tx_busy = 1;
        tx_tail = uart1_tx_tail;
        if (uart1_tx_head > uart1_tx_tail) {
            tx_len = uart1_tx_head - uart1_tx_tail;
        } else {
            tx_len = UART1_TX_BUF_SIZE - uart1_tx_tail;
        }
        need_start = 1;
    }
    __enable_irq();

    if (need_start) {
        if (HAL_UART_Transmit_IT(&huart1, &uart1_tx_buf[tx_tail], tx_len) != HAL_OK) {
            __disable_irq();
            uart1_tx_busy = 0;
            __enable_irq();
        }
    }
}

int fputc(int ch, FILE *f)
{
    uint8_t c = (uint8_t)ch;
    (void)f;
    UART1_Send(&c, 1);
    return ch;
}



/* USER CODE END 4 */

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

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
