import argparse
import socket
import struct


HOST = '127.0.0.1'
PORT = 8000
BUFSIZE = 512


def parse_arguments():
    parser = argparse.ArgumentParser(description='UDP client')

    parser.add_argument('-s', '--host', help=f"The server's hostname or IP address, default: {HOST}", default=HOST)
    parser.add_argument('-p', '--port', help=f"Port that will be used, default: {PORT}", default=PORT)

    return parser.parse_args()


def handle_datagram(datagram):
    received_bit = struct.unpack('!B', datagram[:1])[0]
    return received_bit


def main():
    args = parse_arguments()

    if args.host:
        host = args.host
    if args.port:
        port = int(args.port)

    print("Will listen on ", host, ":", port, flush=True)

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind((host, port))

        excpected_bit = 0

        while True:
            try:
                print()
                data, address = s.recvfrom( BUFSIZE )

                received_bit = handle_datagram(data)

                if received_bit == excpected_bit:
                    print(f"Received correct bit: {received_bit}", flush=True)
                    ack_message = f"ACK {excpected_bit}"
                    s.sendto(ack_message.encode("utf-8"), address)
                    print(f"Sended {ack_message}", flush=True)
                    excpected_bit = 1 - excpected_bit
                else:
                    print(f"Received incorrect bit: {received_bit}. Waiting for retransmission...", flush=True)
                    return False

            except socket.timeout:
                print("Socket timed out", flush=True)

            except Exception:
                print("Error in server communication", flush=True)


if __name__ == "__main__":
    main()
