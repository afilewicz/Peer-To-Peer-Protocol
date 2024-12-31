#pragma once

#include <string>
#include <vector>

class Resource {
public:
    Resource() = default;
    Resource(std::string name, std::vector<u_char> data) : name(name), data(data) {};

private:
    std::string name;
    std::vector<u_char> data;
};
