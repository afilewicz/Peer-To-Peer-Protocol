#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_BUFFOR_SIZE 66000

void fill_msg_buffor(char* buffer, int msg_length) {
    uint16_t len = htons((uint16_t)msg_length);
    memcpy(buffer, &len, 2);

    for(int i = 2; i < msg_length; ++i) {
        buffer[i] = 'A' + (i - 2) % 26;
    }
}

int main(int argc, char *argv[])
{
    int sock_d, length;
    struct sockaddr_in server_addr;
    char client_msg[MAX_BUFFOR_SIZE], server_resp[MAX_BUFFOR_SIZE];

    if (argc < 2) {
        printf("Usage: %s <port>\n", argv[0]);
        fflush(stdout);
        exit(1);
    }

    sock_d = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_d == -1) {
        perror("Error while opening datagram socket");
        fflush(stdout);
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("172.21.16.2");
    server_addr.sin_port = htons(atoi(argv[1]));

    length = sizeof server_addr;

    for(int size = 65000; size < MAX_BUFFOR_SIZE; size += 1) {
        
        fill_msg_buffor(client_msg, size);
        printf("Sending message with size: %i\n", size);
        fflush(stdout);
        
        if(sendto(sock_d, client_msg, size, 0, 
            (struct sockaddr *) &server_addr, sizeof server_addr) == -1) {
            perror("Error while sending datagram message");
            fflush(stdout);
            exit(1);
        }

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
