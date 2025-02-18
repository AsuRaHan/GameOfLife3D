#include "GameStateManager.h"


bool GameStateManager::validateDimensions(const Grid& grid, int width, int height) {
    if (width <= 0 || height <= 0) {
        std::cerr << "������������ ������� � �����" << std::endl;
        return false;
    }
    if (width != grid.getWidth() || height != grid.getHeight()) {
        std::cerr << "������� ����� � ����� �� ��������� � �������� ���������" << std::endl;
        return false;
    }
    return true;
} 

bool GameStateManager::saveGameState(const Grid& grid, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "�� ������� ������� ���� ��� ������: " << filename << std::endl;
        return false;
    }

    // ��������� ��������� �������
    //file << "x = " << grid.getWidth() << ", y = " << grid.getHeight() << ", rule = B3/S23\n"; // ��� �� �����������, �� ����� ���� �������

    // ��������� �����������
    file << "!Name: Saved world to Pattern\n";
    file << "!Description: Just an example\n";

    // ���������� ��������
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            file << (grid.getCellState(x, y) ? 'O' : '.');
        }
        file << '\n';
    }

    return true;
}

bool GameStateManager::loadGameState(Grid& grid, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "�� ������� ������� ���� ��� ������: " << filename << std::endl;
        return false;
    }

    std::string line;
    std::vector<std::string> patternLines;

    // ���������� ����������� � ���������, ���� ��� ����
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '!') continue; // ���������� �����������
        if (line.find("x =") == 0) continue; // ���������� ������ � ���������, ���� ��� ����

        patternLines.push_back(line);
    }

    if (patternLines.empty()) {
        std::cerr << "���� �� �������� ������ ��� ��������." << std::endl;
        return false;
    }

    int height = static_cast<int>(patternLines.size());
    int width = static_cast<int>(patternLines[0].length());

    // ��������, ��� ������� ����� ���������
    if (width != grid.getWidth() || height != grid.getHeight()) {
        std::cerr << "������� �������� �� ��������� � �������� ��������� �����." << std::endl;
        return false;
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (x < static_cast<int>(patternLines[y].length())) {
                grid.setCellState(x, y, patternLines[y][x] == 'O');
                if (patternLines[y][x] == 'O') {
                    grid.setCellColor(x, y, 0.0f, 0.6f, 0.0f); // ������� ��� ����� ������
                }
                else {
                    grid.setCellColor(x, y, 0.0f, 0.0f, 0.0f); // ������ ��� ������� ������
                }
            }
        }
    }

    return true;
}

//bool GameStateManager::saveBinaryGameState(const Grid& grid, const std::string& filename) {
//    std::ofstream file(filename, std::ios::binary);
//    if (!file.is_open()) {
//        std::cerr << "�� ������� ������� ���� ��� �������� ������: " << filename << std::endl;
//        return false;
//    }
//
//    auto* automaton = grid.GetGPUAutomaton();
//    int width = grid.getWidth();
//    int height = grid.getHeight();
//
//    // ���������� ���������
//    file.write(GAME_FILE_MAGIC_NUMBER, strlen(GAME_FILE_MAGIC_NUMBER));
//
//    // ���������� ������ � ������
//    file.write(reinterpret_cast<const char*>(&width), sizeof(int));
//    file.write(reinterpret_cast<const char*>(&height), sizeof(int));
//
//    // ��������� ��������� ������
//    std::vector<int> states;
//    automaton->GetGridState(states);
//    file.write(reinterpret_cast<const char*>(states.data()), sizeof(int) * states.size());
//
//    // ��������� �����
//    std::vector<float> colors;
//    automaton->GetColorsBuf(colors);
//    file.write(reinterpret_cast<const char*>(colors.data()), sizeof(float) * colors.size());
//
//    return true;
//}

