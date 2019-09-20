import serial
from time import sleep

port = serial.Serial('/dev/ttyACM0', 115200, timeout=1)


def start():
	to_send = b's\r\n'
	port.write(to_send)
	print(port.readline())


def read_memory():
	address = 0x8000000
	responses = []
	for chunk in range(70):
		to_send = bytearray()
		to_send += b'r'
		to_send += address.to_bytes(4, 'big')
		address += 128
		to_send += (127).to_bytes(1, 'big')
		to_send += b'\r\n'
		port.write(to_send)
		responses += [port.read(129)]
		sleep(0.05)
	return responses


def erase_memory():
	to_send = b'p\r\n'
	port.write(to_send)
	print(port.readline())


start()
