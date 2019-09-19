#!/usr/bin/python3

import serial
import argparse
from tqdm import trange
import sys
import glob
from time import sleep

def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

parser = argparse.ArgumentParser(description="Send a binary file ")

parser.add_argument('file', help='binary file to send')
parser.add_argument('-v', '--verbose',
                    help='print debug output', action="store_true")
parser.add_argument('-p', '--port', help='serial port')


args = parser.parse_args()

filename = args.file

if args.verbose:
    print("DEBUG")

try:
    if args.port:
        port = serial.Serial(args.port, 115200, timeout=1)
    else:
        port = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
except Exception:
    print("No serial port found use:")
    print("[" + ", ".join(serial_ports()) + "]")
    exit()

try:
    f = open(filename, 'rb')
except Exception:
    print('No file found.')
    exit()

file_content = f.read()
chunks = len(file_content)//128
file_written = 0
address = 0x08000000

bar_params = dict(desc="Progress",
                  ascii=False,
                  unit="Chunks",
                  bar_format="{desc}: {percentage:3.0f}% [{bar}] \
{n_fmt}/{total_fmt} {unit} [{rate_fmt}]",
                  ncols=70,
                  unit_scale=True)

responses = []

try:
    to_send = bytearray()
    to_send += b's\r\n'
    port.write(to_send)
    response = port.readline()
    print(response)
    responses += [response]
    sleep(0.05)
    # for i in trange(chunks, **bar_params):
    for i in range(chunks):
        chunk = file_content[file_written:file_written+128]
        file_written += 128
        checksum = chunk[0]
        for val in chunk[1:]:
            checksum ^= val
        to_send = bytearray()
        to_send += b'e'

        to_send += address.to_bytes(4, 'big')
        address += 128

        to_send += (127).to_bytes(1, 'big')

        to_send += chunk
        to_send += (checksum).to_bytes(1, 'big')
        to_send += '\r\n'.encode()

        port.write(to_send)

        response = port.readline()
        print(response)
        responses += [response]

    if file_written < 0x0fffffff:
        chunk = file_content[file_written:]
        checksum = chunk[0]
        for val in chunk[1:]:
            checksum ^= val
        to_send = bytearray()
        to_send += b'e'

        to_send += address.to_bytes(4, 'big')

        to_send += (len(chunk)-1).to_bytes(1, 'big')
        to_send += chunk
        to_send += checksum.to_bytes(1, 'big')
        to_send += '\r\n'.encode()

        port.write(to_send)

        response = port.readline()
        responses += [response]

        to_send = bytearray()
        to_send += b'f\r\n'
        port.write(to_send)
        response = port.readline()
        print(responses)
        print('f: ' + response)
except serial.serialutil.SerialException as serialError:
    print(f"[Serial Error] {serialError}")
    exit()
