import argparse
import socket
import struct


HOST = '127.0.0.1'
PORT = 8000
BUFSIZE = 66000


def generate_message(declared_length):
    message = b''
    for i in range(declared_length - 2):
        message += bytes([65 + (i % 26)])
    return message


def parse_arguments():
    parser = argparse.ArgumentParser(description='UDP client')

    parser.add_argument('-s', '--host', help=f"The server's hostname or IP address, default: {HOST}", default=HOST)
    parser.add_argument('-p', '--port', help=f"Port that will be used, default: {PORT}", default=PORT)

    return parser.parse_args()


def main():
    args = parse_arguments()

    if args.host:
        host = args.host
    if args.port:
        port = int(args.port)

    print("Will listen on ", host, ":", port, flush=True)

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind((host, port))
        i=1
        while True:
            data_address = s.recvfrom( BUFSIZE )
            data = data_address[0]
            address = data_address[1]

            data_length = int.from_bytes(data[:2], byteorder='big')
            response = struct.pack('>H', data_length)
            print("Expected message length: ", data_length, flush=True)

            if data_length != len(data):
                print("Length of datagram is incorrect", flush=True)
                response = struct.pack('>H', 0)

            print( "Client IP Address:{}".format(address) , flush=True)

            message = generate_message(data_length)

            if message != data[2:]:
                print("Message is incorrect", flush=True)
                response = struct.pack('>H', 0)

            if not data:
                print("Error in datagram?", flush=True)
                response = struct.pack('>H', 0)

            s.sendto(response, address)
            print('sending dgram #', i, flush=True)
            i+=1


if __name__ == "__main__":
    main()
