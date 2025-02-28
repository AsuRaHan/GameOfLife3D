// CommandLineParser.cpp

#include "CommandLineParser.h"


namespace constants {
    const int DEFAULT_GRID_WIDTH = 400;
    const int DEFAULT_GRID_HEIGHT = 300;
    const int DEFAULT_SCREEN_WIDTH = 1024;
    const int DEFAULT_SCREEN_HEIGHT = 768;
}

CommandLineParser::CommandLineParser(const std::wstring& cmdLine) : cmdLine_(cmdLine) {}

CommandLineParser::Options CommandLineParser::parse() {
    Options options;
    std::wistringstream iss(cmdLine_);
    std::wstring token;

    while (iss >> token) {
        if (token == L"-gridWidth") {
            options.gridWidth = parseIntOption(iss, "gridWidth", constants::DEFAULT_GRID_WIDTH);
        } else if (token == L"-gridHeight") {
            options.gridHeight = parseIntOption(iss, "gridHeight", constants::DEFAULT_GRID_HEIGHT);
        } else if (token == L"-fullscreen") {
            options.fullscreen = true;
        } else if (token == L"-screenResolution") {
            parseScreenResolution(iss, options);
        }
    }
    return options;
}

int CommandLineParser::parseIntOption(std::wistringstream& iss, const std::string& optionName, int defaultValue) {
    std::wstring token;
    if (iss >> token) {
        try {
            int value = std::stoi(token);
            if (value <= 0) {
                throw std::invalid_argument(optionName + " must be positive.");
            }
            return value;
        } catch (const std::exception& e) {
            std::wcerr << L"Invalid " << optionName.c_str() << L" value: " << token << L". Using default value " << defaultValue << L"." << std::endl;
        }
    }
    return defaultValue;
}

void CommandLineParser::parseScreenResolution(std::wistringstream& iss, Options& options) {
    std::wstring token;
    if (iss >> token) {
        size_t xPos = token.find(L'x');
        if (xPos != std::wstring::npos) {
            try {
                options.screenWidth = std::stoi(token.substr(0, xPos));
                options.screenHeight = std::stoi(token.substr(xPos + 1));
                if (options.screenWidth <= 0 || options.screenHeight <= 0) {
                    throw std::invalid_argument("Width and height must be positive.");
                }
            } catch (const std::exception& e) {
                std::wcerr << L"Invalid screen resolution value: " << token << L". Using default resolution." << std::endl;
                options.screenWidth = constants::DEFAULT_SCREEN_WIDTH;
                options.screenHeight = constants::DEFAULT_SCREEN_HEIGHT;
            }
        } else {
            std::wcerr << L"Invalid screen resolution format: " << token << L". Using default resolution." << std::endl;
            options.screenWidth = constants::DEFAULT_SCREEN_WIDTH;
            options.screenHeight = constants::DEFAULT_SCREEN_HEIGHT;
        }
    }
}
