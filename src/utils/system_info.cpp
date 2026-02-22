#include "utils/system_info.h"

#include <fstream>
#include <sstream>
#include <thread>
#include <unordered_map>

#ifdef __linux__
#include <sys/sysinfo.h>
#include <unistd.h>
#endif

namespace blas_benchmark::utils
{

namespace
{

// Helper function to read file contents
std::string read_file(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Helper function to trim whitespace and newlines from string
std::string trim(const std::string& str)
{
    auto start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
    {
        return "";
    }
    auto end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

// Helper function to parse CPU info from /proc/cpuinfo on Linux
std::unordered_map<std::string, std::string> parse_cpuinfo()
{
    std::unordered_map<std::string, std::string> info;
#ifdef __linux__
    std::ifstream file("/proc/cpuinfo");
    if (!file.is_open())
    {
        return info;
    }

    std::string line;
    while (std::getline(file, line))
    {
        auto pos = line.find(':');
        if (pos != std::string::npos)
        {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Trim whitespace
            while (!key.empty() && (key.back() == ' ' || key.back() == '\t'))
            {
                key.pop_back();
            }
            while (!value.empty() && (value.front() == ' ' || value.front() == '\t'))
            {
                value = value.substr(1);
            }

            info[key] = value;
        }
    }
#endif
    return info;
}

} // anonymous namespace

SystemInfo SystemInfoCollector::collect() const
{
    SystemInfo info;
    info.cpu_model = get_cpu_model();
    info.cpu_cores = get_cpu_cores();
    info.physical_cores = get_physical_cores();
    info.threads_per_core = get_threads_per_core();
    info.cpu_freq_mhz = get_cpu_freq_mhz();
    info.l1_cache = get_l1_cache();
    info.l2_cache = get_l2_cache();
    info.l3_cache = get_l3_cache();
    info.total_memory = get_total_memory();
    info.os_name = get_os_name();
    return info;
}

std::string SystemInfoCollector::get_cpu_model() const
{
#ifdef __linux__
    auto cpuinfo = parse_cpuinfo();
    auto it = cpuinfo.find("model name");
    if (it != cpuinfo.end())
    {
        return it->second;
    }
    // Fallback for ARM processors
    it = cpuinfo.find("Hardware");
    if (it != cpuinfo.end())
    {
        return it->second;
    }
#endif
    return "Unknown CPU";
}

int SystemInfoCollector::get_cpu_cores() const
{
    return static_cast<int>(std::thread::hardware_concurrency());
}

int SystemInfoCollector::get_physical_cores() const
{
#ifdef __linux__
    // Count unique physical cores from /proc/cpuinfo
    auto cpuinfo = parse_cpuinfo();
    int max_core_id = -1;

    std::ifstream file("/proc/cpuinfo");
    if (!file.is_open())
    {
        return get_cpu_cores(); // Fallback to logical cores
    }

    std::string line;
    int current_core_id = -1;

    while (std::getline(file, line))
    {
        if (line.find("core id") != std::string::npos)
        {
            auto pos = line.find(':');
            if (pos != std::string::npos)
            {
                current_core_id = std::stoi(line.substr(pos + 1));
                max_core_id = std::max(max_core_id, current_core_id);
            }
        }
    }

    if (max_core_id >= 0)
    {
        return max_core_id + 1;
    }
#endif
    return get_cpu_cores(); // Fallback to logical cores
}

int SystemInfoCollector::get_threads_per_core() const
{
    int logical = get_cpu_cores();
    int physical = get_physical_cores();
    if (physical > 0)
    {
        return logical / physical;
    }
    return 1;
}

double SystemInfoCollector::get_cpu_freq_mhz() const
{
#ifdef __linux__
    // Try to read from /proc/cpuinfo
    auto cpuinfo = parse_cpuinfo();
    auto it = cpuinfo.find("cpu MHz");
    if (it != cpuinfo.end())
    {
        try
        {
            return std::stod(it->second);
        }
        catch (...)
        {
        }
    }

    // Try to read max frequency from sysfs
    std::string freq_str = read_file("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    if (!freq_str.empty())
    {
        try
        {
            // Frequency is in kHz
            return std::stod(freq_str) / 1000.0;
        }
        catch (...)
        {
        }
    }
#endif
    return 0.0;
}

std::size_t SystemInfoCollector::get_l1_cache() const
{
#ifdef __linux__
    // Read L1 data cache size from sysfs
    std::string cache_size = trim(read_file("/sys/devices/system/cpu/cpu0/cache/index0/size"));
    if (cache_size.empty())
    {
        cache_size = trim(read_file("/sys/devices/system/cpu/cpu0/cache/index1/size"));
    }

    if (!cache_size.empty())
    {
        // Parse size string (e.g., "32K", "64K")
        std::size_t multiplier = 1;
        if (cache_size.back() == 'K')
        {
            multiplier = 1024;
        }
        else if (cache_size.back() == 'M')
        {
            multiplier = 1024 * 1024;
        }

        try
        {
            return std::stoul(cache_size) * multiplier;
        }
        catch (...)
        {
        }
    }
#endif
    // Default L1 cache size (32KB is common)
    return 32 * 1024;
}

std::size_t SystemInfoCollector::get_l2_cache() const
{
#ifdef __linux__
    // Read L2 cache size from sysfs
    for (int i = 0; i < 8; ++i)
    {
        std::string level_str = trim(read_file("/sys/devices/system/cpu/cpu0/cache/index" + std::to_string(i) + "/level"));

        if (level_str == "2")
        {
            std::string cache_size = trim(read_file("/sys/devices/system/cpu/cpu0/cache/index" + std::to_string(i) + "/size"));
            if (!cache_size.empty())
            {
                std::size_t multiplier = 1;
                if (cache_size.back() == 'K')
                {
                    multiplier = 1024;
                }
                else if (cache_size.back() == 'M')
                {
                    multiplier = 1024 * 1024;
                }

                try
                {
                    return std::stoul(cache_size) * multiplier;
                }
                catch (...)
                {
                }
            }
        }
    }
#endif
    // Default L2 cache size (256KB is common)
    return 256 * 1024;
}

std::size_t SystemInfoCollector::get_l3_cache() const
{
#ifdef __linux__
    // Read L3 cache size from sysfs
    for (int i = 0; i < 8; ++i)
    {
        std::string level_str = trim(read_file("/sys/devices/system/cpu/cpu0/cache/index" + std::to_string(i) + "/level"));

        if (level_str == "3")
        {
            std::string cache_size = trim(read_file("/sys/devices/system/cpu/cpu0/cache/index" + std::to_string(i) + "/size"));
            if (!cache_size.empty())
            {
                std::size_t multiplier = 1;
                if (cache_size.back() == 'K')
                {
                    multiplier = 1024;
                }
                else if (cache_size.back() == 'M')
                {
                    multiplier = 1024 * 1024;
                }

                try
                {
                    return std::stoul(cache_size) * multiplier;
                }
                catch (...)
                {
                }
            }
        }
    }
#endif
    // Default L3 cache size (8MB is common)
    return 8 * 1024 * 1024;
}

std::size_t SystemInfoCollector::get_total_memory() const
{
#ifdef __linux__
    struct sysinfo info;
    if (sysinfo(&info) == 0)
    {
        return info.totalram * info.mem_unit;
    }
#endif
    // Default 16GB if unable to detect
    return 16ULL * 1024 * 1024 * 1024;
}

std::string SystemInfoCollector::get_os_name() const
{
#ifdef __linux__
    std::string os_release = read_file("/etc/os-release");
    if (!os_release.empty())
    {
        // Extract PRETTY_NAME
        auto pos = os_release.find("PRETTY_NAME=");
        if (pos != std::string::npos)
        {
            auto start = os_release.find('"', pos);
            auto end = os_release.find('"', start + 1);
            if (start != std::string::npos && end != std::string::npos)
            {
                return os_release.substr(start + 1, end - start - 1);
            }
        }
    }
    return "Linux";
#else
    return "Unknown OS";
#endif
}

} // namespace blas_benchmark::utils
