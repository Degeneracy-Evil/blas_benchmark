#include "config/config_parser.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include <toml.hpp>

namespace blas_benchmark::config
{

namespace
{

std::pair<int, int> parse_size_pair(const std::string& str)
{
    auto pos = str.find(',');
    if (pos == std::string::npos)
    {
        throw std::runtime_error("Invalid size format: " + str);
    }
    int m = std::stoi(str.substr(0, pos));
    int n = std::stoi(str.substr(pos + 1));
    return {m, n};
}

std::tuple<int, int, int> parse_size_triple(const std::string& str)
{
    auto pos1 = str.find(',');
    if (pos1 == std::string::npos)
    {
        throw std::runtime_error("Invalid size format: " + str);
    }
    auto pos2 = str.find(',', pos1 + 1);
    if (pos2 == std::string::npos)
    {
        throw std::runtime_error("Invalid size format: " + str);
    }
    int m = std::stoi(str.substr(0, pos1));
    int n = std::stoi(str.substr(pos1 + 1, pos2 - pos1 - 1));
    int k = std::stoi(str.substr(pos2 + 1));
    return {m, n, k};
}

} // anonymous namespace

BenchmarkConfig ConfigParser::parse_file(const std::string& path) const
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open config file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return parse_string(buffer.str());
}

BenchmarkConfig ConfigParser::parse_string(const std::string& content) const
{
    BenchmarkConfig config = get_default();

    try
    {
        auto tbl = toml::parse(content);

        // Parse functions section
        if (tbl.contains("functions"))
        {
            auto functions = tbl["functions"];

            if (functions.as_table()->contains("level1"))
            {
                config.level1_functions.clear();
                auto arr = functions["level1"].as_array();
                if (arr)
                {
                    for (const auto& item : *arr)
                    {
                        config.level1_functions.push_back(item.value_or(""));
                    }
                }
            }

            if (functions.as_table()->contains("level2"))
            {
                config.level2_functions.clear();
                auto arr = functions["level2"].as_array();
                if (arr)
                {
                    for (const auto& item : *arr)
                    {
                        config.level2_functions.push_back(item.value_or(""));
                    }
                }
            }

            if (functions.as_table()->contains("level3"))
            {
                config.level3_functions.clear();
                auto arr = functions["level3"].as_array();
                if (arr)
                {
                    for (const auto& item : *arr)
                    {
                        config.level3_functions.push_back(item.value_or(""));
                    }
                }
            }
        }

        // Parse weights section
        if (tbl.contains("weights"))
        {
            auto weights = tbl["weights"];

            // Level 1 weights
            if (weights.as_table()->contains("level1"))
            {
                config.level1_weights.clear();
                auto level1 = weights["level1"].as_table();
                if (level1)
                {
                    for (const auto& [key, value] : *level1)
                    {
                        config.level1_weights.emplace_back(key, value.value_or(1.0));
                    }
                }
            }

            // Level 2 weights
            if (weights.as_table()->contains("level2"))
            {
                config.level2_weights.clear();
                auto level2 = weights["level2"].as_table();
                if (level2)
                {
                    for (const auto& [key, value] : *level2)
                    {
                        config.level2_weights.emplace_back(key, value.value_or(1.0));
                    }
                }
            }

            // Level 3 weights
            if (weights.as_table()->contains("level3"))
            {
                config.level3_weights.clear();
                auto level3 = weights["level3"].as_table();
                if (level3)
                {
                    for (const auto& [key, value] : *level3)
                    {
                        config.level3_weights.emplace_back(key, value.value_or(1.0));
                    }
                }
            }
        }

        // Parse defaults section
        if (tbl.contains("defaults"))
        {
            auto defaults = tbl["defaults"];

            config.threads = defaults["threads"].value_or(config.threads);
            config.warmup = defaults["warmup"].value_or(config.warmup);
            config.cycles = defaults["cycles"].value_or(config.cycles);
            config.flush_cache = defaults["flush_cache"].value_or(config.flush_cache);

            if (defaults.as_table()->contains("level1_size"))
            {
                config.level1_size = defaults["level1_size"].value_or(0);
            }
            if (defaults.as_table()->contains("level2_m") && defaults.as_table()->contains("level2_n"))
            {
                config.level2_size = {
                    defaults["level2_m"].value_or(1024),
                    defaults["level2_n"].value_or(1024)
                };
            }
            if (defaults.as_table()->contains("level3_m") && defaults.as_table()->contains("level3_n") &&
                defaults.as_table()->contains("level3_k"))
            {
                config.level3_size = {
                    defaults["level3_m"].value_or(1024),
                    defaults["level3_n"].value_or(1024),
                    defaults["level3_k"].value_or(1024)
                };
            }
        }
    }
    catch (const toml::parse_error& e)
    {
        throw std::runtime_error(std::string("TOML parse error: ") + e.what());
    }

    return config;
}

} // namespace blas_benchmark::config
