#pragma once

#include <map>
#include <string>

#include "Resource.h"

class ResourceManager {
public:
    ResourceManager();

    void add_resource(const std::string& name, const std::string& path, bool replace = false);
    void remove_resource(const std::string& name);
    const std::vector<std::string> get_resource_names() const;
    const std::map<std::string, Resource>& get_resources() const { return resources; }

private:
    std::map<std::string, Resource> resources;
};
