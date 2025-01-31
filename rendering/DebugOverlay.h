#pragma once

#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>

class DebugOverlay {
private:
    bool isSimulationRunning;
    float windowWidth;
    float windowHeight;

public:
    DebugOverlay();
    void setSimulationRunning(bool running);
    void draw() const;
    void onWindowResize(float width, float height); // Новый метод для обновления размеров
};