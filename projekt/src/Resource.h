#pragma once

#include <string>
#include <array>

class Resource {
private:
    std::string name;
    std::array<std::byte> data;
};
