export module Logging;
import std;

export namespace Logging {

    //All of these behave the same as std::print, IE:
    //auto myThing = 105;
    //Logging::info("You can {} print a thing.", myThing);
    //See <https://en.cppreference.com/w/cpp/io/print> for more details
    template <typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args);
    template <typename... Args>
    void warning(std::format_string<Args...> fmt, Args&&... args);
    template <typename... Args>
    void failure(std::format_string<Args...> fmt, Args&&... args);

    constexpr const char *logName = "last_run_log.txt";
    bool logIniitalized = false;
    constexpr bool loggingToFileEnabled = true;
    constexpr bool loggingToConsoleEnabled = true;
    enum class Severity { Info, Warning, Failure };
    constexpr Severity minimumLoggingSeverity = Severity::Info;
    
}

namespace Logging {
    constexpr const char *severityNameLookup[] = {"Info", "Warning", "Failure"};
    constexpr const char *severityColorLookup[] = {"\033[32m", "\033[33m", "\033[31m"}; //ANSI color codes
    constexpr bool meetsMinimumSeverity(Severity a, Severity b) { return static_cast<int>(a) >= static_cast<int>(b); }

    template <Severity S, typename... Args>
    void log(std::format_string<Args...> fmt, Args&&... args) {
        if constexpr (!meetsMinimumSeverity(S, minimumLoggingSeverity)) {
            return;
        }

        std::ofstream logFile;
        auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
        auto severityName = severityNameLookup[static_cast<int>(S)];
        auto severityColor = severityColorLookup[static_cast<int>(S)];
        constexpr auto resetColor = "\033[0m";
        std::string content = std::format(fmt, std::forward<Args>(args)...);

        if constexpr (loggingToConsoleEnabled) {
            std::print("[{:%Y-%m-%d %H:%M:%S}]\n {}[{}]{} {}\n", now, severityColor, severityName, resetColor, content);
        }

        if constexpr (!loggingToFileEnabled) {
            return;
        }

        if (!logIniitalized) {
            logIniitalized = true;
            logFile.open(logName, std::ios::out | std::ios::trunc);
            logFile.close();
        }

        logFile.open(logName, std::ios::out | std::ios::app);
        if (logFile.is_open()) {
            std::print(logFile, "[{:%Y-%m-%d %H:%M:%S}]\n [{}] {}\n", now, severityName, content);
            logFile.close();
        }
    }

    template <typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args) {
        log<Severity::Info>(fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void warning(std::format_string<Args...> fmt, Args&&... args) {
        log<Severity::Warning>(fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void failure(std::format_string<Args...> fmt, Args&&... args) {
        log<Severity::Failure>(fmt, std::forward<Args>(args)...);
    }
}