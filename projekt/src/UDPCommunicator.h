#pragma once

#include <atomic>
#include <string>
#include <vector>
#include <chrono>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <bits/std_thread.h>

class UDP_Communicator {
public:
    explicit UDP_Communicator(int port);
    ~UDP_Communicator();
    void start_broadcast_thread();
    void start_transmission_thread(
        const std::string& resource_name,
        const std::string& target_address
        );
    void stop_threads();
private:
    int sockfd;
    struct sockaddr_in address{};

    mutable std::atomic<bool> broadcast_running;
    mutable std::atomic<bool> transmission_running;

    std::thread broadcast_thread;
    std::thread transmission_thread;
};
