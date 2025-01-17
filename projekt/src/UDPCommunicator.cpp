#include "UDPCommunicator.h"

#include <thread>
#include <atomic>


UDP_Communicator::UDP_Communicator(int port)
    : broadcast_running(false), transmission_running(false){
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        close(sockfd);
        throw std::runtime_error("Failed to bind socket");
    }
}

UDP_Communicator::~UDP_Communicator() {
    stop_threads();
    if (sockfd >= 0) {
        close(sockfd);
    }
}

void UDP_Communicator::start_transmission_thread(const std::string& resource_name, const std::string& target_address) {
    transmission_running = true;
    transmission_thread = std::thread([this, resource_name, target_address]() {
        while (transmission_running) {
            std::cout << "Sending resource: " << resource_name << " to " << target_address << std::endl;
            // TODO: Dodaj logikę wysyłania danych UDP
            transmission_running = false;
        }
    });
}

void UDP_Communicator::start_broadcast_thread() {
    broadcast_running = true;
    broadcast_thread = std::thread([this]() {
        while (broadcast_running) {
            std::cout << "Broadcasting resource information..." << std::endl;
            // TODO: rozgłaszanie
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });
}

void UDP_Communicator::stop_threads() {
    broadcast_running = false;
    transmission_running = false;

    if (broadcast_thread.joinable()) {
        broadcast_thread.join();
    }

    if (transmission_thread.joinable()) {
        transmission_thread.join();
    }
}
