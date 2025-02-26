#ifndef PERFORMANCE_STATS_H
#define PERFORMANCE_STATS_H

#include <windows.h> // Для GetTickCount64
#include <string>
#include <sstream>

class PerformanceStats {
public:
    // Получение единственного экземпляра
    static PerformanceStats& getInstance() {
        static PerformanceStats instance; // Создается один раз и живет весь процесс
        return instance;
    }

    // Обновление статистики для конкретного события
    void recordFrame();              // Для подсчета FPS (вызов Draw)
    void recordSimulation();         // Для подсчета симуляций (вызов Update)

    // Получение текущих значений
    float getFPS() const { return smoothedFPS; } // Возвращаем сглаженное значение
    float getSimulationsPerSecond() const { return simulationsPerSecond; }

    float getMinSimulationDelayMs() const; // Минимальная задержка симуляции
    bool shouldUpdateSimulationSpeed() const; // Нужно ли обновить скорость симуляции
    void setTargetRefreshRate(float rate); // Установить частоту монитора

    // Получение строки со статистикой (для вывода)
    std::string getStatsString() const;

    // Сброс статистики (если нужно начать заново)
    void reset();
    // Внутренний метод для пересчета статистики
    void updateStats();
private:
    PerformanceStats(); // Приватный конструктор для Singleton

    // Запрещаем копирование и присваивание
    PerformanceStats(const PerformanceStats&) = delete;
    PerformanceStats& operator=(const PerformanceStats&) = delete;

    // Внутренние переменные
    LONGLONG lastUpdateTime;         // Время последнего обновления статистики (в мс)
    int frameCount;                  // Счетчик кадров
    int simulationCount;             // Счетчик симуляций
    float fps;                       // Текущий FPS
    float simulationsPerSecond;      // Текущие симуляции в секунду
    float updateInterval;            // Интервал обновления статистики (в мс)


    float targetFPS = 75.0f;         // Целевое значение FPS
    float avgFrameTimeMs = 0.0f;     // Среднее время кадра (мс)
    float avgSimulationTimeMs = 0.0f;// Среднее время симуляции (мс)

    float fpsHistory[5] = { 75.0f, 75.0f, 75.0f, 75.0f, 75.0f }; // История FPS (5 значений)
    int historyIndex = 0; // Индекс для записи
    float smoothedFPS = 75.0f; // Сглаженное значение FPS

};

#endif // PERFORMANCE_STATS_H