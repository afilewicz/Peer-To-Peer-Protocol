import socket
import sys

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
# BUFSIZE = 512
BUFSIZE = 1024

def generate_message(declared_length):
    message = b''
    for i in range(declared_length - 2):
      message = ''.join(chr(65 + (i % 26))).encode('utf-8')
    return message

def main():
    if len(sys.argv) < 2:
        print("no port, using 8000")
        port=8000
    else:
        port = int( sys.argv[1] )

    print("Will listen on ", HOST, ":", port)


    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind((HOST, port))
        i=1
        while True:
            data_address = s.recvfrom( BUFSIZE )
            data = data_address[0]
            address = data_address[1]

            data_length = int.from_bytes(data[:2], byteorder='big')
            response = struct.pack('>H', data_length)
            print("Expected message length: ", data_length)

            if data_length != len(data):
                print("Length of datagram is incorrect")
                response = struct.pack('>H', 0)

            print( "Message from Client:{}".format(data[2:]) )
            print( "Client IP Address:{}".format(address) )

            message = generate_message(data_length)

            if message != data[2:]:
                print("Message is incorrect")
                response = struct.pack('>H', 0)

            if not data:
                print("Error in datagram?")
                response = struct.pack('>H', 0)

            s.sendto(response, address)
            print('sending dgram #', i)
            i+=1

if __name__ == "__main__":
    main()