//bool GameStateManager::loadBinaryGameState(Grid& grid, const std::string& filename) {
//    std::ifstream file(filename, std::ios::binary);
//    if (!file.is_open()) {
//        std::cerr << "�� ������� ������� ���� ��� ��������� ������: " << filename << std::endl;
//        return false;
//    }
//
//    // ��������� ���������
//    char magic[5] = { 0 }; // +1 ��� �������� �������
//    file.read(magic, strlen(GAME_FILE_MAGIC_NUMBER));
//    if (std::memcmp(magic, GAME_FILE_MAGIC_NUMBER, strlen(GAME_FILE_MAGIC_NUMBER)) != 0) {
//        std::cerr << "���� ����� �������� ������ ��� ���������." << std::endl;
//        return false;
//    }
//
//    int width, height;
//    file.read(reinterpret_cast<char*>(&width), sizeof(int));
//    file.read(reinterpret_cast<char*>(&height), sizeof(int));
//
//    // �������� ������������ ��������
//    auto* automaton = grid.GetGPUAutomaton();
//    if (width != grid.getWidth() || height != grid.getHeight()) {
//        std::cerr << "������� � ����������� ����� �� ��������� � �������� ��������� �����." << std::endl;
//        return false;
//    }
//
//    // ������ ��������� ������
//    std::vector<int> states(width * height);
//    file.read(reinterpret_cast<char*>(states.data()), sizeof(int) * states.size());
//    automaton->SetGridState(states);
//
//    // ������ ������
//    std::vector<float> colors(width * height * 4);
//    file.read(reinterpret_cast<char*>(colors.data()), sizeof(float) * colors.size());
//    automaton->SetColorsBuf(colors);
//
//    // ���������� true, �����������, ��� ����� ��� ������������� � ��������� ������ ������ GPUAutomaton
//    return true;
//}

bool GameStateManager::CompressData(const BYTE* uncompressedData, SIZE_T uncompressedSize, BYTE** compressedData, SIZE_T* compressedSize) {
    COMPRESSOR_HANDLE compressor;
    if (!CreateCompressor(COMPRESS_ALGORITHM_XPRESS, NULL, &compressor)) {
        // ��������� ������
        std::cerr << "Failed to create compressor: " << GetLastError() << std::endl;
        return false;
    }

    SIZE_T estimatedSize = 0;
    if (!Compress(compressor, uncompressedData, uncompressedSize, NULL, 0, &estimatedSize)) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            *compressedData = (BYTE*)malloc(estimatedSize);
            if (*compressedData == NULL) {
                // ��������� ������ ��������� ������
                std::cerr << "Memory allocation failed for compression buffer" << std::endl;
                CloseCompressor(compressor);
                return false;
            }
            if (!Compress(compressor, uncompressedData, uncompressedSize, *compressedData, estimatedSize, (SIZE_T*)compressedSize)) {
                // ��������� ������
                std::cerr << "Compression failed: " << GetLastError() << std::endl;
                free(*compressedData);
                *compressedData = NULL;
                CloseCompressor(compressor);
                return false;
            }
        }
        else {
            // ��������� ������ ������ ����������
            std::cerr << "Unexpected error during compression size estimation: " << GetLastError() << std::endl;
            CloseCompressor(compressor);
            return false;
        }
    }

    CloseCompressor(compressor);
    return true;
}

bool GameStateManager::DecompressData(const BYTE* compressedData, SIZE_T compressedSize, BYTE** decompressedData, SIZE_T* decompressedSize) {
    DECOMPRESSOR_HANDLE decompressor;
    if (!CreateDecompressor(COMPRESS_ALGORITHM_XPRESS, NULL, &decompressor)) {
        // ��������� ������
        std::cerr << "Failed to create decompressor: " << GetLastError() << std::endl;
        return false;
    }

    SIZE_T neededSize = 0;
    if (!Decompress(decompressor, compressedData, compressedSize, NULL, 0, &neededSize)) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            *decompressedData = (BYTE*)malloc(neededSize);
            if (*decompressedData == NULL) {
                // ��������� ������ ��������� ������
                std::cerr << "Memory allocation failed for decompression buffer" << std::endl;
                CloseDecompressor(decompressor);
                return false;
            }
            if (!Decompress(decompressor, compressedData, compressedSize, *decompressedData, neededSize, (SIZE_T*)decompressedSize)) {
                // ��������� ������
                std::cerr << "Decompression failed: " << GetLastError() << std::endl;
                free(*decompressedData);
                *decompressedData = NULL;
                CloseDecompressor(decompressor);
                return false;
            }
        }
        else {
            // ��������� ������ ������ ������������
            std::cerr << "Unexpected error during decompression size estimation: " << GetLastError() << std::endl;
            CloseDecompressor(decompressor);
            return false;
        }
    }

    CloseDecompressor(decompressor);
    return true;
}


