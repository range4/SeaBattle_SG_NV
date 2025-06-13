#include "Ship.h"

Ship::Ship(int size, int x, int y, Direction dir)
    : size(size), x(x), y(y), direction(dir) {
}

int Ship::getSize() const { return size; }
int Ship::getX() const { return x; }
int Ship::getY() const { return y; }
Ship::Direction Ship::getDirection() const { return direction; }

void Ship::setPosition(int x, int y) {
    this->x = x;
    this->y = y;
}

void Ship::setDirection(Direction dir) {
    direction = dir;
}

std::vector<Cell> Ship::getCells() const {
    std::vector<Cell> cells;
    if (direction == HORIZONTAL) {
        for (int i = 0; i < size; i++) {
            cells.push_back({ x + i, y });
        }
    }
    else {
        for (int i = 0; i < size; i++) {
            cells.push_back({ x, y + i });
        }
    }
    return cells;
}