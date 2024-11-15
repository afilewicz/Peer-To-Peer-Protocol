import argparse
import socket
import struct


HOST = '127.0.0.1'
PORT = 8000


def generate_datagram(datagram_size: int) -> bytes:
    length_field = struct.pack("!H", datagram_size)
    data_field = bytes([(65 + i % 26) for i in range(datagram_size - 2)])
    print(data_field)
    return length_field + data_field


def parse_arguments() -> argparse.Namespace:
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

    print(f"Will send to {host}:{port}")

    sizes = [3, 50, 100, 200, 500, 1000, 2000, 3000, 5000]

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        for size in sizes:
            message = generate_datagram(size)
            s.sendto(message, (host, port))

            data, _ = s.recvfrom(size)
            received_message = int.from_bytes(data, byteorder='big')

            print(f"Received from {host}:{port}: {received_message}")

            if received_message == size:
                print("Received size matches sent size.")
            else:
                print(f"Sent size doesn't match received size")


if __name__ == '__main__':
    main()
