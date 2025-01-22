#pragma once

#include <map>
#include <string>

#include "Resource.h"

class ResourceManager
{
public:
    ResourceManager();

    void add_local_resource(const std::string &name, const std::string &path, bool replace = false);

    void add_received_resource(const std::string &name, const std::vector<u_char> &data, bool replace = false);

    void remove_resource(const std::string& name);

    const std::vector<std::string> get_resource_names() const;

    bool has_resource(const std::string &name) const;

    const std::map<std::string, Resource> &get_local_resources() const { return local_resources; }

    const std::vector<u_char> &get_resource_data(const std::string &resource_name) const;

    void add_remote_resource(const std::string &ip, const std::vector<std::string> &resources);

    const std::map<std::string, std::vector<std::string>> &get_remote_resources() const { return remote_resources; }

private:
    std::map<std::string, Resource> local_resources;
    std::map<std::string, std::vector<std::string>> remote_resources;
};
