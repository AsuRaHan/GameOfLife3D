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
    float getFPS() const { return fps; }
    float getSimulationsPerSecond() const { return simulationsPerSecond; }
    
    // Получение строки со статистикой (для вывода)
    std::string getStatsString() const;

    // Сброс статистики (если нужно начать заново)
    void reset();

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

    // Внутренний метод для пересчета статистики
    void updateStats();
};

#endif // PERFORMANCE_STATS_H