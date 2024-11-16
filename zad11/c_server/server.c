#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int is_response_length_valid(char* buffer, int received_size) {
    uint16_t len;
    memcpy(&len, buffer, 2);
    len = ntohs(len);
    printf("Expected: %d, got: %d\n", len, received_size);
    fflush(stdout);
    return len == received_size; 
}

int main(int argc, char* argv[])
{
    int sock_d, length, response_len;
    struct sockaddr_in server_addr, client_addr;
    char msg_received[66000], response[1024];

    sock_d = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_d < 0) {
        perror("Error while opening datagram socket");
        fflush(stdout);
        exit(1);
    }
    printf("Socket created\n");
    fflush(stdout);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (argc < 2) {
        printf("No port specified, using default 8000\n");
        fflush(stdout);
        server_addr.sin_port = htons(8000);
    } else
        server_addr.sin_port = htons(atoi(argv[1]));

    length = sizeof server_addr; 

    if(bind(sock_d, (struct sockaddr *)&server_addr, length) == -1) {
        perror("Error while binding datagram socket");
        fflush(stdout);
        exit(1);
    }

    while(1) {
        length = sizeof client_addr;
        response_len = recvfrom(sock_d, msg_received, sizeof msg_received, 0, 
                (struct sockaddr*)&client_addr, &length);

        if(response_len == -1) {
            perror("Error while receiving message");
            fflush(stdout);
            exit(1);
        }

        if (is_response_length_valid(msg_received, response_len))
            strcpy(response, "Response length is valid");
        else {
            strcpy(response, "Mismatch between response lengths");
            exit(1);
        }

        if(sendto(sock_d, response, sizeof response, 0, 
                (struct sockaddr *) &client_addr, sizeof client_addr) == -1) {
            perror("Error while sending datagram message");
            fflush(stdout);
            exit(1);
        }
    }
    
    close(sock_d);
    exit(0);
}
