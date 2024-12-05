import serial
import time

ser = serial.Serial('COM4', 9600, timeout=1)
print("Port opened successfully")

while True:
    if ser.in_waiting > 0:
        data = ser.read(ser.in_waiting)
        print(data.decode(), end='', flush=True)
    time.sleep(0.1)
