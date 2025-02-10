#pragma once
#ifndef CELLINSTANCE_H_
#define CELLINSTANCE_H_

#include "../mathematics/Vector3d.h"

struct CellInstance {
    float x, y; // Позиция клетки
    Vector3d color; // Цвет клетки
};
#endif // CELLINSTANCE_H_