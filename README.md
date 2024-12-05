# STM32 Light Sensor with Bluetooth

Reads light levels from a BH1750 light sensor and transmit the data via Bluetooth using a HC-05 module and ST-Link debugging.

## Hardware Requirements
- STM32F446RE Nucleo Board
- BH1750 Light Sensor
- HC-05 Bluetooth Module
- Jumper wires

## Pin Connections
### BH1750 (I2C1)
- VCC → 3.3V
- GND → GND
- SCL → PB8
- SDA → PB9
- ADDR → GND

### HC-05 (USART1)
- VCC → 3.3V
- GND → GND
- TX → PB7 (USART1 RX)
- RX → PB6 (USART1 TX)

## Software Components
- STM32 program for sensor reading and Bluetooth transmission
- Simple Python script for receiving data

## Features
- Continuous light level readings
- Bluetooth transmission of sensor data
- Debug output via USART2

## Usage
1. Connect hardware according to pin configurations
2. Upload STM32 program to board
3. Pair HC-05 with computer (PIN: 1234)
4. Run Python script to receive data

![Terminal Outputs](/images/Terminal_Outputs.png)
![Connections](/images/Connections.jpeg)