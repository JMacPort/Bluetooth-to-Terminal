# STM32 Light Sensor with Bluetooth

Reads light levels from a BH1750 light sensor and transmit the data via Bluetooth using an HC-05 module. Commands can be sent through a Python terminal to request sensor readings and system status.

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
- ADDR → GND (Sets address to 0x23)

### HC-05 (USART1)
- VCC → 3.3V
- GND → GND
- TX → PB7 (USART1 RX)
- RX → PB6 (USART1 TX)

## Software Components
- I2C sensor communication
- Interrupt-driven UART
- Command processing using circular buffer
- Bluetooth communication

### Implementation Details:
- Circular Buffer for Command Processing
- Interrupt-safe design
- Non-blocking operation 
- Efficient memory usage
- Overflow protection
- Ring buffer implementation for UART reception

### Python Interface
- Command sending
- Response handling
- Serial port management

## Features
- Command-based light sensor readings
- System status queries
- Help menu
- Debug output via UART2
- Interrupt-driven communication

## Available Commands
1. "light read" - Get current light sensor reading
2. "status" - Check system status
3. "help" - Display available commands

## Usage
1. Connect hardware according to pin configurations
2. Upload STM32 program to board
3. Pair HC-05 with computer (default PIN: 1234)
4. Run Python script to send commands and receive data

![Terminal Outputs](/images/Terminal_Outputs.png)
![Connections](/images/Connections.jpeg)
![New Outputs](/images/New_Terminal_Output.png)