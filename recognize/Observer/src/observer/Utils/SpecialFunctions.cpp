#include "SpecialFunctions.hpp"

#include <cstring>

#ifdef _WIN32
#include <Shlwapi.h>
#include <io.h>
#include <windows.h>

#define access _access_s
#elif defined(__linux__)
#include <libgen.h>
#include <limits.h>
#include <unistd.h>

#if defined(__sun)
#define PROC_SELF_EXE "/proc/self/path/a.out"
#else
#define PROC_SELF_EXE "/proc/self/exe"
#endif

#endif

namespace Observer::SpecialFunctions {
    // output: 29_01_1900_23_41_13
    std::string GetCurrentTime(const std::string& format) {
        std::chrono::time_point<std::chrono::system_clock> timepoint =
            std::chrono::system_clock::now();

        std::time_t time_now_t =
            std::chrono::system_clock::to_time_t(timepoint);
        return std::move(TimeToString(format, time_now_t));
    }

    std::string TimeToString(const std::string& format,
                             const std::time_t& time_now_t) {
        std::tm* now_tm = localtime(&time_now_t);

        std::ostringstream ss;
        ss << std::put_time(now_tm, format.c_str());
        return ss.str();
    }

    std::string FormatNotificationTextString(std::string str,
                                             const std::string& name) {
        size_t pos = str.find("{N}");

        if (pos != std::string::npos) {
            return str.replace(pos, 3, name);
        }

        return str;
    }

    /**
     * Returns a JSON string with all the pairs given
     * e.g.
     *  > func({{"url", "test"}, {"path", "c:/here"}})
     *  < {"url": "test","path": "c:/here"}
     * @param pairs all the key-value pairs to generate
     * @return
     */
    std::string JsonStringGenerator(
        const std::vector<std::pair<std::string, std::string>>& pairs) {
        std::string res = "{";

        for (auto&& i : pairs) {
            res += "\"" + i.first + "\": \"" + i.second + "\",";
        }

        res.pop_back();  // pop last ,
        res += "}";

        return res;
    }

    namespace Paths {
#ifdef _WIN32
        std::filesystem::path GetExecutablePath() {
            char rawPathName[MAX_PATH];
            GetModuleFileNameA(NULL, rawPathName, MAX_PATH);
            return std::filesystem::path(rawPathName);
        }

        std::filesystem::path GetExecutableDirectory() {
            std::string executablePath = getExecutablePath();
            char* exePath = new char[executablePath.length()];
            strcpy_s(exePath, executablePath.length() + 1,
                     executablePath.c_str());
            PathRemoveFileSpecA(exePath);
            std::string directory = std::string(exePath);
            delete[] exePath;
            return directory;
        }

#elif defined(__linux__)
        std::filesystem::path GetExecutablePath() {
            char rawPathName[PATH_MAX];
            realpath(PROC_SELF_EXE, rawPathName);
            return std::filesystem::path(rawPathName);
        }

        std::filesystem::path GetExecutableDirectory() {
            std::string executablePath = GetExecutablePath();
            char* executablePathStr = new char[executablePath.length() + 1];
            strcpy(executablePathStr, executablePath.c_str());
            char* executableDir = dirname(executablePathStr);
            auto path = std::filesystem::path(executableDir);
            delete[] executablePathStr;
            return path;
        }
#endif
    }  // namespace Paths
}  // namespace Observer::SpecialFunctions
