#include "main.h"
#include <stdio.h>

#define LIGHT_ADDR 			0x23
#define LIGHT_ON			0x01
#define LIGHT_CONT			0x10

// Prototypes
void USART2_Init();
void USART1_Init();

int main() {
	USART2_Init();

	printf("Hello\r\n");
	while (1) {

	}
}

// USART2 - ST-Link Setup
void USART2_Init() {
	RCC -> APB1ENR |= (1 << 17);								// USART2
	RCC -> AHB1ENR |= (1 << 0);									// GPIOA

	GPIOA -> MODER &= ~((3 << (2 * 2)) | (3 << (2 * 3)));		// PA2 & 3 Set to Alternate Function
	GPIOA -> MODER |= (2 << (2 * 2)) | (2 << (2 * 3));

	GPIOA -> AFR[0] &= ~((15 << (4 * 2)) | (15 << (4 * 3)));	// Alternate Function to AF7
	GPIOA -> AFR[0] |= (7 << (4 * 2)) | (7 << (4 * 3));

	USART2 -> BRR = 0x0683;										// 9600 Baud Rate

	USART2 -> CR1 |= (1 << 3);									// Transmitter Enabled
	USART2 -> CR1 |= (1 << 13);									// USART2 Enabled
}

// USART1 - Bluetooth Module
void USART1_Init() {
	RCC -> APB2ENR |= (1 << 4);									// USART1
	RCC -> AHB1ENR |= (1 << 1);									// GPIOB

	GPIOB -> MODER &= ~((3 << (2 * 6)) | (3 << (2 * 7)));		// PB6 & 7 Set to Alternate Function
	GPIOB -> MODER |= (2 << (2 * 6)) | (2 << (2 * 7));

	GPIOB -> AFR[0] &= ~((15 << (4 * 6)) | (15 << (4 * 7)));	// Alternate Function to AF7
	GPIOB -> AFR[0] |= (2 << (4 * 6)) | (2 << (4 * 7));

	USART1 -> BRR = 0x0683;										// 9600 Baud Rate

	USART1 -> CR1 |= (1 << 2) | (1 << 3);						// Transmitter/Receiver Enabled
	USART1 -> CR1 |= (1 << 13);									// USART1 Enabled
}

// I2C for sensor/s; Maybe more than 1 but starting with BH1750 light sensor
void I2C2_Init() {
	RCC -> AHB1ENR |= (1 << 1);									// GPIOB
	RCC -> APB1ENR |= (1 << 21);								// I2C1

	GPIOB -> OTYPER &= ~((1 << 8) | (1 << 9));					// Open-Drain on PB8 & 9
	GPIOB -> OTYPER |= (1 << 8) | (1 << 9);

	GPIOB -> PUPDR &= ~((3 << (2 * 8)) | (3 << (2 * 9)));		// Pull-Up on PB8 & 9
	GPIOB -> PUPDR |= (1 << (2 * 8)) | (1 << (2 * 9));

	GPIOB -> MODER &= ~((3 << (2 * 8)) | (3 << (2 * 9)));		// PB8 & 9 Set to Alternate Function
	GPIOB -> MODER |= (2 << (2 * 8)) | (2 << (2 * 9));

	GPIOB -> AFR[1] &= ~((15 << (4 * 0)) | (15 << (4 * 1)));	// Alternate Function to AF4
	GPIOB -> AFR[1] |= (4 << (4 * 0)) | (4 << 4 * 1);

	I2C1 -> CR2 = 42;											// 42MHz Frequency
	I2C1 -> CCR = 210; 											// 100kHz Frequency
	I2C1 -> TRISE = 43;											// Max Rise Time
	I2C1 -> CR1 |= (1 << 0);									// I2C1 Enabled
}

uint8_t I2C_Check_Busy() {
	if (I2C1 -> SR1 & (1 << 1)) {
		return 1;
	}
	return 0;
}

void I2C_Start() {
	I2C1 -> CR1 |= (1 << 8);
	while (!(I2C1 -> SR1 & (1 << 0)));
}

void I2C_Send_Address(uint8_t addr, uint8_t read) {
	I2C1 -> DR |= (addr << 1) | read;
	while(!(I2C1 -> SR1 & (1 << 1)));

	uint8_t temp = I2C1 -> SR1;
	temp = I2C1 -> SR2;
	(void) temp;
}

void I2C_Send_Data(uint8_t data) {
	I2C1 -> DR = data;
	while(!(I2C1 -> SR1 & (1 << 2)));
}

void I2C_Stop() {
	I2C1 -> CR1 |= (1 << 9);
	while(!(I2C1 -> SR1 & (1 << 4)));
}

void I2C_Write(uint8_t addr, uint8_t data) {
	while(I2C_Check_Busy());
	I2C_Start();
	I2C_Send_Address(addr, 0);
	I2C_Send_Data(data);
	I2C_Stop();
}

void Light_Sensor_Init() {
	I2C_Write(LIGHT_ADDR, LIGHT_ON);
	I2C_Write(LIGHT_ADDR, LIGHT_CONT);
}

uint16_t Light_Read() {
	uint8_t upper, lower;

	while(I2C_Check_Busy());
	I2C_Start();
	I2C_Send_Address(LIGHT_ADDR, 1);

	I2C1 -> CR1 |= (1 << 10);
	while(!(I2C1 -> SR1 & (1 << 6)));
	upper = I2C1 -> DR;

	I2C1 -> CR1 &= ~(1 << 10);

	while(!(I2C1 -> SR1 & (1 << 6)));
	lower = I2C1 -> DR;

	I2C_Stop();

	uint16_t value = (upper << 8) | lower;
	return value;
}


// printf retarget with putchar
int __io_putchar(int c) {
    while(!(USART2 -> SR & (1 << 7)));
    USART2 -> DR = c;

    while(!(USART2 -> SR & (1 << 6)));
    USART2 -> SR &= ~(1 << 6);
    USART2 -> SR &= ~(1 << 5);

    return c;
}

// Maybe bluetooth can submit commands to get readings and to update
