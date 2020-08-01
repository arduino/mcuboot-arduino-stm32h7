from cobs import cobsr
import serial
import argparse
import struct
from crc16 import crc16xmodem as crc16

CHUNK_SIZE = 128


# Header format:
# Little Endian
# CRC16 (2-bytes)
# Sequence Number (2-bytes)
# Payload Size (2-bytes)
HEADER_FORMAT = "<HH".format(CHUNK_SIZE)


def read_in_chunks(file_object, chunk_size=256):
    while True:
        data = file_object.read(chunk_size)
        if not data:
            break
        yield data

def main():

    parser = argparse.ArgumentParser(description='Upload an update over SerialOTA')
    parser.add_argument('binary', help='path to binary to upload over SerialOTA')
    parser.add_argument('port', help='Serial port to use for SerialOTA')

    args = parser.parse_args()

    # Create the OTA serial
    ser = serial.Serial(args.port, 115200, timeout=0.1)

    # Open the binary
    with open(args.binary, 'rb') as f:

        # Start by sending 0 bytes until we get a response
        while True:
            ser.write(bytes([0]))
            rx = ser.read()
            print("received: {}".format(rx))
            if rx == bytes([1]):
                print("Received bootloader ACK -- waiting for ready")
                break

        while True:
            # Wait until the bootloader sends another response
            rx = ser.read()
            print("received: {}".format(rx))
            if rx == bytes([2]):
                print("Bootloader ready -- begin upload")
                break

        ser.flushInput()

        sequence_num = 0
        for chunk in read_in_chunks(f, CHUNK_SIZE):

            if len(chunk) != CHUNK_SIZE:
                sequence_num = 0xFFFF

            # Pack the header
            tx_buf = struct.pack(HEADER_FORMAT, sequence_num, len(chunk)) + chunk

            # Calculate the CRC16 of the chunk and the header info
            crc = crc16(bytes(tx_buf))
            print("Sending chunk: crc({}), seq num({}), size({})".format(crc, sequence_num, len(chunk)))

            tx_buf = struct.pack("<H", crc) + tx_buf

            # Encode with COBS
            tx_cobs = cobsr.encode(tx_buf) + bytes([0])
            print(type(tx_cobs))
            ser.write(tx_cobs)
            print(tx_cobs.hex())

            while(True):
                # Wait until the bootloader is ready
                c = ser.read()
                print("rx:{}".format(c))
                if c == bytes([0]):
                    break

            sequence_num = sequence_num + 1

if __name__ == '__main__':
    main()