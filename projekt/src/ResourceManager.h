#pragma once

#include <map>
#include <string>

#include "Resource.h"

class ResourceManager {
public:
    ResourceManager();

    void add_resource(const std::string name, const std::string path);
    void remove_resource(const std::string name);

private:
    std::map<std::string, Resource> resources;
};