bool GameStateManager::saveBinaryGameState(const Grid& grid, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "�� ������� ������� ���� ��� �������� ������: " << filename << std::endl;
        return false;
    }

    auto* automaton = grid.GetGPUAutomaton();
    int width = grid.getWidth();
    int height = grid.getHeight();

    // ��������� ���������
    file.write(GAME_FILE_MAGIC_NUMBER, strlen(GAME_FILE_MAGIC_NUMBER));

    // ���������� ������ � ������
    file.write(reinterpret_cast<const char*>(&width), sizeof(int));
    file.write(reinterpret_cast<const char*>(&height), sizeof(int));

    // ������ ��� ��������� ������
    std::vector<int> states;
    automaton->GetGridState(states);
    BYTE* compressedStates = NULL;
    SIZE_T compressedStatesSize = 0;
    if (CompressData(reinterpret_cast<BYTE*>(states.data()), states.size() * sizeof(int), &compressedStates, &compressedStatesSize)) {
        SectionHeader statesHeader = { {'S', 'T', 'A', 'T'}, compressedStatesSize };
        file.write(reinterpret_cast<const char*>(&statesHeader), sizeof(SectionHeader));
        file.write(reinterpret_cast<const char*>(compressedStates), compressedStatesSize);
        free(compressedStates);
    }
    else {
        std::cerr << "������ ��� ������ ������ ��������� ������!" << std::endl;
        return false;
    }

    // ������ ��� ������
    std::vector<float> colors;
    automaton->GetColorsBuf(colors);
    BYTE* compressedColors = NULL;
    SIZE_T compressedColorsSize = 0;
    if (CompressData(reinterpret_cast<BYTE*>(colors.data()), colors.size() * sizeof(float), &compressedColors, &compressedColorsSize)) {
        SectionHeader colorsHeader = { {'C', 'O', 'L', 'R'}, compressedColorsSize };
        file.write(reinterpret_cast<const char*>(&colorsHeader), sizeof(SectionHeader));
        file.write(reinterpret_cast<const char*>(compressedColors), compressedColorsSize);
        free(compressedColors);
    }
    else {
        std::cerr << "������ ��� ������ ������ ������!" << std::endl;
        return false;
    }

    return true;
}

bool GameStateManager::loadBinaryGameState(Grid& grid, const std::string& filename, int & loadStatus) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "�� ������� ������� ���� ��� ��������� ������: " << filename << std::endl;
        return false;
    }

    // �������� ���������
    char magic[5] = { 0 };
    file.read(magic, strlen(GAME_FILE_MAGIC_NUMBER));
    if (std::memcmp(magic, GAME_FILE_MAGIC_NUMBER, strlen(GAME_FILE_MAGIC_NUMBER)) != 0) {
        std::cerr << "���� ����� �������� ������ ��� ���������." << std::endl;
        return false;
    }

    int width, height;
    file.read(reinterpret_cast<char*>(&width), sizeof(int));
    file.read(reinterpret_cast<char*>(&height), sizeof(int));

    // �������� ������������ ��������
    auto* automaton = grid.GetGPUAutomaton();
    if (width != grid.getWidth() || height != grid.getHeight()) {
        std::cerr << "������� � ����������� ����� �� ��������� � �������� ��������� �����. ��� ����� ���������� � ������ ���������. width=" << width << " height=" << height << std::endl;
        grid.setSize(width, height);
        automaton->SetNewGridSize(width, height);
        loadStatus = 2;
    }

    while (file) {
        SectionHeader header;
        if (!file.read(reinterpret_cast<char*>(&header), sizeof(SectionHeader))) break;

        std::vector<BYTE> compressedData(header.size);
        file.read(reinterpret_cast<char*>(compressedData.data()), header.size);

        if (strncmp(header.type, "STAT", 4) == 0) {
            BYTE* decompressedStates = NULL;
            SIZE_T decompressedStatesSize = width * height * sizeof(int);
            if (DecompressData(compressedData.data(), compressedData.size(), &decompressedStates, &decompressedStatesSize)) {
                std::vector<int> states(reinterpret_cast<int*>(decompressedStates), reinterpret_cast<int*>(decompressedStates) + decompressedStatesSize / sizeof(int));
                automaton->SetGridState(states);
                free(decompressedStates);
            }
            else {
                std::cerr << "������ ��� ���������� ������ ��������� ������!" << std::endl;
                loadStatus = 0;
                return false;
            }
        }
        else if (strncmp(header.type, "COLR", 4) == 0) {
            BYTE* decompressedColors = NULL;
            SIZE_T decompressedColorsSize = width * height * 4 * sizeof(float);
            if (DecompressData(compressedData.data(), compressedData.size(), &decompressedColors, &decompressedColorsSize)) {
                std::vector<float> colors(reinterpret_cast<float*>(decompressedColors), reinterpret_cast<float*>(decompressedColors) + decompressedColorsSize / sizeof(float));
                automaton->SetColorsBuf(colors);
                free(decompressedColors);
            }
            else {
                std::cerr << "������ ��� ���������� ������ ������!" << std::endl;
                loadStatus = 0;
                return false;
            }
        }
    }
    return true;
}