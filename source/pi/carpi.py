from smbus import SMBus
import time
import sys


# servo 1
#50 max left
#70 center
#90 max right
# example: sudo python3 carpi.py 2 180


if (len(sys.argv) != 3):
	print("Error: Not enough arguments")
	exit()

device = int(sys.argv[1])
value = int(sys.argv[2])
print("device " + str(device))
print("value " + str(value))


bus = SMBus(1) # 512M Pi use i2c port 1, 256M Pi use i2c port 0
time.sleep(1)
address = 0x8

if (value < 256):
	bus.write_i2c_block_data(address, device, [value])
else:
	high, low = value >> 4, value & 0x0FF
	print(high)
	print(low)
	bus.write_i2c_block_data(address, device, [high, low])

bus.close()
