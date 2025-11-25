#include <iostream>
#include <string>
#include "logger.hpp"

void Logger::setLevel(Level level) {
    currentLevel = level;
}

void Logger::error(const std::string& message) const {
    if (currentLevel >= Level::ERRORS)
        std::cout << "[ERROR] " << message << std::endl;
}

void Logger::warning(const std::string& message) const {
    if (currentLevel >= Level::WARNING)
        std::cout << "[WARN]  " << message << std::endl;
}

void Logger::info(const std::string& message) const {
    if (currentLevel >= Level::INFO)
        std::cout << "[INFO]  " << message << std::endl;
}

void Logger::debug(const std::string& message) const {
    if (currentLevel >= Level::DEBUG)
        std::cout << "[DEBUG] " << message << std::endl;
}

// for logging -- could be expanded
