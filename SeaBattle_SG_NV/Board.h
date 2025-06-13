#pragma once
#ifndef BOARD_H
#define BOARD_H

#include "Ship.h"
#include <vector>
#include <memory>

class Board {
public:
    static const int GRID_SIZE = 10;
    static const int CELL_SIZE = 40;

    enum CellState {
        EMPTY, SHIP, MISS, SHIP_HIT
    };

    Board();

    void reset();
    bool placeShip(std::shared_ptr<Ship> ship);
    bool isValidPlacement(const Ship& ship) const;
    CellState getCellState(int x, int y) const;
    CellState takeShot(int x, int y);
    bool allShipsSunk() const;
    void autoPlaceShips();
    const std::vector<std::shared_ptr<Ship>>& getShips() const;

private:
    CellState grid[GRID_SIZE][GRID_SIZE];
    std::vector<std::shared_ptr<Ship>> ships;

    bool canPlaceShip(const Ship& ship) const;
};

#endif // BOARD_H