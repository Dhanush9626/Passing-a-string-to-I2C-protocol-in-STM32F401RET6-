#include "stm32f401xe.h"
#include "stdint.h"

#define I2C_WRITE_ADDRESS (0xA0UL)
#define I2C_READ_ADDRESS  (0xA1UL)

// PINOUT
// PB8 - SCL
// PB9 - SDA
//#define SM100KHz 80
// TPCLK => 16MHz => 1/16 => 0.0625 us => 62.5 ns

//USE CCR REGISTER => 503
// Standard mode
// Standard mode => 100KHz => 1/100 => 0.01 ms => 0.01 * 1000 * 1000 => 10000 ns
// THIGH && TLOW => 5000 ns
// THIGH => CCR * TPCLK
// CCR => THIGH / TPCLK => 5000 / 62.5 => 80
// TLOW => CCR * TPCLK
// CCR => TLOW / TPCLK => 5000 / 62.5 => 80

#define SM100KHz 80

void I2C_CONFIGURATION();
void START();
void STOP();
void RESTART();
void WRITE_TO_EEPROM(uint8_t MSB, uint8_t LSB,unsigned char *BYTE);
void DELAY(uint32_t ms);

void I2C_CONFIGURATION()
{

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    GPIOB->MODER |= (0x1UL << 19U) | (0x1UL << 17U);
    GPIOB->MODER &= ~((0x1UL << 18U) | (0x1UL << 16U));
    GPIOB->OTYPER |= (0x1UL << 8) | (0x1UL << 9);

    GPIOB->AFR[1] |= (0x4UL<<0U) | (0x4UL<<4U);

    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    I2C1->CR2 = (0x1UL << 4U); //frequency
    I2C1->CCR = SM100KHz;
    //SM Mode (Standard Mode) =>  Speed: Up to 100 kHz.
    //FM Mode (Fast Mode) => Speed: Up to 400 kHz.
    I2C1->CR1 |= I2C_CR1_PE;
}

void START()
{
    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));
}

void STOP()
{
    I2C1->CR1 |= I2C_CR1_STOP;
    while (I2C1->SR1 & I2C_SR1_STOPF);
}

void RESTART()
{
    STOP();
    START();
}

void WRITE_TO_EEPROM(uint8_t MSB, uint8_t LSB, unsigned char *BYTE)
{
    START();

    I2C1->DR = I2C_WRITE_ADDRESS;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));

    I2C1->DR = MSB;
    while (!(I2C1->SR1 & I2C_SR1_BTF));


    I2C1->DR = LSB;
    while (!(I2C1->SR1 & I2C_SR1_BTF));
    while(*BYTE)
    {
      I2C1->DR = *BYTE++;
      while (!(I2C1->SR1 & I2C_SR1_BTF));
    }

    STOP();
}

int main()
{
    I2C_CONFIGURATION();
    WRITE_TO_EEPROM(0X00,0X38,"Dhanush");
    DELAY(100);
    while(1);
}

void DELAY(uint32_t ms)
{
    uint32_t i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 16000; j++);
}
