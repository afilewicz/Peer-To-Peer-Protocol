#include <filesystem>
#include <iostream>
#include <limits>
#include <string>
#include <iomanip>

#include "ResourceManager.h"
#include "exceptions/FileNotFoundException.h"
#include "UDPCommunicator.h"

void print_choices()
{
    std::cout << "1. Add resource" << std::endl;
    std::cout << "2. Remove resource" << std::endl;
    std::cout << "3. Display resources" << std::endl;
    std::cout << "4. Display remote resources" << std::endl;
    std::cout << "5. Broadcast" << std::endl;
    std::cout << "6. Exit program" << std::endl;
    std::cout << "7. Send request" << std::endl;
    std::cout << std::endl;
}

void print_formatted_resources(const std::map<std::string, Resource> &resources)
{
    std::cout << std::left << std::setw(5) << "" << std::setw(20) << "Resource Name" << std::setw(15) << "Size (bytes)"
              << std::setw(25) << "Time of Addition" << std::endl;
    std::cout << std::string(90, '-') << std::endl;

    int counter = 1;
    for (const auto &resource : resources)
    {
        std::time_t time = std::chrono::system_clock::to_time_t(resource.second.time_of_addition);

        std::cout << std::left << std::setw(5) << counter++ << std::setw(20) << resource.first << std::setw(15)
                  << resource.second.size << std::setw(25) << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
                  << std::endl;
    }
    std::cout << std::endl;
}

void print_formated_remote_resources(const std::map<std::string, std::vector<std::string>> &remote_resources)
{
    std::cout << std::left << std::setw(5) << "" << std::setw(20) << "IP Address" << std::setw(25) << "Resources" << std::endl;
    std::cout << std::string(50, '-') << std::endl;

    int counter = 1;
    for (const auto &remote_resource : remote_resources)
    {
        std::string resource_list;
        for (const auto &resource : remote_resource.second)
        {
            if (!resource_list.empty())
            {
                resource_list += ", ";
            }
            resource_list += resource;
        }
        std::cout << std::left << std::setw(5) << counter++ << std::setw(20) << remote_resource.first << std::setw(25) << resource_list << std::endl;
    }
    std::cout << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);

    if (port == 8888) {
        std::cerr << "Invalid port number" << std::endl;
        return 1;
    }

    ResourceManager manager;
    UDP_Communicator udp_communicator(port, manager);
    std::cout << "UDP Communicator initialized on port " << port << std::endl;
    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;

    udp_communicator.start_broadcast_thread();

    std::thread request_handler([&udp_communicator]()
                                {
        while (true) {
            try {
                udp_communicator.handle_request();
            } catch (const std::exception& e) {
                std::cerr << "Error handling request: " << e.what() << std::endl;
            }
        } });
    request_handler.detach();

    std::thread data_thread([&udp_communicator]()
                            {
        while(true) {
            try {
                P2PDataMessage data = udp_communicator.receive_data();
                udp_communicator.save_data(data);
            } catch(const std::exception& e) {
                std::cerr << "Data error: " << e.what() << std::endl;
            }
        } });
    data_thread.detach();

    while (true)
    {
        print_choices();
        std::cout << "Enter choice number: ";
        int choice;

        if (!(std::cin >> choice))
        {
            std::cout << std::endl;
            std::cout << "Invalid choice. Please enter a valid number." << std::endl;
            std::cout << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        std::cout << std::endl;

        if (choice == 1)
        {
            std::string name;
            std::string path;
            std::cout << "Enter resource name: ";
            std::cin >> name;
            std::cout << "Enter resource path: ";
            std::cin >> path;
            std::cout << std::endl;
            // name = "test_2.txt";
            // path = "test.txt";
            try {
                manager.add_resource(name, path);
            try
            {
                manager.add_local_resource(name, path);
                std::cout << "Resource added." << std::endl;
                std::cout << std::endl;
            }
            catch (const FileNotFoundException &e)
            {
                std::cout << e.what() << ", resource has not been added." << std::endl;
                std::cout << std::endl;
            }
            catch (const std::ios_base::failure &e)
            {
                std::cout << "File name cannot be a directory." << std::endl;
                std::cout << std::endl;
            }
            catch (const std::invalid_argument &e)
            {
                std::cout << e.what() << std::endl;
                std::cout << "Would you like to replace the existing resource? (y/n): ";
                char overwrite_choice;
                std::cin >> overwrite_choice;
                if (overwrite_choice == 'y')
                {
                    manager.add_local_resource(name, path, true);
                    std::cout << "Resource replaced." << std::endl;
                }
                else
                {
                    std::cout << "Resource has not been replaced." << std::endl;
                }
                std::cout << std::endl;
            }
        }
        else if (choice == 2)
        {
            std::string name;
            std::cout << "Enter resource name: ";
            std::cin >> name;
            try
            {
                manager.remove_resource(name);
                std::cout << "Resource: " << "'" << name << "'" << " removed." << std::endl;
                std::cout << std::endl;
            }
            catch (const std::invalid_argument &e)
            {
                std::cout << e.what() << std::endl;
                std::cout << std::endl;
            }
        }
        else if (choice == 3)
        {
            if (manager.get_local_resources().empty())
            {
                std::cout << "No resources to display." << std::endl;
                std::cout << std::endl;
            }
            else
            {
                print_formatted_resources(manager.get_local_resources());
            }
        }
        else if (choice == 4)
        {
            if (manager.get_remote_resources().empty())
            {
                std::cout << "No remote resources to display." << std::endl;
                std::cout << std::endl;
            }
            else
            {
                print_formated_remote_resources(manager.get_remote_resources());
            }
        }

        else if (choice == 5)
        {
            try
            {
                udp_communicator.send_broadcast_message();
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error initializing UDP communication: " << e.what() << std::endl;
            }
        }
        else if (choice == 6)
        {
            break;
        }
        else if (choice == 7)
        {
            std::string resource_name, target_address;
            uint16_t target_port;

            std::cout << "Enter resource name: ";
            std::cin >> resource_name;
            std::cout << "Enter target IP address: ";
            std::cin >> target_address;
            std::cout << "Enter target port: ";
            std::cin >> target_port;
            // resource_name = "test_2.txt";
            // target_address = "127.0.0.1";
            // target_port = 5555;

            // Wywołujemy metodę, która wyśle P2PRequestMessage
            try {
                udp_communicator.send_request(resource_name, target_address, target_port);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error sending request: " << e.what() << std::endl;
            }
            std::cout << std::endl;
        }
        else
        {
            std::cout << "Invalid choice.\n"
                      << std::endl;
        }
    }
    return 0;
}
