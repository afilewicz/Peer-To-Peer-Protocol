FROM gcc:4.9
COPY . /Sockets
WORKDIR /Sockets/
RUN gcc -o server server.c
ENTRYPOINT ["./server"]