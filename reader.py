import serial
from time import sleep

port = serial.Serial('/dev/ttyACM0', 115200, timeout=1)


def start():
    to_send = b's\r\n'
    port.write(to_send)
    print(port.readline())


def end():
    to_send = b'f\r\n'
    port.write(to_send)
    print(port.readline())


def read_memory():
    address = 0x8000000
    responses = []
    for chunk in range(125):
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


def read_sector(sector):
    addr = 0x8000000+0x80*sector
    to_send = b'r'
    to_send += addr.to_bytes(4, 'big')
    to_send += (127).to_bytes(1, 'big')
    to_send += b'\r\n'
    port.write(to_send)
    return port.readline()[1:]


def mem_to_file(responses):
    file = bytearray()
    for i in responses:
        file += i[1:]
    return file


def erase_memory():
    to_send = b'p\r\n'
    port.write(to_send)
    print(port.readline())


def good_erase(pages):
    to_send = b'b'
    to_send += str(len(pages)).encode()
    str_pages = [str(i) for i in pages]
    to_send += b' '
    to_send += ','.join(str_pages).encode()
    to_send += b'\r\n'
    port.write(to_send)
    return port.readline()


def readout_unprotect():
    to_send = b'l\r\n'
    port.write(to_send)
    return port.readline()


def readout_protect():
    to_send = b'p\r\n'
    port.write(to_send)
    return port.readline()


def write_unprotect():
    to_send = b'u\r\n'
    port.write(to_send)
    return port.readline()


if __name__ == '__main__':
    start()
    print(write_unprotect())
    end()
    print('Sleep 1')
    sleep(1)

    start()
    print(good_erase([1, 2, 3, 4, 5]))
    end()
    print('Sleep2')
    sleep(1)

    start()
    sector = read_sector(4)
    end()
