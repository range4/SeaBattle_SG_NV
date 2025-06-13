#pragma once
#ifndef SHIP_H
#define SHIP_H

#include <vector>

struct Cell {
    int x;
    int y;
};

class Ship {
public:
    enum Direction { HORIZONTAL, VERTICAL };

    Ship(int size, int x, int y, Direction dir);

    int getSize() const;
    int getX() const;
    int getY() const;
    Direction getDirection() const;

    void setPosition(int x, int y);
    void setDirection(Direction dir);

    std::vector<Cell> getCells() const;

private:
    int size;
    int x;
    int y;
    Direction direction;
};

#endif // SHIP_H