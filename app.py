import serial
import time

ser = serial.Serial('COM4', 9600, timeout=1)
print("Port opened successfully")

while True:
    cmd = input("Enter command: ")
    if cmd.lower() == 'quit':
        break

    full_cmd = cmd + '\r\n'
    ser.write(full_cmd.encode())
    ser.flush()

    time.sleep(0.2)
    response = ""

    while True:
        if ser.in_waiting > 0:
            response += ser.read(ser.in_waiting).decode()
            if '\n' in response:
                break
        time.sleep(0.1)

    if response:
        print(response)
    else:
        print("No response received")

    ser.reset_input_buffer()

ser.close()