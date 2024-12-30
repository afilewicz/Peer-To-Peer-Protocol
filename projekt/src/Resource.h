#pragma once

#include <string>
#include <vector>

class Resource {
public:
    Resource(std::string name, std::vector<std::byte> data) : name(name), data(data) {};

private:
    std::string name;
    std::vector<std::byte> data;
};
