#include <fstream>

#include "ResourceManager.h"
#include "exceptions/FileNotFoundException.h"

#include <iostream>

ResourceManager::ResourceManager() {}

void ResourceManager::add_local_resource(const std::string &name, const std::string &path, bool replace)
{
    if (local_resources.find(name) != local_resources.end() && !replace)
    {
        throw std::invalid_argument("Resource with name " + name + " already exists.");
    }
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        throw FileNotFoundException();
    }
    std::vector<u_char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    Resource resource = Resource(name, data, local_ip);
    resource.size = data.size();
    local_resources[name] = resource;
    file.close();
}

void ResourceManager::set_local_ip(const std::string &ip)
{
    local_ip = ip;
}

void ResourceManager::remove_resource(const std::string &name)
{
    if (local_resources.find(name) == local_resources.end())
    {
        throw std::invalid_argument("Resource with name " + name + " does not exist.");
    }
    local_resources.erase(name);
}

const std::vector<std::string> ResourceManager::get_resource_names() const
{
    std::vector<std::string> names;
    for (const auto &resource : local_resources)
    {
        names.push_back(resource.first);
    }
    return names;
}

bool ResourceManager::has_resource(const std::string &name) const
{
    return local_resources.contains(name);
}

const std::vector<u_char> &ResourceManager::get_resource_data(const std::string &resource_name) const
{
    auto it = local_resources.find(resource_name);
    if (it == local_resources.end())
    {
        throw std::invalid_argument("Resource with name " + resource_name + " does not exist.");
    }
    return it->second.data;
}

void ResourceManager::add_remote_resource(const std::string &ip, const std::vector<std::string> &resources)
{
    remote_resources[ip] = resources;
}