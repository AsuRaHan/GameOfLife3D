#pragma once
#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <string>
#include <sstream>
#include <optional>
#include <iostream>
#include <stdexcept>

namespace constants {
    //Default values
    extern const int DEFAULT_GRID_WIDTH;
    extern const int DEFAULT_GRID_HEIGHT;
    extern const int DEFAULT_SCREEN_WIDTH;
    extern const int DEFAULT_SCREEN_HEIGHT;
}

class CommandLineParser {
public:
    struct Options {
        int gridWidth = constants::DEFAULT_GRID_WIDTH;
        int gridHeight = constants::DEFAULT_GRID_HEIGHT;
        int screenWidth = constants::DEFAULT_SCREEN_WIDTH;
        int screenHeight = constants::DEFAULT_SCREEN_HEIGHT;
        bool fullscreen = false;
    };

    CommandLineParser(const std::wstring& cmdLine);
    Options parse();

private:
    int parseIntOption(std::wistringstream& iss, const std::string& optionName, int defaultValue);
    void parseScreenResolution(std::wistringstream& iss, Options& options);
    std::wstring cmdLine_;
};

#endif // COMMANDLINEPARSER_H
