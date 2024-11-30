import argparse
import socket
import struct

from datetime import datetime


def generate_datagram(datagram_size: int, alternating_bit_protocol: int) -> bytes:
    # Set first byte to ABP value
    length_field = struct.pack("!B", alternating_bit_protocol)

    # Fill the rest of the buffer with A … Z characters
    data_field = bytes([(65 + i % 26) for i in range(datagram_size - 1)])
    return length_field + data_field


def parse_arguments(default_host: str, default_port: int) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='UDP client with Alternating Bit Protocol')

    parser.add_argument('-s', '--host', help=f"The server's hostname or IP address, default: {default_host}", default=default_host)
    parser.add_argument('-p', '--port', help=f"Port that will be used, default: {default_port}", default=default_port)

    return parser.parse_args()


def main():
    default_host = '127.0.0.1'
    default_port = 8000
    datagram_size = 512

    # Set the server port and IP address
    args = parse_arguments(default_host, default_port)

    # Change host and port if given by user, else default values: host = 127.0.0.1, port = 8000
    if args.host:
        host = args.host
    if args.port:
        port = int(args.port)

    print(f"Now: {datetime.now().strftime("%H:%M:%S")} - Will send to {host}:{port}", flush=True)

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        # Set socket timeout
        s.settimeout(2)

        alternating_bit_protocol = 0

        # Send and receive response for messages
        datagram_id = 1
        while True:
            while True:
                # Generate message to send
                message = generate_datagram(datagram_size, alternating_bit_protocol)
                print(flush=True)

                try:
                    # Send the message to the server
                    s.sendto(message, (host, port))
                    print(f"Now: {datetime.now().strftime("%H:%M:%S")} - Sending datagram {datagram_id} with ABP {alternating_bit_protocol} to {host}:{port}", flush=True)

                    # Receive the server's response
                    ack, _ = s.recvfrom(1024)
                    received_message = ack.decode('utf-8').strip()

                    # Acknowledge the server's response
                    if received_message == f"ACK {alternating_bit_protocol}":
                        print(f"Now: {datetime.now().strftime("%H:%M:%S")} - For datagram {datagram_id} received: OK", flush=True)

                        # Change alternating bit protocol (0 -> 1 or 1 -> 0)
                        alternating_bit_protocol = 1 - alternating_bit_protocol
                        break
                    else:
                        print(f"Now: {datetime.now().strftime("%H:%M:%S")} - For datagram {datagram_id} received unexpected ACK, resending message", flush=True)

                except socket.timeout:
                    print(f"Now: {datetime.now().strftime("%H:%M:%S")} - For datagram {datagram_id} received timeout waiting for response, resending message", flush=True)

                except Exception as e:
                    print(f"Now: {datetime.now().strftime("%H:%M:%S")} - For datagram {datagram_id}: Error during communication", flush=True)
                    print(e, flush=True)
                    break

            datagram_id += 1


if __name__ == '__main__':
    main()
