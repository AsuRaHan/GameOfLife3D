#pragma once
#include <vector>
#include "CellInstance.h" // Включаем определение CellInstance
#include "Camera.h"

class IRendererProvider {
public:
    virtual ~IRendererProvider() = default;
    virtual const std::vector<CellInstance> &GetCellInstances() const = 0;
    virtual void RebuildGameField() = 0;
    virtual Camera& GetCamera() = 0;
};