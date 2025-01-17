#pragma once

#include <chrono>
#include <string>
#include <vector>

struct Resource {
    Resource() = default;
    Resource(std::string name, std::vector<u_char> data, std::string ip_address) : name(name), data(data), ip_address(ip_address) {};

    std::string name;
    std::vector<u_char> data;
    std::string ip_address;
    size_t size = 0;
    std::chrono::time_point<std::chrono::system_clock> time_of_addition = std::chrono::system_clock::now();
};
