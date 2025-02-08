#pragma once
#include <vector>
//#include "Renderer.h"
#include "CellInstance.h" // Включаем определение CellInstance

class ICellInstanceProvider {
public:
    virtual ~ICellInstanceProvider() = default;
    virtual const std::vector<CellInstance> &GetCellInstances() const = 0;
};