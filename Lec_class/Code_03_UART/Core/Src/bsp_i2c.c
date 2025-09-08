#include "bsp_i2c.h"

#define DELAY_TIME	5

void SDA_Input_Mode()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.Pin = GPIO_PIN_7;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void SDA_Output_Mode()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.Pin = GPIO_PIN_7;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void SDA_Output( uint16_t val )
{
    if(val)
        GPIOB->BSRR |= GPIO_PIN_7;
    else
        GPIOB->BRR |= GPIO_PIN_7;
}

void SCL_Output( uint16_t val )
{
    if(val)
        GPIOB->BSRR |= GPIO_PIN_6;
    else
        GPIOB->BRR |= GPIO_PIN_6;
}

uint8_t SDA_Input(void)
{
    return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7);
}

static void delay1(unsigned int n)
{
    uint32_t i;
    for ( i = 0; i < n; ++i);
}

void I2CStart(void)
{
    SDA_Output(1);
    delay1(DELAY_TIME);
    SCL_Output(1);
    delay1(DELAY_TIME);
    SDA_Output(0);
    delay1(DELAY_TIME);
    SCL_Output(0);
    delay1(DELAY_TIME);
}

void I2CStop(void)
{
    SCL_Output(0);
    delay1(DELAY_TIME);
    SDA_Output(0);
    delay1(DELAY_TIME);
    SCL_Output(1);
    delay1(DELAY_TIME);
    SDA_Output(1);
    delay1(DELAY_TIME);
}

unsigned char I2CWaitAck(void)
{
    unsigned short cErrTime = 5;
    SDA_Input_Mode();
    delay1(DELAY_TIME);
    SCL_Output(1);
    delay1(DELAY_TIME);
    while(SDA_Input())
    {
        cErrTime--;
        delay1(DELAY_TIME);
        if (0 == cErrTime)
        {
            SDA_Output_Mode();
            I2CStop();
            return ERROR;
        }
    }
    SCL_Output(0);
    delay1(DELAY_TIME);
    SDA_Output_Mode();
    return SUCCESS;
}

void I2CSendAck(void)
{
    SDA_Output(0);
    delay1(DELAY_TIME);
    delay1(DELAY_TIME);
    SCL_Output(1);
    delay1(DELAY_TIME);
    SCL_Output(0);
    delay1(DELAY_TIME);
}

void I2CSendNotAck(void)
{
    SDA_Output(1);
    delay1(DELAY_TIME);
    delay1(DELAY_TIME);
    SCL_Output(1);
    delay1(DELAY_TIME);
    SCL_Output(0);
    delay1(DELAY_TIME);

}

void I2CSendByte(unsigned char cSendByte)
{
    unsigned char  i = 8;
    while (i--)
    {
        SCL_Output(0);
        delay1(DELAY_TIME);
        SDA_Output(cSendByte & 0x80);
        delay1(DELAY_TIME);
        cSendByte += cSendByte;
        delay1(DELAY_TIME);
        SCL_Output(1);
        delay1(DELAY_TIME);
    }
    SCL_Output(0);
    delay1(DELAY_TIME);
}

unsigned char I2CReceiveByte(void)
{
    unsigned char i = 8;
    unsigned char cR_Byte = 0;
    SDA_Input_Mode();
    while (i--)
    {
        cR_Byte += cR_Byte;
        SCL_Output(0);
        delay1(DELAY_TIME);
        delay1(DELAY_TIME);
        SCL_Output(1);
        delay1(DELAY_TIME);
        cR_Byte |=  SDA_Input();
    }
    SCL_Output(0);
    delay1(DELAY_TIME);
    SDA_Output_Mode();
    return cR_Byte;
}


void EEPROM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.Pin = GPIO_PIN_7 | GPIO_PIN_6;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}

//24C02的相关代码
void Write_24c(uint8_t *pucBuf, uint16_t ucAddr, uint8_t ucNum)
{
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(ucAddr);	
	I2CWaitAck();
    
	while(ucNum--)
	{
		I2CSendByte(*pucBuf++);
		I2CWaitAck();	
	}
	I2CStop();
	delay1(50);	
}
void Read_24c(uint8_t *pucBuf, uint16_t ucAddr, uint8_t ucNum)
{
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(ucAddr);	
	I2CWaitAck();
    
	I2CStart();
	I2CSendByte(0xa1);
	I2CWaitAck();
	while(ucNum--)
	{
		*pucBuf++ = I2CReceiveByte();
		if(ucNum)
			I2CSendAck();	
		else
			I2CSendNotAck();
	}
	I2CStop();	
}




uint8_t mcy=0;
uint8_t BUF[3];
/***开始信号**/
void BH1750_Start()
{
    HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_SET);                    //拉高数据线
    HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_SET);                   //拉高时钟线
    delay1(DELAY_TIME);
    HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_RESET);                    //产生下降沿
    delay1(DELAY_TIME);
    HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_RESET);                    //拉低时钟线
}

