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


enum class MessageType {
    REQUEST,
    DATA,
    BROADCAST,
};

struct P2PHeader {
    uint8_t message_type;
    char message_id[32];
    uint8_t sender_ip[4];
    uint16_t sender_port;
    uint8_t receiver_ip[4];
    uint16_t receiver_port;
};

struct P2PBroadcastMessage
{
    P2PHeader header;
    char broadcast_message[256];
};

struct P2PRequestMessage
{
    P2PHeader header;
    char resource_name[64];
    char additional_info[128];
};

struct P2PResponseMessage
{
    P2PHeader header;
    char resource_name[64];
    uint8_t status_code;
    char response_data[512];
};

struct P2PDataMessage
{
    P2PHeader header;
    size_t data_length;
    char data[32000];
};

class UDP_Communicator
{
public:
    UDP_Communicator(int port, ResourceManager &manager);

    ~UDP_Communicator();

    void send_broadcast_message();

    void start_broadcast_thread();

    void stop_broadcast_thread();

    void send_to_host(const P2PDataMessage &message, const std::string &target_address, int target_port);

    void send_file_sync(const std::string &resource_name, const std::string &target_address, uint16_t target_port);

    void dispatch_message();

    P2PDataMessage receive_data(const P2PDataMessage& data_message, const sockaddr_in& sender_addr);

    void save_data(const P2PDataMessage &message);

    void send_request(const std::string &resource_name, const std::string &target_ip, uint16_t target_port);

    void handle_request(const P2PRequestMessage& request_message, const sockaddr_in& sender_addr);


private:
    int port;

    int sockfd;
    struct sockaddr_in address
    {
    };

    int data_sock;
    sockaddr_in data_address;

    int broadcast_sock;
    struct sockaddr_in broadcast_address
    {
    };
    mutable std::atomic<bool> broadcast_running;
    std::thread broadcast_thread;

    ResourceManager &resource_manager;
};
