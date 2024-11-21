#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_BUFFOR_SIZE 66000

void fill_msg_buffor(char* buffer, int msg_length) {
    // Set the first two bytes to the message length
    uint16_t len = htons((uint16_t)msg_length);
    memcpy(buffer, &len, 2);

    // Fill the rest of the buffer with 'A'..'Z' characters
    for(int i = 2; i < msg_length; ++i) {
        buffer[i] = 'A' + (i - 2) % 26;
    }
}

int main(int argc, char *argv[])
{
    int sock_d, length;
    struct sockaddr_in server_addr;
    char client_msg[MAX_BUFFOR_SIZE], server_resp[MAX_BUFFOR_SIZE];

    // Check if the server IP and port were provided
    if (argc < 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        fflush(stdout);
        exit(1);
    }

    // Create a datagram socket
    sock_d = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_d == -1) {
        perror("Error while opening datagram socket");
        fflush(stdout);
        exit(1);
    }

    // Set the server port and IP address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    length = sizeof server_addr;

    for(int size = 3; size < MAX_BUFFOR_SIZE; size += 1) {
        
        fill_msg_buffor(client_msg, size);
        printf("Sending message with size: %i\n", size);
        fflush(stdout);
        
        // Send the message to the server
        if(sendto(sock_d, client_msg, size, 0, 
            (struct sockaddr *) &server_addr, sizeof server_addr) == -1) {
            perror("Error while sending datagram message");
            fflush(stdout);
            exit(1);
        }

        // Receive the server's response
        if(recvfrom(sock_d, server_resp, sizeof server_resp, 0, 
            (struct sockaddr*) &server_addr, &length) == -1) {
            perror("Error while receiving server response");
            fflush(stdout);
            exit(1);
        }

        printf("Server's response: %s\n", server_resp);
        fflush(stdout);

    }
    close(sock_d);
    exit(0);

}
