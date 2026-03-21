#pragma once

#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace rwe
{
    // Simple command-line and config-file argument parser.
    // Supports --key value, --key=value, --flag (bool), and multi-value keys.
    // Config files use key=value format, one per line; # comments are ignored.
    // Command-line args take precedence over config file values.
    class OpaqueArgs
    {
    private:
        std::map<std::string, std::vector<std::string>> values;
        bool helpRequested{false};

    public:
        void parse(int argc, char* argv[])
        {
            for (int i = 1; i < argc; ++i)
            {
                std::string arg(argv[i]);
                if (arg.size() < 3 || arg[0] != '-' || arg[1] != '-')
                {
                    throw std::runtime_error("Unexpected argument: " + arg);
                }
                arg = arg.substr(2);

                if (arg == "help")
                {
                    helpRequested = true;
                    continue;
                }

                // --key=value form
                auto eqPos = arg.find('=');
                if (eqPos != std::string::npos)
                {
                    auto key = arg.substr(0, eqPos);
                    auto val = arg.substr(eqPos + 1);
                    values[key].push_back(val);
                    continue;
                }

                // --flag (no next arg or next arg starts with --)
                if (i + 1 >= argc || (argv[i + 1][0] == '-' && argv[i + 1][1] == '-'))
                {
                    values[arg].emplace_back("true");
                    continue;
                }

                // --key value form
                ++i;
                values[arg].push_back(argv[i]);
            }
        }

        void parseConfig(const std::string& filePath)
        {
            std::ifstream file(filePath);
            if (!file.is_open())
            {
                return;
            }
            parseConfig(file);
        }

        void parseConfig(std::istream& stream)
        {
            std::string line;
            while (std::getline(stream, line))
            {
                // trim leading whitespace
                auto start = line.find_first_not_of(" \t\r\n");
                if (start == std::string::npos)
                {
                    continue;
                }
                line = line.substr(start);

                // skip comments and empty lines
                if (line.empty() || line[0] == '#')
                {
                    continue;
                }

                auto eqPos = line.find('=');
                if (eqPos == std::string::npos)
                {
                    throw std::runtime_error("Invalid config line (no '='): " + line);
                }

                auto key = line.substr(0, eqPos);
                auto val = line.substr(eqPos + 1);

                // trim trailing whitespace from key
                auto keyEnd = key.find_last_not_of(" \t");
                if (keyEnd != std::string::npos)
                {
                    key = key.substr(0, keyEnd + 1);
                }

                // trim leading whitespace from value
                auto valStart = val.find_first_not_of(" \t");
                if (valStart != std::string::npos)
                {
                    val = val.substr(valStart);
                }
                else
                {
                    val = "";
                }

                // Only set from config if not already set by command line
                if (values.find(key) == values.end())
                {
                    values[key].push_back(val);
                }
            }
        }

        bool isHelpRequested() const
        {
            return helpRequested;
        }

        bool contains(const std::string& key) const
        {
            return values.find(key) != values.end();
        }

        std::string getString(const std::string& key, const std::string& defaultValue) const
        {
            auto it = values.find(key);
            if (it == values.end() || it->second.empty())
            {
                return defaultValue;
            }
            return it->second.back();
        }

        std::string getString(const std::string& key) const
        {
            auto it = values.find(key);
            if (it == values.end() || it->second.empty())
            {
                throw std::runtime_error("Missing required argument: --" + key);
            }
            return it->second.back();
        }

        unsigned int getUint(const std::string& key, unsigned int defaultValue) const
        {
            auto it = values.find(key);
            if (it == values.end() || it->second.empty())
            {
                return defaultValue;
            }
            return static_cast<unsigned int>(std::stoul(it->second.back()));
        }

        bool getBool(const std::string& key) const
        {
            auto it = values.find(key);
            if (it == values.end() || it->second.empty())
            {
                return false;
            }
            return it->second.back() == "true" || it->second.back() == "1";
        }

        std::vector<std::string> getMulti(const std::string& key) const
        {
            auto it = values.find(key);
            if (it == values.end())
            {
                return {};
            }
            return it->second;
        }
    };
}
