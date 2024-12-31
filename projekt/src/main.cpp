#include <filesystem>
#include <iostream>
#include <limits>
#include <string>

#include "ResourceManager.h"
#include "exceptions/FileNotFoundException.h"

void print_choices() {
    std::cout << "1. Add resource" << std::endl;
    std::cout << "2. Remove resource" << std::endl;
    std::cout << "3. Display resources" << std::endl;
    std::cout << "4. Broadcast" << std::endl;
    std::cout << "5. Exit program" << std::endl;
    std::cout << std::endl;
}

void print_formatted_resources(const std::map<std::string, Resource>& resources) {
    std::cout << std::left << std::setw(5) << "" << std::setw(20) << "Resource Name" << std::setw(15) << "Size (bytes)"
              << std::setw(25) << "Time of Addition" << std::endl;
    std::cout << std::string(65, '-') << std::endl;

    int counter = 1;
    for (const auto& resource: resources) {
        std::time_t time = std::chrono::system_clock::to_time_t(resource.second.time_of_addition);

        std::cout << std::left << std::setw(5) << counter++ << std::setw(20) << resource.first << std::setw(15)
                  << resource.second.size << std::setw(25) << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
                  << std::endl;
    }
    std::cout << std::endl;
}


int main() {
    ResourceManager manager;
    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
    while (true) {
        print_choices();
        std::cout << "Enter choice number: ";
        int choice;

        if (!(std::cin >> choice)) {
            std::cout << std::endl;
            std::cout << "Invalid choice. Please enter a valid number." << std::endl;
            std::cout << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        std::cout << std::endl;

        if (choice == 1) {
            std::string name;
            std::string path;
            std::cout << "Enter resource name: ";
            std::cin >> name;
            std::cout << "Enter resource path: ";
            std::cin >> path;
            std::cout << std::endl;
            try {
                manager.add_resource(name, path);
                std::cout << "Resource added." << std::endl;
                std::cout << std::endl;
            } catch (const FileNotFoundException& e) {
                std::cout << e.what() << ", resource has not been added." << std::endl;
                std::cout << std::endl;
            } catch (const std::ios_base::failure& e) {
                std::cout << "File name cannot be a directory." << std::endl;
                std::cout << std::endl;
            } catch (const std::invalid_argument& e) {
                std::cout << e.what() << std::endl;
                std::cout << "Would you like to replace the existing resource? (y/n): ";
                char overwrite_choice;
                std::cin >> overwrite_choice;
                if (overwrite_choice == 'y') {
                    manager.add_resource(name, path, true);
                    std::cout << "Resource replaced." << std::endl;
                } else {
                    std::cout << "Resource has not been replaced." << std::endl;
                }
                std::cout << std::endl;
            }

        } else if (choice == 2) {
            std::string name;
            std::cout << "Enter resource name: ";
            std::cin >> name;
            manager.remove_resource(name);
        } else if (choice == 3) {
            if (manager.get_resources().empty()) {
                std::cout << "No resources to display." << std::endl;
                std::cout << std::endl;
            } else {
                print_formatted_resources(manager.get_resources());
            }
        } else if (choice == 4) {
            // TODO: Implement broadcasting
            std::cout << "Broadcasting" << std::endl;
        } else if (choice == 5) {
            break;
        } else {
            std::cout << "Invalid choice" << std::endl;
            std::cout << std::endl;
        }
    }
}
