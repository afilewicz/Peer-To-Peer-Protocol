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
#include "ResourceManager.h"

struct P2PHeader {
    uint8_t message_type;
    uint32_t message_id;
    uint8_t sender_ip[4];
    uint16_t sender_port;
    uint8_t receiver_ip[4];
    uint16_t receiver_port;
};

struct P2PBroadcastMessage {
    P2PHeader header;
    char broadcast_message[256];
};

struct P2PRequestMessage {
    P2PHeader header;
    char resource_name[64];
    char additional_info[128];
};

struct P2PResponseMessage {
    P2PHeader header;
    char resource_name[64];
    uint8_t status_code;
    char response_data[512];
};

struct P2PDataMessage {
    P2PHeader header;
    uint32_t chunk_id;
    uint32_t total_chunks;
    char data_chunk[1024];
};

class UDP_Communicator {
public:
    UDP_Communicator(int port, ResourceManager &manager);

    ~UDP_Communicator();
    static uint32_t generate_message_id();
    void start_broadcast_thread();
    void start_transmission_thread(
        const std::string& resource_name,
        const std::string& target_address
        );

    void send_to_host(const P2PDataMessage &message, const std::string &target_address, int target_port);

    P2PDataMessage receive_from_host();

    void stop_threads();
private:
    int sockfd;
    struct sockaddr_in address{};

    mutable std::atomic<bool> broadcast_running;
    mutable std::atomic<bool> transmission_running;

    std::thread broadcast_thread;
    std::thread transmission_thread;

    ResourceManager& resource_manager;
};
