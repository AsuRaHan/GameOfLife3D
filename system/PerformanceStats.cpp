#include "PerformanceStats.h"


PerformanceStats::PerformanceStats()
    : lastUpdateTime(GetTickCount64()),
      frameCount(0),
      simulationCount(0),
      fps(0.0f),
      simulationsPerSecond(0.0f),
      updateInterval(1000.0f) // 1 секунда по умолчанию
    , targetFPS(75.0f)
    , avgFrameTimeMs(0.0f)
    , avgSimulationTimeMs(0.0f)
{
}

void PerformanceStats::recordFrame() {
    frameCount++;
    //updateStats();
}

void PerformanceStats::recordSimulation() {
    simulationCount++;
    //updateStats();
}

void PerformanceStats::updateStats() {
    LONGLONG currentTime = GetTickCount64();
    LONGLONG elapsedTime = currentTime - lastUpdateTime;

    if (elapsedTime >= updateInterval) {
        fps = static_cast<float>(frameCount) * 1000.0f / elapsedTime;
        simulationsPerSecond = static_cast<float>(simulationCount) * 1000.0f / elapsedTime;

        // Считаем среднее время на кадр и симуляцию
        avgFrameTimeMs = (frameCount > 0) ? (1000.0f / fps) : 0.0f;
        avgSimulationTimeMs = (simulationCount > 0) ? (1000.0f / simulationsPerSecond) : 0.0f;

        // Обновляем историю FPS
        fpsHistory[historyIndex] = fps;
        historyIndex = (historyIndex + 1) % 5; // Циклический индекс

        // Считаем сглаженное FPS
        smoothedFPS = 0.0f;
        for (int i = 0; i < 5; i++) {
            smoothedFPS += fpsHistory[i];
        }
        smoothedFPS /= 5.0f;

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

float PerformanceStats::getMinSimulationDelayMs() const {
    // Целевое время кадра для targetFPS (например, 13.33 мс для 75 FPS)
    float targetFrameTimeMs = 1000.0f / targetFPS;

    // Сколько времени остаётся на симуляцию после рендеринга
    float availableTimeMs = targetFrameTimeMs - (avgFrameTimeMs - avgSimulationTimeMs);

    // Если времени не хватает, увеличиваем задержку симуляции
    if (availableTimeMs <= 0) {
        return avgSimulationTimeMs + 1.0f; // Добавляем 1 мс для стабильности
    }

    // Если время есть, возвращаем минимальную задержку (1 мс или время симуляции)
    float minDelay = avgSimulationTimeMs;
    if (minDelay < 1.0f) {
        minDelay = 1.0f;
    }
    return minDelay;
}

bool PerformanceStats::shouldUpdateSimulationSpeed() const {
    // Целевое время кадра (например, 13.33 мс для 75 FPS)
    float targetFrameTimeMs = 1000.0f / targetFPS;

    // Если текущее время кадра сильно отличается от целевого
    float frameTimeDiff = avgFrameTimeMs - targetFrameTimeMs;

    // Проверяем, требуется ли корректировка (например, отклонение больше 10%)
    return frameTimeDiff > (targetFrameTimeMs * 0.1f) || frameTimeDiff < -(targetFrameTimeMs * 0.3f);
}

void PerformanceStats::setTargetRefreshRate(float rate) {
    if (rate > 0.0f) { // Проверка на корректность
        targetFPS = rate;
    }
}