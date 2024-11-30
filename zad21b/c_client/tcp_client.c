#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

#define BSIZE 100

void log_info(struct timeval start, struct timeval end, int pckg_num) {
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + microseconds * 1e-6;
    printf("Package number: %d\n", pckg_num);
    fflush(stdout);
    printf("Time stamp: %ld.%06ld\n", end.tv_sec, end.tv_usec);
    fflush(stdout);
    printf("Time difference: %f\n", elapsed);
    fflush(stdout);
    printf("\n");
    fflush(stdout);
}

int main(int argc, char* argv[]) {
    int sock;
    struct sockaddr_in server;
    char buf[BSIZE];
    struct timeval start, end;

    if (argc != 3) {
        printf("Usage: %s <server> <port>\n", argv[0]);
        fflush(stdout);
        exit(1);
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Error while opening stream socket");
        fflush(stdout);
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sock, (struct sockaddr *) &server, sizeof server) == -1) {
        perror("Error while connecting stream socket");
        fflush(stdout);
        exit(1);
    }

    memset(buf, 0, BSIZE);
    gettimeofday(&start, NULL);

    for (int i = 0; i < 200; i++) {
        if (send(sock, buf, sizeof buf, 0) == -1) {
            perror("Error while sending stream message");
            fflush(stdout);
            exit(1);
        }

        gettimeofday(&end, NULL);
        log_info(start, end, i);
        start = end;
    }
    close(sock);
}
