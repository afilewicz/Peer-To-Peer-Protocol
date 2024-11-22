import argparse
import socket
import struct


HOST = '127.0.0.1'
PORT = 8000
BUFSIZE = 66000


# def generate_message(declared_length):
#     message = b''
#     for i in range(declared_length - 2):
#         message += bytes([65 + (i % 26)])
#     return message


def parse_arguments():
    parser = argparse.ArgumentParser(description='UDP client')

    parser.add_argument('-s', '--host', help=f"The server's hostname or IP address, default: {HOST}", default=HOST)
    parser.add_argument('-p', '--port', help=f"Port that will be used, default: {PORT}", default=PORT)

    return parser.parse_args()

def send_message(alternating_bit_protocol):
    ack_message = f"ACK {alternating_bit_protocol}"
    print(f"Sending ACK: {ack_message}", flush=True)
    return ack_message

def handle_datagram(datagram, expected_bit):
    received_bit = struct.unpack('!B', datagram[:1])[0]
    if received_bit == expected_bit:
        print(f"Received correct bit: {received_bit}", flush=True)
        return True
    else:
        print(f"Received incorrect bit: {received_bit}. Waiting for retransmission...", flush=True)
        return False


def main():
    args = parse_arguments()

    if args.host:
        host = args.host
    if args.port:
        port = int(args.port)

    print("Will listen on ", host, ":", port, flush=True)

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind((host, port))
        # i=1

        excpected_bit = 0

        while True:
            try:
                data_address = s.recvfrom( BUFSIZE )

                data = data_address[0]
                address = data_address[1]

                if handle_datagram(data, excpected_bit):
                    ack_message = send_message(excpected_bit)
                    s.sendto(ack_message.encode("utf-8"), address)

                    excpected_bit = 1 - excpected_bit

            except socket.timeout:
                print("Socket timed out", flush=True)

            except Exception:
                print("Error in server communication", flush=True)

           # data_length = int.from_bytes(data[:2], byteorder='big')
            # print("Expected message length: ", data_length, flush=True)
            #
            # if data_length != len(data):
            #     print("Length of datagram is incorrect", flush=True)
            #     response = "Mismatch between response lengths"
            # else:
            #     response = "Response length is valid"
            #
            # message = generate_message(data_length)
            #
            # if message != data[2:]:
            #     print("Message is incorrect", flush=True)
            #     response = "Message is incorrect"
            #
            # if not data:
            #     print("Error in datagram?", flush=True)
            #     response = "Error in datagram?"
            #
            # print(response, flush=True)
            # s.sendto(response.encode('utf-8'), address)
            #
            # print('sending dgram #', i, flush=True)
            # i+=1


if __name__ == "__main__":
    main()
