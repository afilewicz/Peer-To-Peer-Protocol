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

std::string UDP_Communicator::get_local_ip() const {
    char buffer[INET_ADDRSTRLEN];
    sockaddr_in temp_address = {};
    socklen_t len = sizeof(temp_address);

    sockaddr_in external_address = {};
    external_address.sin_family = AF_INET;
    external_address.sin_port = htons(53);
    inet_pton(AF_INET, "8.8.8.8", &external_address.sin_addr);

    if (connect(sockfd, reinterpret_cast<sockaddr*>(&external_address), sizeof(external_address)) == -1) {
        throw std::runtime_error("Failed to connect socket for local IP detection: " + std::string(strerror(errno)));
    }

    if (getsockname(sockfd, reinterpret_cast<sockaddr*>(&temp_address), &len) == -1) {
        throw std::runtime_error("Failed to get local IP: " + std::string(strerror(errno)));
    }

    if (!inet_ntop(AF_INET, &temp_address.sin_addr, buffer, sizeof(buffer))) {
        throw std::runtime_error("Failed to convert IP to string: " + std::string(strerror(errno)));
    }

    return std::string(buffer);
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
                send_to_host(data_message, target_address, 8081);
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

    std::ofstream file(filename, std::ios::binary | std::ios::app);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    file.write(message.data_chunk, sizeof(message.data_chunk));
    file.close();

    std::cout << "Chunk " << message.chunk_id << " saved to " << filename << std::endl;

    return message;
}



void UDP_Communicator::start_broadcast_thread() {
    if (broadcast_running == true) {
        std::cerr << "Broadcast thread is already running." << std::endl;
        return;
    }

    broadcast_running = true;

    broadcast_thread = std::thread([this]() {
        // Enable the socket for broadcasting
        int broadcast_enable = 1;

        if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
            std::cerr << "Failed to set socket option SO_BROADCAST." << std::endl;
            broadcast_running = false;
            return;
        }

        struct sockaddr_in broadcast_addr;
        broadcast_addr.sin_family = AF_INET;
        broadcast_addr.sin_port = htons(0);
        broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

        while (broadcast_running) {
            std::cout << "Broadcasting resource information..." << std::endl;

            // Prepare the broadcast message
            P2PBroadcastMessage broadcast_message{};
            broadcast_message.header.message_type = 0; // Type Broadcast
            broadcast_message.header.message_id = generate_message_id();
            std::memcpy(broadcast_message.header.sender_ip, &address.sin_addr.s_addr, 4);
            broadcast_message.header.sender_port = htons(address.sin_port);

            std::string resource_list;
            for (const auto& resource_name : resource_manager.get_resource_names()) {
                resource_list += resource_name + ",";
            }

            // Remove last comma from resource_list
            if (!resource_list.empty()) {
                resource_list.pop_back();
            }

            snprintf(
                broadcast_message.broadcast_message,
                sizeof(broadcast_message.broadcast_message),
                "Resources: %s",
                resource_list.c_str()
            );

            // Send the broadcast message
            ssize_t sent_bytes = sendto(
                sockfd,
                &broadcast_message,
                sizeof(broadcast_message),
                0,
                reinterpret_cast<sockaddr*>(&broadcast_addr),
                sizeof(broadcast_addr)
            );

            if (sent_bytes < 0) {
                std::cerr << "Failed to send broadcast message: " << strerror(errno) << std::endl;
            } else {
                std::cout << "Broadcast message sent: " << broadcast_message.broadcast_message << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
        broadcast_running = false;
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

void UDP_Communicator::send_request(const std::string& resource_name, const std::string& target_ip, uint16_t target_port) {
    P2PRequestMessage request_message = {};
    request_message.header.message_type = 1; // Typ: Request
    request_message.header.message_id = generate_message_id();
    std::memcpy(request_message.header.sender_ip, &address.sin_addr, 4);
    request_message.header.sender_port = ntohs(address.sin_port);
    std::memset(request_message.header.receiver_ip, 0, 4); // Opcjonalne, jeśli nie znamy odbiorcy
    request_message.header.receiver_port = target_port;

    std::strncpy(request_message.resource_name, resource_name.c_str(), sizeof(request_message.resource_name) - 1);
    std::strncpy(request_message.additional_info, "Requesting resource", sizeof(request_message.additional_info) - 1);

    sockaddr_in target_addr = {};
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(target_port);
    if (inet_pton(AF_INET, target_ip.c_str(), &target_addr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid target address");
    }

    ssize_t sent_bytes = sendto(
        sockfd,
        &request_message,
        sizeof(request_message),
        0,
        reinterpret_cast<sockaddr*>(&target_addr),
        sizeof(target_addr)
    );

    if (sent_bytes == -1) {
        throw std::runtime_error(std::string("Failed to send request: ") + strerror(errno));
    }

    std::cout << "Request sent to " << target_ip << ":" << target_port << " for resource: " << resource_name << std::endl;
}


void UDP_Communicator::handle_request() {
    P2PRequestMessage request_message = {};
    sockaddr_in sender_addr = {};
    socklen_t sender_len = sizeof(sender_addr);

    ssize_t received_bytes = recvfrom(
        sockfd,
        &request_message,
        sizeof(request_message),
        0,
        reinterpret_cast<sockaddr*>(&sender_addr),
        &sender_len
    );

    if (received_bytes == -1) {
        throw std::runtime_error(std::string("Failed to receive request: ") + strerror(errno));
    }

    std::string requested_resource = request_message.resource_name;
    std::string sender_ip = inet_ntoa(sender_addr.sin_addr);
    uint16_t sender_port = ntohs(sender_addr.sin_port);

    std::cout << "Request received for resource: " << requested_resource
              << " from " << sender_ip << ":" << sender_port << std::endl;

    if (resource_manager.has_resource(requested_resource)) {
        std::cout << "Resource found. Sending..." << std::endl;
        start_transmission_thread(requested_resource, sender_ip);
    } else {
        std::cout << "Resource not found: " << requested_resource << std::endl;
    }
}
