import argparse
import socket
import struct

from datetime import datetime


def parse_arguments(default_host: str, default_port: int):
    parser = argparse.ArgumentParser(description='UDP server with Alternating Bit Protocol')

    parser.add_argument('-s', '--host', help=f"The server's hostname or IP address, default: {default_host}", default=default_host)
    parser.add_argument('-p', '--port', help=f"Port that will be used, default: {default_port}", default=default_port)

    return parser.parse_args()


def handle_datagram(datagram):
    received_bit = struct.unpack('!B', datagram[:1])[0]
    return received_bit


def main():
    default_host = '127.0.0.1'
    default_port = 8000
    bufsize = 512

    args = parse_arguments(default_host, default_port)

    # Change host and port if given by user, else default values: host = 127.0.0.1, port = 8000
    if args.host:
        host = args.host
    if args.port:
        port = int(args.port)

    print(f"Now: {datetime.now().strftime("%H:%M:%S")} - Will listen on ", host, ":", port, flush=True)

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind((host, port))

        expected_bit = 0

        while True:
            try:
                print(flush=True)

                # Receive message from client
                data, address = s.recvfrom(bufsize)

                # Get ABP from message
                received_bit = handle_datagram(data)

                if received_bit == expected_bit:
                    print(f"Now: {datetime.now().strftime("%H:%M:%S")} - Received correct bit: {received_bit}", flush=True)

                    # Send response to client
                    ack_message = f"ACK {expected_bit}"
                    s.sendto(ack_message.encode("utf-8"), address)

                    print(f"Now: {datetime.now().strftime("%H:%M:%S")} - Sent {ack_message}", flush=True)

                    # Change expected bit (0 -> 1 or 1 -> 0)
                    expected_bit = 1 - expected_bit
                else:
                    print(f"Now: {datetime.now().strftime("%H:%M:%S")} - Received incorrect bit: {received_bit}. Waiting for retransmission...", flush=True)
                    return False

            except socket.timeout:
                print(f"Now: {datetime.now().strftime("%H:%M:%S")} - Socket timed out", flush=True)

            except Exception:
                print(f"Now: {datetime.now().strftime("%H:%M:%S")} - Error in server communication", flush=True)


if __name__ == "__main__":
    main()
