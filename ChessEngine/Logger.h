#pragma once
#include <iostream>
#include <string>

class Logger {
public:
    enum class Level {
        ERROR = 0,
        WARNING = 1,
        INFO = 2,
        DEBUG = 3
    };

    // Set the minimum log level (default: INFO)
    Logger(Level level = Level::INFO) : currentLevel(level) {}

    void setLevel(Level level);
    void error(const std::string& message) const;
    void warning(const std::string& message) const;
    void info(const std::string& message) const;
    void debug(const std::string& message) const;

private:
    Level currentLevel;
};

extern Logger logger;