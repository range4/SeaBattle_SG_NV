#include "Board.h"
#include <algorithm>
#include <random>
#include <ctime>

Board::Board() {
    reset();
}

void Board::reset() {
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            grid[x][y] = EMPTY;
        }
    }
    ships.clear();
}

bool Board::placeShip(std::shared_ptr<Ship> ship) {
    if (!isValidPlacement(*ship)) {
        return false;
    }

    std::vector<Cell> cells = ship->getCells();
    for (const Cell& cell : cells) {
        if (cell.x >= 0 && cell.x < GRID_SIZE && cell.y >= 0 && cell.y < GRID_SIZE) {
            grid[cell.x][cell.y] = SHIP;
        }
    }

    ships.push_back(ship);
    return true;
}

bool Board::isValidPlacement(const Ship& ship) const {
    std::vector<Cell> cells = ship.getCells();

    for (const Cell& cell : cells) {
        // Проверка выхода за границы
        if (cell.x < 0 || cell.x >= GRID_SIZE || cell.y < 0 || cell.y >= GRID_SIZE) {
            return false;
        }

        // Проверка на пересечение с другими кораблями
        if (grid[cell.x][cell.y] != EMPTY) {
            return false;
        }

        // Проверка соседних клеток (нельзя ставить вплотную)
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int nx = cell.x + dx;
                int ny = cell.y + dy;

                if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE) {
                    if (grid[nx][ny] == SHIP) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

Board::CellState Board::getCellState(int x, int y) const {
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) {
        return EMPTY;
    }
    return grid[x][y];
}

Board::CellState Board::takeShot(int x, int y) {
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) {
        return MISS;
    }

    if (grid[x][y] == SHIP) {
        grid[x][y] = SHIP_HIT;

        // Проверка, потоплен ли корабль (БЕЗ автоматического пометки окружающих клеток)
        for (auto& ship : ships) {
            bool shipSunk = true;
            for (const Cell& cell : ship->getCells()) {
                if (grid[cell.x][cell.y] != SHIP_HIT) {
                    shipSunk = false;
                    break;
                }
            }
        }

        return SHIP_HIT;
    }
    else if (grid[x][y] == EMPTY) {
        grid[x][y] = MISS;
        return MISS;
    }

    return grid[x][y];
}

bool Board::allShipsSunk() const {
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            if (grid[x][y] == SHIP) {
                return false;
            }
        }
    }
    return true;
}

void Board::autoPlaceShips() {
    reset();

    const int shipSizes[] = { 4, 3, 3, 2, 2, 2, 1, 1, 1, 1 };
    std::vector<Ship::Direction> directions = { Ship::HORIZONTAL, Ship::VERTICAL };

    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<int> dist(0, GRID_SIZE - 1);
    std::uniform_int_distribution<int> dirDist(0, 1);

    for (int size : shipSizes) {
        bool placed = false;
        int attempts = 0;

        while (!placed && attempts < 100) {
            int x = dist(rng);
            int y = dist(rng);
            Ship::Direction dir = directions[dirDist(rng)];

            auto ship = std::make_shared<Ship>(size, x, y, dir);

            if (isValidPlacement(*ship)) {
                placeShip(ship);
                placed = true;
            }

            attempts++;
        }

        // Если не удалось разместить после 100 попыток, сбрасываем и пробуем заново
        if (!placed) {
            reset();
            autoPlaceShips();
            return;
        }
    }
}

const std::vector<std::shared_ptr<Ship>>& Board::getShips() const {
    return ships;
}