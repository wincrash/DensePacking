#include "YamlAPI.h"
#include <iostream>
#include <vector>
#include <stdexcept>

double YamlAPI::ReadDouble(std::string group, std::string key)
{
    auto node = config[group][key];
    if (node && !node.IsNull())
    {
        try
        {
            std::cout << "📖 YAML: " << group << "." << key << " = \"" << node.as<std::string>() << "\"\n";
            return node.as<double>();
        }
        catch (const YAML::TypedBadConversion<double> &e)
        {
            std::cerr << "❌ YAML: " << group << "." << key
                      << " → Cannot convert '" << node.as<std::string>()
                      << "' to double!\n";
        }
    }
    // Fallback
    std::cerr << "❌ Error in YAML: " << group << "." << key << "\n";
    exit(1);
    return 0;
}
std::string YamlAPI::ReadString(std::string group, std::string key)
{
    auto node = config[group][key];
    if (node && !node.IsNull())
    {
        try
        {
            std::string value = node.as<std::string>();
            std::cout << "📖 YAML: " << group << "." << key << " = \"" << value << "\"\n";
            return value;
        }
        catch (const YAML::TypedBadConversion<std::string> &e)
        {
            std::cerr << "❌ YAML: " << group << "." << key
                      << " → Cannot convert to string!\n";
        }
    }
    // Fallback
    std::cerr << "❌ Error in YAML: " << group << "." << key << "\n";
    exit(1);
    return "";
}
int YamlAPI::ReadInt(std::string group, std::string key)
{
    auto node = config[group][key];
    if (node && !node.IsNull())
    {
        try
        {
            std::cout << "📖 YAML: " << group << "." << key << " = \"" << node.as<std::string>() << "\"\n";
            return node.as<int>();
        }
        catch (const YAML::TypedBadConversion<int> &e)
        {
            std::cerr << "❌ YAML: " << group << "." << key
                      << " → Cannot convert '" << node.as<std::string>()
                      << "' to int!\n";
        }
    }
    // Fallback
    std::cerr << "❌ Error in YAML: " << group << "." << key << "\n";
    exit(1);
    return 0;
}
bool YamlAPI::ReadBool(std::string group, std::string key)
{
    auto node = config[group][key];
    if (node && !node.IsNull())
    {
        try
        {
            std::cout << "📖 YAML: " << group << "." << key << " = \"" << node.as<std::string>() << "\"\n";
            return node.as<bool>();
        }
        catch (const YAML::TypedBadConversion<bool> &e)
        {
            std::cerr << "❌ YAML: " << group << "." << key
                      << " → Cannot convert '" << node.as<std::string>()
                      << "' to bool!\n";
        }
    }
    // Fallback
    std::cerr << "❌ Error in YAML: " << group << "." << key << "\n";
    exit(1);
    return false;
}

std::vector<double> YamlAPI::ReadDoubleArray(std::string group, std::string key)
{

    auto node = config[group][key];
    std::vector<double> result;
    if (node && !node.IsNull())
    {
        try
        {
            std::cout << "📖 YAML: " << group << "." << key << " = \"" << " = [ ";
            for (size_t i = 0; i < node.size(); ++i)
            {
                result.push_back(node[i].as<double>());
                if (i < (node.size() - 1))
                    std::cout << result[i] << ", ";
                else
                    std::cout << result[i] << "";
            }
            std::cout << " ]\n";

            return result;
        }
        catch (const YAML::Exception &e)
        {
            std::cerr << "❌ YAML: " << group << "." << key
                      << " → Cannot convert '" << node.as<std::string>()
                      << "' to double array!\n";
        }
    }
    // Fallback
    std::cerr << "❌ Error in YAML: " << group << "." << key << "\n";
    exit(1);
    return result;
}