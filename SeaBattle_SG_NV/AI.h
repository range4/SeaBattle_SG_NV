#pragma once
#include "Board.h"
#include <SFML/System/Vector2.hpp>  // Добавлено для sf::Vector2i
#include <vector>

#ifndef AI_H
#define AI_H

class AI {
public:
    AI(const Board& playerBoard);

    sf::Vector2i makeMove();

private:
    const Board& playerBoard;
    std::vector<sf::Vector2i> possibleMoves;

    void initializePossibleMoves();
};

#endif // AI_H