#pragma once
#include <yaml-cpp/yaml.h>

class YamlAPI
{
public:
    std::string ReadString(std::string group, std::string key);
    double ReadDouble(std::string group, std::string key);
    int ReadInt(std::string group, std::string key);
    bool ReadBool(std::string group, std::string key);
    std::vector<double> ReadDoubleArray(std::string group, std::string key);
    YAML::Node config = YAML::LoadFile("config.yaml");
};