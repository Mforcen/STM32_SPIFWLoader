import serial

port = serial.Serial('/dev/ttyACM0', 115200, timeout=1)

f = open('testfile.bin', 'rb')
file_content = f.read()

chunks = len(file_content)//128
file_written = 0
address = 0x08000000
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
    print(response)

