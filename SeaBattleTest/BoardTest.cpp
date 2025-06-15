#include "gtest/gtest.h"
#include "pch.h"
#include "../SeaBattle_SG_NV/Board.h"
#include "../SeaBattle_SG_NV/Ship.h"

class BoardTest : public ::testing::Test {
protected:
    void SetUp() override {
        board.reset();
    }

    Board board;
};

TEST_F(BoardTest, ValidShipPlacement) {
    Ship ship(3, 2, 2, Ship::Direction::HORIZONTAL);
    EXPECT_TRUE(board.isValidPlacement(ship));

    auto sharedShip = std::make_shared<Ship>(ship);
    board.placeShip(sharedShip);

    // Проверка занятых клеток
    EXPECT_EQ(board.getCellState(2, 2), Board::SHIP);
    EXPECT_EQ(board.getCellState(3, 2), Board::SHIP);
    EXPECT_EQ(board.getCellState(4, 2), Board::SHIP);
}

TEST_F(BoardTest, InvalidPlacements) {
    Ship validShip(3, 2, 2, Ship::Direction::HORIZONTAL);
    auto sharedShip = std::make_shared<Ship>(validShip);
    board.placeShip(sharedShip);

    // Пересечение
    Ship overlapping(3, 3, 3, Ship::Direction::VERTICAL);
    EXPECT_FALSE(board.isValidPlacement(overlapping));

    // Соседняя клетка
    Ship nearby(2, 1, 1, Ship::Direction::HORIZONTAL);
    EXPECT_FALSE(board.isValidPlacement(nearby));

    // Выход за границы
    Ship outOfBounds(4, 8, 8, Ship::Direction::HORIZONTAL);
    EXPECT_FALSE(board.isValidPlacement(outOfBounds));
}

TEST_F(BoardTest, ShotHandling) {
    auto ship = std::make_shared<Ship>(3, 2, 2, Ship::Direction::HORIZONTAL);
    board.placeShip(ship);

    EXPECT_EQ(board.takeShot(2, 2), Board::SHIP_HIT);
    EXPECT_EQ(board.takeShot(0, 0), Board::MISS);
    EXPECT_EQ(board.takeShot(2, 2), Board::SHIP_HIT); // Повторный выстрел

    board.takeShot(3, 2);
    board.takeShot(4, 2);
    EXPECT_TRUE(board.allShipsSunk());
}

TEST_F(BoardTest, AutoPlacement) {
    board.autoPlaceShips();

    int shipCells = 0;
    for (int y = 0; y < Board::GRID_SIZE; y++) {
        for (int x = 0; x < Board::GRID_SIZE; x++) {
            if (board.getCellState(x, y) == Board::SHIP) {
                shipCells++;
            }
        }
    }

    // Проверка общего количества клеток кораблей
    // (4+3+3+2+2+2+1+1+1+1 = 20)
    EXPECT_EQ(shipCells, 20);

    // Проверка количества кораблей
    EXPECT_EQ(board.getShips().size(), 10);
}

TEST_F(BoardTest, ResetFunctionality) {
    auto ship = std::make_shared<Ship>(3, 2, 2, Ship::Direction::HORIZONTAL);
    board.placeShip(ship);
    board.takeShot(2, 2);

    board.reset();

    // Проверка сброса
    EXPECT_EQ(board.getCellState(2, 2), Board::EMPTY);
    EXPECT_TRUE(board.getShips().empty());
    EXPECT_TRUE(board.allShipsSunk()); // На пустой доске все корабли "потоплены"
}