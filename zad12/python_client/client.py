import argparse
import socket
import struct


HOST = '127.0.0.1'
PORT = 8000
DATAGRAM_SIZE = 512


def generate_datagram(datagram_size: int, alternating_bit_protocol: int) -> bytes:
    length_field = struct.pack("!H", alternating_bit_protocol)
    data_field = bytes([(65 + i % 26) for i in range(datagram_size - 2)])
    return length_field + data_field


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='UDP client')

    parser.add_argument('-s', '--host', help=f"The server's hostname or IP address, default: {HOST}", default=HOST)
    parser.add_argument('-p', '--port', help=f"Port that will be used, default: {PORT}", default=PORT)

    return parser.parse_args()


def main():
    # Set the server port and IP address
    args = parse_arguments()

    # Change host and port if given by user, else default values: host = 127.0.0.1, port = 8000
    if args.host:
        host = args.host
    if args.port:
        port = int(args.port)

    print(f"Will send to {host}:{port}", flush=True)

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        alternating_bit_protocol = 0
        datagram_id = 1

        message = generate_datagram(DATAGRAM_SIZE, alternating_bit_protocol)

        try:
            # Send the message to the server
            s.sendto(message, (host, port))

        except Exception as e:
            print(f"For datagram {datagram_id}: Error while sending datagram message", flush=True)
            print(e, flush=True)

        try:
            # Receive the server's response
            data, _ = s.recvfrom(1024)
            received_message = data.decode('utf-8').strip()

            # Print the server's response
            print(f"For datagram {datagram_id} received from {host}:{port}: {received_message}", flush=True)

        except Exception as e:
            print(f"For datagram {datagram_id}: Error while receiving server response", flush=True)
            print(e, flush=True)


if __name__ == '__main__':
    main()
