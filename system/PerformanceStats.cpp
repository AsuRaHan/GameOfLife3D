#include "PerformanceStats.h"


PerformanceStats::PerformanceStats()
    : lastUpdateTime(GetTickCount64()),
      frameCount(0),
      simulationCount(0),
      fps(0.0f),
      simulationsPerSecond(0.0f),
      updateInterval(1000.0f) // 1 секунда по умолчанию
{
}

void PerformanceStats::recordFrame() {
    frameCount++;
    updateStats();
}

void PerformanceStats::recordSimulation() {
    simulationCount++;
    updateStats();
}

void PerformanceStats::updateStats() {
    LONGLONG currentTime = GetTickCount64();
    LONGLONG elapsedTime = currentTime - lastUpdateTime;

    if (elapsedTime >= updateInterval) {
        // Вычисляем FPS и симуляции в секунду
        fps = static_cast<float>(frameCount) * 1000.0f / elapsedTime;
        simulationsPerSecond = static_cast<float>(simulationCount) * 1000.0f / elapsedTime;

        // Сбрасываем счетчики и время
        frameCount = 0;
        simulationCount = 0;
        lastUpdateTime = currentTime;
    }
}

std::string PerformanceStats::getStatsString() const {
    std::ostringstream oss;
    oss << "FPS: " << static_cast<int>(fps + 0.5f) // Округляем для читаемости
        << " | Simulations/s: " << static_cast<int>(simulationsPerSecond + 0.5f);
    return oss.str();
}

void PerformanceStats::reset() {
    lastUpdateTime = GetTickCount64();
    frameCount = 0;
    simulationCount = 0;
    fps = 0.0f;
    simulationsPerSecond = 0.0f;
}