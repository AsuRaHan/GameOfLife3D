#pragma once
#ifndef CELL_H_
#define CELL_H_

#include "../mathematics/Vector3d.h"

class Cell {
private:
    bool isAlive;
    //int type;
    Vector3d color;

public:
    Cell(bool alive = false, int type = 0, Vector3d color = Vector3d(0.0f, 0.0f, 0.0f)) : isAlive(alive)
        //, type(type)
        , color(color)
    {}
    void setAlive(bool alive) { isAlive = alive; }
    bool getAlive() const { return isAlive; }

    //void setType(int type) { this->type = type; }
    //int getType() const { return type; }

    void setColor(const Vector3d& color) { 
        this->color = color; 
    }
    Vector3d getColor() const { return color; }
};
#endif // CELL_H_