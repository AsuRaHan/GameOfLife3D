#include "Cell.h"

Cell::Cell(bool alive) : isAlive(alive) {}

void Cell::setAlive(bool alive) {
    isAlive = alive;
}

bool Cell::getAlive() const {
    return isAlive;
}