#include "UDPCommunicator.h"

#include <thread>
#include <random>
#include <fstream>

#include "ResourceManager.h"

UDP_Communicator::UDP_Communicator(int port, ResourceManager &manager)
    : resource_manager(manager), port(port)
{

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        throw std::runtime_error("Failed to create socket");
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        close(sockfd);
        throw std::runtime_error("Failed to bind socket");
    }
}


UDP_Communicator::~UDP_Communicator() {
    stop_broadcast_thread();
    if (sockfd >= 0) {
        close(sockfd);
    }
}

void UDP_Communicator::send_request(const std::string &resource_name,
                                    const std::string &target_ip,
                                    uint16_t target_port)
{
    P2PRequestMessage request_message = {};
    request_message.header.message_type = static_cast<uint8_t>(MessageType::REQUEST);


    std::strncpy(request_message.resource_name,
                 resource_name.c_str(),
                 sizeof(request_message.resource_name) - 1);

    std::strncpy(request_message.additional_info,
                 "Requesting resource",
                 sizeof(request_message.additional_info) - 1);

    sockaddr_in target_addr = {};
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(target_port);
    if (inet_pton(AF_INET, target_ip.c_str(), &target_addr.sin_addr) <= 0)
    {
        throw std::runtime_error("Invalid target IP in send_request");
    }

    ssize_t sent_bytes = sendto(sockfd,
                                &request_message,
                                sizeof(request_message),
                                0,
                                (struct sockaddr *)&target_addr,
                                sizeof(target_addr));
    if (sent_bytes == -1)
    {
        throw std::runtime_error(std::string("Failed to send request: ") + strerror(errno));
    }

    std::cout << "Request sent to " << target_ip << ":" << target_port
              << " for resource: " << resource_name << std::endl;
}

void UDP_Communicator::handle_request()
{
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
    if (received_bytes < 0) {
        throw std::runtime_error("Failed to receive request");
    }

    std::string requested_resource = request_message.resource_name;
    std::string sender_ip = inet_ntoa(sender_addr.sin_addr);
    uint16_t sender_port = ntohs(sender_addr.sin_port);

    std::cout << "Request received for resource: " << requested_resource
              << " from " << sender_ip << ":" << sender_port << std::endl;

    if (resource_manager.has_resource(requested_resource))
    {
        std::cout << "Resource found. Sending..." << std::endl;
        send_file_sync(requested_resource, sender_ip, sender_port);
    }
    else
    {
        std::cout << "Resource not found 2: " << requested_resource << std::endl;
    }
}

void UDP_Communicator::send_to_host(const P2PDataMessage &message, const std::string &target_address, int target_port)
{
    sockaddr_in target_addr = {};
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(target_port);
    if (inet_pton(AF_INET, target_address.c_str(), &target_addr.sin_addr) <= 0)
    {
        throw std::runtime_error("Invalid target address");
    }

    ssize_t sent_bytes = sendto(
        sockfd,
        &message,
        sizeof(message),
        0,
        reinterpret_cast<sockaddr *>(&target_addr),
        sizeof(target_addr));

    if (sent_bytes == -1)
    {
        throw std::runtime_error(std::string("Failed to send data: ") + strerror(errno));
    }
    std::cout << "Data sent to " << target_address << ":" << target_port << std::endl;
}

void UDP_Communicator::send_file_sync(const std::string &resource_name,
                                      const std::string &target_address,
                                      uint16_t target_port)
{
    if (!resource_manager.has_resource(resource_name)) {
        std::cerr << "Resource not found 1: " << resource_name << std::endl;
        return;
    }

    const auto& resource_data = resource_manager.get_resource_data(resource_name);

    P2PDataMessage data_message = {};
    data_message.header.message_type = static_cast<uint8_t>(MessageType::DATA);
    std::strcpy(data_message.header.message_id, resource_name.c_str());
    size_t to_copy = std::min<size_t>(resource_data.size(), sizeof(data_message.data));
    std::memcpy(data_message.data, resource_data.data(), to_copy);

    try
    {
        send_to_host(data_message, target_address, target_port);
    }
    catch (const std::exception &e)
    {
        std::cerr << "[send_file_sync] Error sending chunk " << e.what() << std::endl;
        return;
    }

    std::cout << "[send_file_sync] Finished sending resource '" << resource_name << "'\n";
}


