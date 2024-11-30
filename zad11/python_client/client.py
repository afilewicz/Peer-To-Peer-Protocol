import argparse
import socket
import struct


def generate_datagram(datagram_size: int) -> bytes:
    # Set the first two bytes to the message length
    length_field = struct.pack("!H", datagram_size)
    # Fill the rest of the buffer with A … Z characters
    data_field = bytes([(65 + i % 26) for i in range(datagram_size - 2)])
    return length_field + data_field


def parse_arguments(default_host: str, default_port: int) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='UDP client')

    parser.add_argument('-s', '--host', help=f"The server's hostname or IP address, default: {default_host}", default=default_host)
    parser.add_argument('-p', '--port', help=f"Port that will be used, default: {default_port}", default=default_port)

    return parser.parse_args()


def main():
    default_host = '127.0.0.1'
    default_port = 8000

    # Set the server port and IP address
    args = parse_arguments(default_host, default_port)

    # Change host and port if given by user, else default values: host = 127.0.0.1, port = 8000
    if args.host:
        host = args.host
    if args.port:
        port = int(args.port)

    print(f"Will send to {host}:{port}", flush=True)

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        for size in range(3, 66000):
            message = generate_datagram(size)

            try:
                # Send the message to the server
                s.sendto(message, (host, port))

            except Exception as e:
                print(f"For length {size}: Error while sending datagram message", flush=True)
                print(e, flush=True)
                break

            try:
                # Receive the server's response
                data, _ = s.recvfrom(1024)
                received_message = data.decode('utf-8').strip()

                # Print the server's response
                print(f"For length {size} received from {host}:{port}: {received_message}", flush=True)

            except Exception as e:
                print(f"For length {size}: Error while receiving server response", flush=True)
                print(e, flush=True)
                break


if __name__ == '__main__':
    main()
