#include "main.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Macros
#define LIGHT_ADDR 			0x23
#define LIGHT_ON			0x01
#define LIGHT_CONT			0x10

#define RX_BUFFER_SIZE 		64
#define CMD_LENGTH 			30

// Prototypes
void USART2_Init();
void USART1_Init();
void I2C1_Init();
void Light_Sensor_Init();
uint16_t Light_Read();
void Bluetooth_Print(const char*);

// Circular Buffer setup; Ensures a max of 64 characters only
volatile char rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_head = 0;
volatile uint8_t rx_tail = 0;
// True when full command is read
volatile bool cmd_ready = false;

int main() {
    USART2_Init();
    USART1_Init();
    I2C1_Init();
    Light_Sensor_Init();

    printf("System Ready\r\n");

    while(1) {
        if(cmd_ready) {
            printf("Command received!\r\n");

            char cmd[CMD_LENGTH];
            uint8_t i = 0;

            while(rx_tail != rx_head && i < CMD_LENGTH-1) {
                cmd[i++] = rx_buffer[rx_tail];
                rx_tail = (rx_tail + 1) % RX_BUFFER_SIZE;
            }
            cmd[i] = '\0';
            cmd_ready = false;

            printf("Command is: %s\r\n", cmd);

            // Process commands
            if(strcmp(cmd, "light read\r\n") == 0) {
                char str[30];
                uint16_t value = Light_Read();
                sprintf(str, "Light Value: %d\r\n", value);
                Bluetooth_Print(str);
            }
            else if(strcmp(cmd, "status\r\n") == 0) {
                Bluetooth_Print("System Status: OK\r\n");
            }
            else if(strcmp(cmd, "help\r\n") == 0) {
                Bluetooth_Print("1. light read\r\n2. status\r\n3. help\r\n");
            }
            else {
                Bluetooth_Print("Unknown Command\r\n");
            }
        }
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
	GPIOB -> AFR[0] |= (7 << (4 * 6)) | (7 << (4 * 7));

	USART1 -> BRR = 0x0683;										// 9600 Baud Rate

	USART1 -> CR1 |= (1 << 2) | (1 << 3) | (1 << 5);			// Transmitter/Receiver/Read Interrupt Enabled

	NVIC -> ISER[1] |= (1 << 5);								// Enables USART1 Global Interrupt

	USART1 -> CR1 |= (1 << 13);									// USART1 Enabled
}

// Interrupts whenever a character is sent from Python; Stores characters in a cicular buffer and continues until full command is received
void USART1_IRQHandler(void) {
    if(USART1->SR & (1 << 5)) {
        char c = USART1->DR;
        uint8_t next_head = (rx_head + 1) % RX_BUFFER_SIZE;

        if(next_head != rx_tail) {
            rx_buffer[rx_head] = c;
            rx_head = next_head;

            if(c == '\n') {
                cmd_ready = true;
            }
        }
    }
}

// I2C for sensor/s; Maybe more than 1 but starting with BH1750 light sensor
void I2C1_Init() {
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

// Checks if I2C bus is busy
uint8_t I2C_Check_Busy() {
	if (I2C1 -> SR2 & (1 << 1)) {
		return 1;
	}
	return 0;
}

// Generates start condition
void I2C_Start() {
	I2C1 -> CR1 |= (1 << 8);
	while (!(I2C1 -> SR1 & (1 << 0)));
}

// Sends device address and clears associated flags
void I2C_Send_Address(uint8_t addr, uint8_t read) {
	I2C1 -> DR = (addr << 1) | read;
	while(!(I2C1 -> SR1 & (1 << 1)));

	uint8_t temp = I2C1 -> SR1;
	temp = I2C1 -> SR2;
	(void) temp;
}

// Sends a byte of data to device from master
void I2C_Send_Data(uint8_t data) {
	I2C1 -> DR = data;
	while(!(I2C1 -> SR1 & (1 << 2)));
}

// Generates stop condition
void I2C_Stop() {
	I2C1 -> CR1 |= (1 << 9);
}

// Entire write to a device
void I2C_Write(uint8_t addr, uint8_t data) {
	while(I2C_Check_Busy());
	I2C_Start();
	I2C_Send_Address(addr, 0);
	I2C_Send_Data(data);
	I2C_Stop();
}

// Initializes BH1750 light sensor
void Light_Sensor_Init() {
	I2C_Write(LIGHT_ADDR, LIGHT_ON);
	I2C_Write(LIGHT_ADDR, LIGHT_CONT);
}

// Returns the value from the light sensor
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

// Prints a string to the USART1 (Python) terminal
void Bluetooth_Print(const char* str) {
    while(*str) {
        while(!(USART1->SR & (1 << 7)));
        USART1->DR = *str;
        str++;
    }
    while(!(USART1->SR & (1 << 6)));
}