P2PDataMessage UDP_Communicator::receive_data() {
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

    if (received_bytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return {};
        }
        std::cerr << "Failed to receive data: " << strerror(errno) << std::endl;
        return {};
    }

    std::cout << "Data received from "
              << inet_ntoa(sender_addr.sin_addr) << ":" << ntohs(sender_addr.sin_port) << std::endl;
    std::string name = message.header.message_id;
    try {
        resource_manager.add_local_resource(name, name);
    }
    catch (const std::invalid_argument &e)
    {
        std::cout << e.what() << std::endl;
        std::cout << "Would you like to replace the existing resource? (y/n): ";
        char overwrite_choice;
        std::cin >> overwrite_choice;
        if (overwrite_choice == 'y')
        {
            resource_manager.add_local_resource(name, name, true);
            std::cout << "Resource replaced." << std::endl;
        }
        else
        {
            std::cout << "Resource has not been replaced." << std::endl;
        }
        std::cout << std::endl;
    }

    return message;
}

void UDP_Communicator::save_data(const P2PDataMessage& message) {
    std::string filename = message.header.message_id;

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    file.write(message.data, sizeof(message.data));
    file.close();

    std::cout << "Saved to " << filename << std::endl;
}


void UDP_Communicator::send_broadcast_message() {
    if (broadcast_running == false) {
        return;
    }

    int broadcastEnable = 1;
    if (setsockopt(broadcast_sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0)
    {
        perror("setsockopt failed");
        close(broadcast_sock);
        return;
    }

    P2PBroadcastMessage message = {};

    message.header.message_type = static_cast<uint8_t>(MessageType::BROADCAST);
    std::memcpy(message.header.sender_ip, &broadcast_address.sin_addr, 4);
    message.header.sender_port = ntohs(broadcast_address.sin_port);


    std::string resources;
    for (const auto &resource_name : resource_manager.get_resource_names())
    {
        if (!resources.empty())
        {
            resources += ", ";
        }
        resources += resource_name;
    }

    snprintf(
        message.broadcast_message,
        sizeof(message.broadcast_message),
        "Host %p broadcasts: %s", message.header.sender_ip, resources.c_str());

        ssize_t sent_bytes = sendto(
            broadcast_sock,
            &message,
            sizeof(message),
            0,
            (struct sockaddr*)&broadcast_address,
    sizeof(broadcast_address)
        );

    if (sent_bytes < 0)
    {
        std::cerr << "Failed to send broadcast message: " << strerror(errno) << std::endl;
    }
    else
    {
        std::cout << "Broadcast message sent: " << message.broadcast_message << std::endl;
    }
}

void UDP_Communicator::start_broadcast_thread()
{
    if (broadcast_running == true)
    {
        std::cerr << "Broadcast thread is already running." << std::endl;
        return;
    }

    broadcast_running = true;

    broadcast_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (broadcast_sock < 0)
    {
        std::cerr << "Failed to create socket." << strerror(errno) << std::endl;
        broadcast_running = false;
        return;
    }

    memset(&broadcast_address, 0, sizeof(broadcast_address));
    broadcast_address.sin_family = AF_INET;
    broadcast_address.sin_addr.s_addr = inet_addr("255.255.255.255");
    broadcast_address.sin_port = htons(8888);

    int opt = 1;
    if (setsockopt(broadcast_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "Failed to set socket options." << strerror(errno) << std::endl;
        close(broadcast_sock);
        broadcast_running = false;
        return;
    }

    if (bind(broadcast_sock, (struct sockaddr *)&broadcast_address, sizeof(broadcast_address)) < 0)
    {
        std::cerr << "Failed to bind socket." << strerror(errno) << std::endl;
        close(broadcast_sock);
        broadcast_running = false;
        return;
    }

    broadcast_thread = std::thread([this]()
                                   {
        P2PBroadcastMessage receivedMessage;

        while (broadcast_running) {

            sockaddr_in from_addr;
            socklen_t from_addr_len = sizeof(from_addr);

            ssize_t len = recvfrom(broadcast_sock, &receivedMessage, sizeof(receivedMessage), 0,
                                (struct sockaddr *)&from_addr, &from_addr_len);

            if (len < 0) {
                std::cerr << "recvfrom failed." << std::endl;
                broadcast_running = false;
                close(broadcast_sock);
                return;
            }

            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &from_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
            std::string sender_ip(ip_str);
            std::vector<std::string> resources;

            std::string message_str(receivedMessage.broadcast_message);
            size_t pos = message_str.find(": ");
            if (pos != std::string::npos)
            {
                std::string resource_list = message_str.substr(pos + 2);
                size_t start = 0, end;
                while ((end = resource_list.find(", ", start)) != std::string::npos)
                {
                    resources.push_back(resource_list.substr(start, end - start));
                    start = end + 2;
                }
                resources.push_back(resource_list.substr(start));
            }
            resource_manager.add_remote_resource(sender_ip, resources);


            std::cout << "Received broadcast message: " << receivedMessage.broadcast_message << "\n";
        }

        close(broadcast_sock);
        broadcast_running = false;
    });
}



void UDP_Communicator::stop_broadcast_thread() {
    broadcast_running = false;
    if (broadcast_thread.joinable()) {
        broadcast_thread.join();
    }
}
