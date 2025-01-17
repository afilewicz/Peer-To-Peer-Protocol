#include "UDPCommunicator.h"

#include <thread>
#include <atomic>
#include <random>
#include <fstream>


#include "ResourceManager.h"


UDP_Communicator::UDP_Communicator(int port, ResourceManager& manager)
    : broadcast_running(false), transmission_running(false),
    resource_manager(manager) {
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

uint32_t UDP_Communicator::generate_message_id() {
    static std::random_device rd;
    static std::mt19937 generator(rd());
    static std::uniform_int_distribution<uint32_t> distribution(1, UINT32_MAX);

    return distribution(generator);
}

// void UDP_Communicator::start_transmission_thread(const std::string& resource_name, const std::string& target_address) {
//     transmission_running = true;
//
//     transmission_thread = std::thread([this, resource_name, target_address]() {
//         while (transmission_running) {
//             std::cout << "Sending resource: " << resource_name << " to " << target_address << std::endl;
//             // TODO: Dodaj logikę wysyłania danych UDP
//             transmission_running = false;
//         }
//     });
// }

void UDP_Communicator::start_transmission_thread(const std::string& resource_name, const std::string& target_address) {
    transmission_running = true;

    transmission_thread = std::thread([this, resource_name, target_address]() {
        if (!resource_manager.has_resource(resource_name)) {
            std::cerr << "Resource not found: " << resource_name << std::endl;
            transmission_running = false;
            return;
        }

        const auto& resource_data = resource_manager.get_resource_data(resource_name);
        size_t total_chunks = (resource_data.size() + 1023) / 1024;

        for (size_t i = 0; i < total_chunks && transmission_running; ++i) {
            P2PDataMessage data_message = {};
            data_message.header.message_type = 3; // Typ Data
            data_message.header.message_id = generate_message_id();
            std::memcpy(data_message.header.sender_ip, &address.sin_addr, 4);
            data_message.header.sender_port = ntohs(address.sin_port);

            data_message.chunk_id = i + 1;
            data_message.total_chunks = total_chunks;

            size_t chunk_start = i * 1024;
            size_t chunk_size = std::min<size_t>(1024, resource_data.size() - chunk_start);
            std::memcpy(data_message.data_chunk, resource_data.data() + chunk_start, chunk_size);

            try {
                send_to_host(data_message, target_address, 8081); // Zakładany port
            } catch (const std::exception& e) {
                std::cerr << "Error sending chunk " << data_message.chunk_id << ": " << e.what() << std::endl;
                break;
            }
        }
        transmission_running = false;
    });
}


void UDP_Communicator::send_to_host(const P2PDataMessage& message, const std::string& target_address, int target_port) {
    sockaddr_in target_addr = {};
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(target_port);
    if (inet_pton(AF_INET, target_address.c_str(), &target_addr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid target address");
    }

    ssize_t sent_bytes = sendto(
        sockfd,
        &message,
        sizeof(message),
        0,
        reinterpret_cast<sockaddr*>(&target_addr),
        sizeof(target_addr)
    );

    if (sent_bytes == -1) {
        throw std::runtime_error(std::string("Failed to send data: ") + strerror(errno));
    }
    std::cout << "Data sent to " << target_address << ":" << target_port << std::endl;
}

P2PDataMessage UDP_Communicator::receive_from_host() {
    P2PDataMessage message = {};
    sockaddr_in sender_addr = {};
    socklen_t sender_len = sizeof(sender_addr);

    ssize_t received_bytes = recvfrom(
        sockfd,
        &message,
        sizeof(message),
        0,
        reinterpret_cast<sockaddr*>(&sender_addr),
        &sender_len
    );

    if (received_bytes == -1) {
        throw std::runtime_error(std::string("Failed to receive data: ") + strerror(errno));
    }

    std::cout << "Data received from " << inet_ntoa(sender_addr.sin_addr) << ":" << ntohs(sender_addr.sin_port) << std::endl;

    std::string filename = std::to_string(message.header.message_id) + std::string("_received") + ".txt";

    // Otwórz plik w trybie zapisu (dodawanie bloków danych)
    std::ofstream file(filename, std::ios::binary | std::ios::app);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    // Zapisz odebrane dane do pliku
    file.write(message.data_chunk, sizeof(message.data_chunk));
    file.close();

    std::cout << "Chunk " << message.chunk_id << " saved to " << filename << std::endl;

    return message;
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
