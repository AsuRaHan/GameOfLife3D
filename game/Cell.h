#pragma once

class Cell {
private:
    bool isAlive;

public:
    Cell(bool alive = false);
    void setAlive(bool alive);
    bool getAlive() const;
};