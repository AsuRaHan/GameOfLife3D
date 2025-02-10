#pragma once
#ifndef IRENDERERPROVIDER_H_
#define IRENDERERPROVIDER_H_

#include <vector>
#include "CellInstance.h" // Включаем определение CellInstance
#include "Camera.h"

class IRendererProvider {
public:
    virtual ~IRendererProvider() = default;
    virtual const std::vector<CellInstance> &GetCellInstances() const = 0;
    virtual void RebuildGameField() = 0;
    virtual Camera& GetCamera() = 0;
    virtual void OnWindowResize(int newWidth, int newHeight) = 0;
    virtual void Draw() = 0;
};
#endif // IRENDERERPROVIDER_H_