/*****停止信号******/
void BH1750_Stop()
{
    HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_RESET);                   //拉低数据线
    HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_SET);                      //拉高时钟线
    delay_nus(5);
    HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_SET);                    //产生上升沿
    delay_nus(5);
}
/*****初始化BH1750******/
void BH1750_Init()
{
    BH1750_Start();                                                 //起始信号
    BH1750_SendByte(SlaveAddress);                                  //发送设备地址+写信号
    BH1750_SendByte(0x01);                                  //内部寄存器地址
    BH1750_Stop();                                                  //停止信号
}

//连续读出BH1750内部数据
void mread(void)
{
    uint8_t i;
    BH1750_Start();                          //起始信号
    BH1750_SendByte(SlaveAddress+1);         //发送设备地址・+读信号

    for (i=0; i<3; i++)                      //连续读取6个地址数据到BUF
    {
        BUF[i] = BH1750_RecvByte();
        if (i == 3)
        {
            BH1750_SendACK(1);                //最后一个数据需要回NOACK
        }
        else
        {
            BH1750_SendACK(0);                //回应ACK
        }
    }
    BH1750_Stop();                          //停止信号
    Delay_mms(1);

}

uint32_t GY30_Read_Data(void)
{
    uint16_t dis_data;
    uint16_t Value_GY_30;
    Single_Write_BH1750(0x01);   // power on
    Single_Write_BH1750(0x10);   // H- resolution mode
    HAL_Delay(10);            //延时180ms
    mread();       //连续读出数据，存储在BUF中
    dis_data=BUF[0];
    dis_data=(dis_data<<8)+BUF[1];//字节合成数据
    Value_GY_30=(float)dis_data/1.2;
    return Value_GY_30;
}
//系统主频72MHZ
void delay_nus(uint16_t us)
{
    while(us--)
    {
        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
//        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
//        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
//        __NOP();__NOP();
    }
}
void Delay_mms(uint16_t tmp)
{
    uint16_t i=0;
    while(tmp--)
    {
        i=12000;
        while(i--);
    }
}

/**************************************
发送应答信号
入口参数:ack (0:ACK 1:NAK)
**************************************/
void BH1750_SendACK(int ack)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = scl|sda;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    if(ack == 1)   //写应答信号
        HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_SET);
    else if(ack == 0)
        HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_RESET);
    else
        return;

    HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_SET);
    delay_nus(5);
    HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_RESET);
    delay_nus(5);
}

/**************************************
接收应答信号
**************************************/
int BH1750_RecvACK()
{

    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  /*输入上拉*/
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin = sda;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_SET);            //拉高时钟线
    delay_nus(5);

    if(HAL_GPIO_ReadPin( GPIOB, sda ) == 1 )//读应答信号
        mcy = 1 ;
    else
        mcy = 0 ;

    HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_RESET);                    //拉低时钟线
    delay_nus(5);

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );

    return mcy;
}

/**************************************
向iic总线发送一个字节数据
**************************************/
void BH1750_SendByte(uint8_t dat)
{
    uint8_t i;

    for (i=0; i<8; i++)         //8位计数器
    {
        if( 0X80 & dat )
            HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_RESET);

        dat <<= 1;
        HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_SET);               //拉高时钟线
        delay_nus(5);
        HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_RESET);                //拉低时钟线
        delay_nus(5);
    }
    BH1750_RecvACK();
}

//我们对BH1750发送命令时，要先发送器件地址+写入位，然后发送指令
//读取数据的时候，需要先发送器件地址+读入位，然后读取两字节数据

//写入指令
void Single_Write_BH1750(uint8_t REG_Address)//REG_Address是要写入的指令
{
    BH1750_Start();                  //起始信号
    BH1750_SendByte(SlaveAddress);  //发送器件地址+写信号
    BH1750_SendByte(REG_Address);   //写入指令，内部寄存器地址
    BH1750_Stop();                   //结束信号
}
/**************************************
从iic总线读取一个字节地址
**************************************/
uint8_t BH1750_RecvByte()
{
    uint8_t i;
    uint8_t dat = 0;
    uint8_t bit;

    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;   /*上拉输入*/
    GPIO_InitStruct.Pin = sda;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );

    HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_SET);          //准备读取数据
    for (i=0; i<8; i++)         //8位计数器
    {
        dat <<= 1;
        HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_SET);               //拉高时钟线
        delay_nus(5);

        if( SET == HAL_GPIO_ReadPin( GPIOB, sda ) )
            bit = 0X01;
        else
            bit = 0x00;

        dat |= bit;             //读数据

        HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_RESET);                //拉低时钟线
        delay_nus(5);
    }

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );
    return dat;
}
