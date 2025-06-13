#include "AI.h"
#include <algorithm>
#include <random>

AI::AI(const Board& playerBoard) : playerBoard(playerBoard) {
    initializePossibleMoves();
}

void AI::initializePossibleMoves() {
    possibleMoves.clear();
    for (int y = 0; y < Board::GRID_SIZE; y++) {
        for (int x = 0; x < Board::GRID_SIZE; x++) {
            possibleMoves.push_back(sf::Vector2i(x, y));
        }
    }

    // Перемешиваем возможные ходы
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(possibleMoves.begin(), possibleMoves.end(), g);
}

sf::Vector2i AI::makeMove() {
    if (possibleMoves.empty()) {
        return sf::Vector2i(-1, -1);
    }

    sf::Vector2i move = possibleMoves.back();
    possibleMoves.pop_back();
    return move;
}