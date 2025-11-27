#include "bsp_dht11.h"

#define Delay_us(X)  delay1((X)*72/5)

void delay1(unsigned int n)
{
    while(n--);
}

//初始化DHT11的IO口，同时检测DHT11的存在

uint8_t DHT11_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    DHT11_PIN_CLOCK;
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull = GPIO_PULLUP ;
    HAL_GPIO_Init(DHT11_PIN_PORT, &GPIO_InitStruct);

    DHT11_Rst();
    return DHT11_Check();
}
void DHT11_PIN_INPUT(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**/
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_PIN_PORT, &GPIO_InitStruct);
}
void DHT11_PIN_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /**/
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_PIN_PORT, &GPIO_InitStruct);
}

//复位DHT11
void DHT11_Rst(void)
{
    DHT11_PIN_OUT(); 	//设置为输出
    DHT11_PIN_OUT_L; 	//低电平
    HAL_Delay(20);	//拉低至少18ms
    DHT11_PIN_OUT_H; 	//高电平
    Delay_us(60);     	//主机拉高20~40us
}

//等待DHT11的回应
//返回1:未检测到DHT11的存在
//返回0:存在
uint8_t DHT11_Check(void)
{
    uint8_t re = 0;
    DHT11_PIN_INPUT();      //设置为输出
    while (DHT11_PIN_IN && re < 100) //DHT11会拉低40~80us
    {
        re++;
        Delay_us(1);
    };
    if(re >= 100)return 1;
    else re = 0;
    while (!DHT11_PIN_IN && re < 100) //DHT11拉低后会再次拉高40~80us
    {
        re++;
        Delay_us(1);
    };
    if(re >= 100)return 1;
    return 0;
}

//从DHT11读取一个位
uint8_t DHT11_Read_Bit(void)
{
    uint8_t re = 0;
    while(DHT11_PIN_IN && re < 110) //等待变为低电平
    {
        re++;
        Delay_us(1);
    }
    re = 0;
    while(!DHT11_PIN_IN && re < 110) //等待变高电平
    {
        re++;
        Delay_us(1);
    }
    Delay_us(40);//等待40us
    if(DHT11_PIN_IN)return 1;
    else return 0;
}

//从DHT11读取一个字节
uint8_t DHT11_Read_Byte(void)
{
    uint8_t i, dat;
    dat = 0;
    for (i = 0; i < 8; i++)
    {
        dat <<= 1;
        dat |= DHT11_Read_Bit();
    }
    return dat;
}

//从DHT11读取一次数据
uint8_t DHT11_Read_Data(uint8_t *humi)
{
    uint8_t buf[5];
    uint8_t i;
    DHT11_Rst();
    if(DHT11_Check() == 0)
    {
        for(i = 0; i < 5; i++)
        {
            buf[i] = DHT11_Read_Byte();
        }
        if((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
        {
            *humi = buf[0];
        }
    }
    else return 1;
    return 0;
}



