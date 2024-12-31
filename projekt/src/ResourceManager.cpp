#include "ResourceManager.h"
#include <fstream>

ResourceManager::ResourceManager() { resources = std::map<std::string, Resource>(); }

void ResourceManager::add_resource(const std::string name, const std::string path) {
    std::ifstream file(path, std::ios::binary);
    std::vector<std::byte> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    resources[name] = Resource(name, data);
}

void ResourceManager::remove_resource(const std::string name) { resources.erase(name); }
