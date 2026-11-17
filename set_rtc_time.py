import serial
from datetime import datetime
import time

# edit this to the port the arduino is connected to
ser = serial.Serial('COM5', 115200)

time.sleep(2)  # Wait for RP2040 to initialize

timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

ser.write((timestamp + "\n").encode())

ser.close()