import argparse
import socket


def generate_message(declared_length):
    message = b''
    for i in range(declared_length - 2):
        message += bytes([65 + (i % 26)])
    return message


def parse_arguments(default_host: str, default_port: int) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='UDP server')

    parser.add_argument('-s', '--host', help=f"The server's hostname or IP address, default: {default_host}", default=default_host)
    parser.add_argument('-p', '--port', help=f"Port that will be used, default: {default_port}", default=default_port)

    return parser.parse_args()


def main():
    default_host = '127.0.0.1'
    default_port = 8000
    bufsize = 66000

    # Set the server port and IP address
    args = parse_arguments(default_host, default_port)

    # Change host and port if given by user, else default values: host = 127.0.0.1, port = 8000
    if args.host:
        host = args.host
    if args.port:
        port = int(args.port)

    print("Will listen on ", host, ":", port, flush=True)

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind((host, port))
        i=1
        while True:
            # Receive message
            data_address = s.recvfrom( bufsize )
            data = data_address[0]
            address = data_address[1]

            # Get message length from first two bytes
            data_length = int.from_bytes(data[:2], byteorder='big')
            print("Expected message length: ", data_length, flush=True)

            # Comparing the length of the message received from the first two bytes with the actual length of the received message
            if data_length != len(data):
                print("Length of datagram is incorrect", flush=True)
                response = "Mismatch between response lengths"
            else:
                response = "Response length is valid"

            # Generating expected message (A ... Z characters)
            message = generate_message(data_length)

            # Comparing received with generated message
            if message != data[2:]:
                print("Message is incorrect", flush=True)
                response = "Message is incorrect"

            # Check if data was received
            if not data:
                print("Error in datagram?", flush=True)
                response = "Error in datagram?"

            # Send response to client
            print(response, flush=True)
            s.sendto(response.encode('utf-8'), address)

            print('sending dgram #', i, flush=True)
            i+=1


if __name__ == "__main__":
    main()
