#include "bsp_ds18b20.h"

#define delay_us(X)  delay((X)*80/5)


void delay(unsigned int n)
{
    while(n--);
}


void DS18B20_Init(void)
{
    GPIO_InitTypeDef	GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = OW_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH ;
    GPIO_InitStruct.Pull = GPIO_PULLUP ;
    HAL_GPIO_Init(OW_PIN_PORT, &GPIO_InitStruct);
}

void mode_input1(void )
{

    GPIO_InitTypeDef	GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = OW_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(OW_PIN_PORT, &GPIO_InitStruct);
}


void mode_output1(void )
{

    GPIO_InitTypeDef	GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = OW_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull = GPIO_NOPULL ;
    HAL_GPIO_Init(OW_PIN_PORT, &GPIO_InitStruct);
}


uint8_t ow_reset(void)
{
    uint8_t err;

    OW_DIR_OUT(); 
    OW_OUT_LOW(); 

    delay_us(400);	  

    OW_DIR_IN(); 

    delay_us(66);
    err = OW_GET_IN();		

    delay_us(480 - 66);
    if( OW_GET_IN() == 0 )		
        err = 1;

    return err;
}


uint8_t ow_bit_io( uint8_t b )
{
    OW_DIR_OUT(); 
    OW_OUT_LOW();
    delay_us(1); 

    if ( b ) OW_DIR_IN(); 

#define  OW_CONF_DELAYOFFSET  5
    delay_us(15 - 1 - OW_CONF_DELAYOFFSET);

    if( OW_GET_IN() == 0 ) b = 0;  

    delay_us(60 - 15);
    OW_DIR_IN();

    return b;
}


uint8_t ow_byte_wr( uint8_t b )
{
    uint8_t i = 8, j;
    do
    {
        j = ow_bit_io( b & 1 );
        b >>= 1;
        if( j ) b |= 0x80;
    }
    while( --i );
    return b;
}


uint8_t ow_byte_rd( void )
{
    return ow_byte_wr( 0xFF );
}

uint16_t ds18b20_read(void)
{
    uint8_t low_temp, high_temp;

    ow_reset();
    ow_byte_wr(OW_SKIP_ROM);
    ow_byte_wr(DS18B20_CONVERT);    
    delay_us(20);

    ow_reset();
    ow_byte_wr(OW_SKIP_ROM);
    ow_byte_wr(DS18B20_READ);   

    low_temp = ow_byte_rd();
    high_temp = ow_byte_rd();
    
    return ((high_temp<<8)|low_temp);
}
