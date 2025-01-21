#include <fstream>

#include "ResourceManager.h"
#include "exceptions/FileNotFoundException.h"

#include <iostream>

ResourceManager::ResourceManager() { resources = std::map<std::string, Resource>(); }

void ResourceManager::add_resource(const std::string& name, const std::string& path, bool replace) {
    if (resources.find(name) != resources.end() && !replace) {
        throw std::invalid_argument("Resource with name " + name + " already exists.");
    }
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw FileNotFoundException();
    }
    std::vector<u_char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    Resource resource = Resource(name, data);
    resource.size = data.size();
    resources[name] = resource;
    file.close();
}


void ResourceManager::remove_resource(const std::string& name) {
    if (resources.find(name) == resources.end()) {
        throw std::invalid_argument("Resource with name " + name + " does not exist.");
    }
    resources.erase(name);
}

const std::vector<std::string> ResourceManager::get_resource_names() const {
    std::vector<std::string> names;
    for (const auto& resource: resources) {
        names.push_back(resource.first);
    }
    return names;
}

bool ResourceManager::has_resource(const std::string& name) const {
    return resources.contains(name);
}

const std::vector<u_char>& ResourceManager::get_resource_data(const std::string& resource_name) const {
    auto it = resources.find(resource_name);
    if (it == resources.end()) {
        throw std::invalid_argument("Resource with name " + resource_name + " does not exist.");
    }
    return it->second.data;
}