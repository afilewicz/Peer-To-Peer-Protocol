#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BSIZE 1024
#define bailout(s) {perror(s); fflush(stdout); exit(1); }

int moreWork(void) {
    return 1;
}

int main(int argc, char *argv[]) {
    int sock, msgsock, rval, ListenQueueSize=5;
    struct addrinfo *bindto_address;
    struct addrinfo hints;
    char buf[BSIZE];
    ssize_t bytes_received;

    char *ip_address = (argc > 1) ? argv[1] : "127.0.0.1";
    char *port = (argc > 2) ? argv[2] : "8888";

    // netdb.h documentation
    // int               ai_flags      Input flags.
    // int               ai_family     Address family of socket.
    // int               ai_socktype   Socket type.
    // int               ai_protocol   Protocol of socket.
    // socklen_t         ai_addrlen    Length of socket address.
    // struct sockaddr  *ai_addr       Socket address of socket.
    // char             *ai_canonname  Canonical name of service location.
    // struct addrinfo  *ai_next       Pointer to next in list.

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(ip_address, port, &hints, &bindto_address) != 0)
        bailout("Blad pobierania adresu");

    sock = socket(bindto_address->ai_family, bindto_address->ai_socktype, bindto_address->ai_protocol);
    if (sock == -1)
        bailout("Blad gniazda");

    if (bind(sock, bindto_address->ai_addr, bindto_address->ai_addrlen) == -1)
        bailout("Bląd bindowania strumienia gniazda");

    freeaddrinfo(bindto_address);

    listen(sock, ListenQueueSize);
    printf("Serwer nasłuchuje na porcie %s...\n", port);
    fflush(stdout);

    do {
        msgsock = accept(sock,(struct sockaddr *) 0, (socklen_t *) 0);
        if (msgsock == -1 ) {
            bailout("Błąd akceptacji");
        } else do {
            memset(buf, 0, sizeof buf);
            bytes_received = recv(msgsock, buf, sizeof(buf), 0);

            if (bytes_received == -1) {
                bailout("Błąd odbierania strumienia wiadomości");
            }

            if (bytes_received == 0) {
                printf("Zakończenie połączenia z klientem\n");
                fflush(stdout);
            } else {
                printf("Odebrano %zd bajtów\n", bytes_received);
                fflush(stdout);
            }

            sleep(1);
        } while (bytes_received != 0);

        close(msgsock);
    } while( moreWork() );

}