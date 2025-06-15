#pragma once
#include <SFML/Graphics.hpp>
#include "Board.h"
#include "AI.h"

class Game {
public:
    enum class GameState {
        MENU, PLACING_PLAYER1, PLACING_PLAYER2, PLAYING_VS_HUMAN, PLAYING_VS_AI, GAME_OVER
    };

    Game();
    void run();

private:
    sf::RenderWindow window;
    sf::Font font;
    sf::View view;  // Добавляем вид для масштабирования
    GameState gameState;
    int currentPlayer;
    bool playerTurn;
    bool isVsComputer;
    bool placingPhasePlayerDone;
    std::shared_ptr<Ship> currentShip;
    std::vector<std::shared_ptr<Ship>> shipsToPlace;
    std::shared_ptr<Ship> selectedShip;

    Board playerBoard;
    Board enemyBoard;
    Board player2Board;

    sf::Clock gameTimer;      // Таймер игры
    sf::Time gameDuration;    // Продолжительность игры
    bool gameStarted;
    sf::Text timerText;

    std::unique_ptr<AI> ai;

    sf::RectangleShape menuButtons[3];
    sf::RectangleShape placeButtons[3];
    sf::RectangleShape gameOverButton;

    std::string winnerMessage;
    std::string gameOverTitle;

    void handleEvents();
    void update();
    void render();
    void resetGame();
    void handleMenuInput(sf::Vector2i mousePos);
    void handlePlacingInput(sf::Vector2i mousePos);
    void handlePlayingInput(sf::Vector2i mousePos);
    void renderMenu();
    void renderPlacing();
    void renderPlaying();
    void renderGameOver();
    void drawGrid(int offsetX, int offsetY, Board& board, bool showShips);
    void drawButton(sf::RectangleShape& button, const std::string& text, unsigned int fontSize);
    bool isMouseOver(sf::RectangleShape& button, sf::Vector2i mousePos);

    sf::RectangleShape createButton(float x, float y, float width, float height, const std::string& text);
};