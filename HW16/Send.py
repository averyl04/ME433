import serial
import matplotlib.pyplot as plt

PORT = "COM5"
BAUD = 115200

ser = serial.Serial(PORT, BAUD, timeout=10)

ser.reset_input_buffer()

ser.write(b'a')

index = []
desired = []
actual = []

while len(index) < 400:

    line = ser.readline().decode().strip()

    if not line:
        continue

    try:
        i, d, a = map(int, line.split(','))

        index.append(i)
        desired.append(d)
        actual.append(a)

        print(i, d, a)

    except:
        pass

ser.close()

plt.figure(figsize=(10,5))

plt.plot(index,
         desired,
         label="Desired Current")

plt.plot(index,
         actual,
         label="Measured Current")

plt.xlabel("Sample")
plt.ylabel("Current")

plt.title("HW16 Current Control")

plt.grid(True)
plt.legend()

plt.show()