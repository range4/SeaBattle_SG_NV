#include "Game.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <iomanip>

Game::Game()
    : window(sf::VideoMode(1000, 600), "Sea Battle", sf::Style::Close | sf::Style::Resize),
    view(window.getDefaultView()),
    gameState(GameState::MENU),
    currentPlayer(0),
    playerTurn(true),
    isVsComputer(false),
    placingPhasePlayerDone(false),
    currentShip(nullptr),
    selectedShip(nullptr),
    gameStarted(false),
    gameOverTitle("GAME OVER")
{
    // Загрузка шрифта
    if (!font.loadFromFile("assets/arial.ttf")) {
        if (!font.loadFromFile("../assets/arial.ttf")) {
            if (!font.loadFromFile("arial.ttf")) {
                if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
                    std::cerr << "Failed to load font! Using default." << std::endl;
                }
            }
        }
    }

    timerText.setFont(font);
    timerText.setCharacterSize(24);
    timerText.setFillColor(sf::Color::Black);
    timerText.setPosition(800, 10);

    // Инициализация кнопок меню
    menuButtons[0] = createButton(350, 200, 300, 50, "Play vs Friend");
    menuButtons[1] = createButton(350, 280, 300, 50, "Play vs Computer");
    menuButtons[2] = createButton(350, 360, 300, 50, "Exit");

    // Кнопки для фазы расстановки
    placeButtons[0] = createButton(50, 500, 200, 40, "Randomize");
    placeButtons[1] = createButton(260, 500, 200, 40, "Rotate");
    placeButtons[2] = createButton(470, 500, 200, 40, "Start Game");

    // Кнопка для окончания игры
    gameOverButton = createButton(350, 400, 300, 50, "Back to Menu");

    resetGame();
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

sf::RectangleShape Game::createButton(float x, float y, float width, float height, const std::string& text) {
    sf::RectangleShape button(sf::Vector2f(width, height));
    button.setPosition(x, y);
    button.setFillColor(sf::Color(70, 130, 180));
    button.setOutlineThickness(2);
    button.setOutlineColor(sf::Color::Black);
    return button;
}

void Game::drawButton(sf::RectangleShape& button, const std::string& text, unsigned int fontSize) {
    window.draw(button);

    sf::Text buttonText(text, font, fontSize);
    buttonText.setFillColor(sf::Color::White);

    // Центрирование текста на кнопке
    sf::FloatRect textRect = buttonText.getLocalBounds();
    buttonText.setOrigin(textRect.left + textRect.width / 2.0f,
        textRect.top + textRect.height / 2.0f);
    buttonText.setPosition(
        button.getPosition().x + button.getSize().x / 2.0f,
        button.getPosition().y + button.getSize().y / 2.0f
    );
    window.draw(buttonText);
}

void Game::resetGame() {
    playerBoard.reset();
    enemyBoard.reset();
    player2Board.reset();
    currentPlayer = 0;
    placingPhasePlayerDone = false;
    currentShip = nullptr;
    selectedShip = nullptr;
    playerTurn = true;
    gameStarted = false;
    timerText.setString("");

    // Создаем корабли для размещения
    const int shipSizes[] = { 4, 3, 3, 2, 2, 2, 1, 1, 1, 1 };
    shipsToPlace.clear();
    for (int size : shipSizes) {
        auto ship = std::make_shared<Ship>(size, 0, 0, Ship::Direction::HORIZONTAL);
        shipsToPlace.push_back(ship);
    }

    // Инициализация ИИ
    ai = std::make_unique<AI>(playerBoard);
}

void Game::run() {
    while (window.isOpen()) {
        handleEvents();
        update();
        render();
    }
}

void Game::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        // Обработка изменения размера окна
        else if (event.type == sf::Event::Resized) {
            sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
            view = sf::View(visibleArea);
            window.setView(view);
        }

        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);

            switch (gameState) {
            case GameState::MENU:
                handleMenuInput(mousePos);
                break;
            case GameState::PLACING_PLAYER1:
            case GameState::PLACING_PLAYER2:
                handlePlacingInput(mousePos);
                break;
            case GameState::PLAYING_VS_HUMAN:
            case GameState::PLAYING_VS_AI:
                if (!gameStarted) {
                    gameTimer.restart();
                    gameStarted = true;
                    timerText.setString("Time: 00:00"); // Инициализируем текст
                }
                handlePlayingInput(mousePos);
                break;
            case GameState::GAME_OVER:
                if (isMouseOver(gameOverButton, mousePos)) {
                    gameState = GameState::MENU;
                    // Сбрасываем флаг таймера при возврате в меню
                    gameStarted = false;
                }
                break;
            }
        }

        // Обработка клавиатуры для отладки (опционально)
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::R) {
                // Перезапуск игры по нажатию R
                resetGame();
                gameState = GameState::MENU;
            }
        }
    }

    // Фиксируем время окончания игры при завершении
    if (gameStarted && gameState == GameState::GAME_OVER) {
        gameDuration = gameTimer.getElapsedTime();
    }
}

void Game::update() {
    if (gameStarted && gameState != GameState::GAME_OVER) {
        gameDuration = gameTimer.getElapsedTime();
        // Форматируем время для отображения
        int totalSeconds = static_cast<int>(gameDuration.asSeconds());
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;

        std::stringstream timeString;
        timeString << "Time: "
            << std::setfill('0') << std::setw(2) << minutes << ":"
            << std::setfill('0') << std::setw(2) << seconds;

        timerText.setString(timeString.str());
    }
    // Ход ИИ в режиме против компьютера
    if (gameState == GameState::PLAYING_VS_AI && !playerTurn) {
        sf::sleep(sf::milliseconds(500)); // Задержка для видимости хода ИИ


        sf::Vector2i aiMove = ai->makeMove();
        Board::CellState result = playerBoard.takeShot(aiMove.x, aiMove.y);

        if (result == Board::CellState::SHIP_HIT) {
            if (playerBoard.allShipsSunk()) {
                gameOverTitle = "GAME OVER";
                winnerMessage = "Computer Wins!";
                gameState = GameState::GAME_OVER;
                gameDuration = gameTimer.getElapsedTime();
            }
        }
        else {
            playerTurn = true;
        }
    }
}

void Game::render() {
    window.clear(sf::Color(230, 230, 230)); // Светло-серый фон
    window.setView(view); // Устанавливаем вид для масштабирования

    switch (gameState) {
    case GameState::MENU:
        renderMenu();
        break;
    case GameState::PLACING_PLAYER1:
    case GameState::PLACING_PLAYER2:
        renderPlacing();
        break;
    case GameState::PLAYING_VS_HUMAN:
    case GameState::PLAYING_VS_AI:
        renderPlaying();
        break;
    case GameState::GAME_OVER:
        renderGameOver();
        break;
    }

    window.display();
}

void Game::handleMenuInput(sf::Vector2i mousePos) {
    for (int i = 0; i < 3; i++) {
        if (isMouseOver(menuButtons[i], mousePos)) {
            if (i == 0) { // Play vs Friend
                gameState = GameState::PLACING_PLAYER1;
                resetGame();
                isVsComputer = false;  // Установка режима игры
            }
            else if (i == 1) { // Play vs Computer
                gameState = GameState::PLACING_PLAYER1;
                resetGame();
                enemyBoard.autoPlaceShips();
                isVsComputer = true;  // Установка режима игры
            }
            else if (i == 2) { // Exit
                window.close();
            }
        }
    }
}

void Game::handlePlacingInput(sf::Vector2i mousePos) {
    Board& currentBoard = (gameState == GameState::PLACING_PLAYER1) ? playerBoard : player2Board;

    // Кнопка Randomize
    if (isMouseOver(placeButtons[0], mousePos)) {
        currentBoard.autoPlaceShips();
        shipsToPlace.clear();
        selectedShip = nullptr;  // Сброс выбранного корабля
        return;
    }

    // Кнопка Rotate
    if (isMouseOver(placeButtons[1], mousePos)) {
        if (selectedShip) {
            selectedShip->setDirection(
                selectedShip->getDirection() == Ship::Direction::HORIZONTAL ?
                Ship::Direction::VERTICAL : Ship::Direction::HORIZONTAL
            );
        }
        return;
    }

    // Кнопка Start Game
    if (isMouseOver(placeButtons[2], mousePos)) {
        if (shipsToPlace.empty()) {
            if (gameState == GameState::PLACING_PLAYER1) {
                if (isVsComputer) {
                    // Переходим сразу к игре против ИИ
                    playerTurn = true;
                    gameState = GameState::PLAYING_VS_AI;
                }
                else {
                    // Переходим к расстановке второго игрока
                    gameState = GameState::PLACING_PLAYER2;
                    shipsToPlace.clear();
                    selectedShip = nullptr;

                    // Создаем корабли для второго игрока
                    const int shipSizes[] = { 4, 3, 3, 2, 2, 2, 1, 1, 1, 1 };
                    for (int size : shipSizes) {
                        auto ship = std::make_shared<Ship>(size, 0, 0, Ship::Direction::HORIZONTAL);
                        shipsToPlace.push_back(ship);
                    }
                }
            }
            else {
                // Оба игрока расставили корабли, начинаем игру
                playerTurn = true;
                gameState = GameState::PLAYING_VS_HUMAN;
            }
        }
        return;
    }

    // Выбор корабля из списка доступных (справа от поля)
    int shipSelectionX = 550;  // Начало области выбора кораблей справа
    int startY = 50;

    for (size_t i = 0; i < shipsToPlace.size(); i++) {
        int width = shipsToPlace[i]->getDirection() == Ship::Direction::HORIZONTAL ?
            shipsToPlace[i]->getSize() * Board::CELL_SIZE : Board::CELL_SIZE;
        int height = shipsToPlace[i]->getDirection() == Ship::Direction::HORIZONTAL ?
            Board::CELL_SIZE : shipsToPlace[i]->getSize() * Board::CELL_SIZE;

        sf::FloatRect shipRect(shipSelectionX, startY + static_cast<float>(i) * 50.0f,
            static_cast<float>(width), static_cast<float>(height));

        if (shipRect.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
            selectedShip = shipsToPlace[i];  // Выбираем корабль
            return;
        }
    }

    // Размещение выбранного корабля на поле
    if (selectedShip &&
        mousePos.x >= 50 && mousePos.x < 50 + Board::GRID_SIZE * Board::CELL_SIZE &&
        mousePos.y >= 50 && mousePos.y < 50 + Board::GRID_SIZE * Board::CELL_SIZE) {

        int gridX = (mousePos.x - 50) / Board::CELL_SIZE;
        int gridY = (mousePos.y - 50) / Board::CELL_SIZE;

        // Корректировка позиции для краев поля
        if (selectedShip->getDirection() == Ship::Direction::HORIZONTAL) {
            if (gridX + selectedShip->getSize() > Board::GRID_SIZE) {
                gridX = Board::GRID_SIZE - selectedShip->getSize();
            }
        }
        else {
            if (gridY + selectedShip->getSize() > Board::GRID_SIZE) {
                gridY = Board::GRID_SIZE - selectedShip->getSize();
            }
        }

        selectedShip->setPosition(gridX, gridY);

        if (currentBoard.isValidPlacement(*selectedShip)) {
            currentBoard.placeShip(selectedShip);
            auto it = std::find(shipsToPlace.begin(), shipsToPlace.end(), selectedShip);
            if (it != shipsToPlace.end()) {
                shipsToPlace.erase(it);
            }
            selectedShip = nullptr;  // Сбрасываем выбранный корабль после размещения
        }
    }
    // Сброс выбранного корабля при клике вне области
    else {
        selectedShip = nullptr;
    }
}

void Game::handlePlayingInput(sf::Vector2i mousePos) {
    Board* targetBoard = nullptr;
    bool isCurrentPlayerTurn = false;

    if (gameState == GameState::PLAYING_VS_AI) {
        if (!playerTurn) return; // Не ход игрока
        targetBoard = &enemyBoard;
        isCurrentPlayerTurn = true;
    }
    else if (gameState == GameState::PLAYING_VS_HUMAN) {
        if ((currentPlayer == 0 && !playerTurn) || (currentPlayer == 1 && playerTurn)) {
            return; // Не ход текущего игрока
        }
        targetBoard = (currentPlayer == 0) ? &player2Board : &playerBoard;
        isCurrentPlayerTurn = true;
    }

    if (!isCurrentPlayerTurn) return;

    // Определяем, по какому полю кликнули (поле противника)
    int boardX = 550;
    int boardY = 50;

    if (mousePos.x >= boardX && mousePos.x < boardX + Board::GRID_SIZE * Board::CELL_SIZE &&
        mousePos.y >= boardY && mousePos.y < boardY + Board::GRID_SIZE * Board::CELL_SIZE) {

        int gridX = (mousePos.x - boardX) / Board::CELL_SIZE;
        int gridY = (mousePos.y - boardY) / Board::CELL_SIZE;

        // Проверяем, не стреляли ли уже сюда
        Board::CellState currentState = targetBoard->getCellState(gridX, gridY);
        if (currentState == Board::CellState::EMPTY || currentState == Board::CellState::SHIP) {
            Board::CellState result = targetBoard->takeShot(gridX, gridY);

            if (result == Board::CellState::SHIP_HIT) {
                if (targetBoard->allShipsSunk()) {
                    // Определяем победителя
                    gameDuration = gameTimer.getElapsedTime();
                    if (gameState == GameState::PLAYING_VS_AI) {
                        gameOverTitle = "CONGRATULATIONS!";
                        winnerMessage = "You win!";
                    }
                    else {
                        gameOverTitle = "GAME OVER";
                        winnerMessage = (currentPlayer == 0) ? "Player 1 Wins!" : "Player 2 Wins!";
                    }
                    gameState = GameState::GAME_OVER;
                }
            }
            else {
                // Передаем ход
                if (gameState == GameState::PLAYING_VS_AI) {
                    playerTurn = false;
                }
                else {
                    playerTurn = !playerTurn;
                    currentPlayer = (currentPlayer == 0) ? 1 : 0;
                }
            }
        }
    }
}

void Game::renderMenu() {
    sf::Text title("Sea Battle", font, 60);
    title.setFillColor(sf::Color::Blue);
    title.setStyle(sf::Text::Bold);
    title.setPosition(500 - title.getLocalBounds().width / 2, 80);
    window.draw(title);

    drawButton(menuButtons[0], "Play vs Friend", 24);
    drawButton(menuButtons[1], "Play vs Computer", 24);
    drawButton(menuButtons[2], "Exit", 24);
}

void Game::renderPlacing() {
    Board& currentBoard = (gameState == GameState::PLACING_PLAYER1) ? playerBoard : player2Board;
    std::string titleText = (gameState == GameState::PLACING_PLAYER1) ?
        "Player 1: Place Your Ships" : "Player 2: Place Your Ships";

    sf::Text title(titleText, font, 40);
    title.setFillColor(sf::Color::Blue);
    title.setStyle(sf::Text::Bold);
    title.setPosition(50, 10);
    window.draw(title);

    // Рисуем поле текущего игрока
    drawGrid(50, 50, currentBoard, true);

    // Рисуем доступные корабли СПРАВА от поля
    int shipSelectionX = 550;  // Позиция справа от поля
    int startY = 50;

    for (size_t i = 0; i < shipsToPlace.size(); i++) {
        int width = shipsToPlace[i]->getDirection() == Ship::Direction::HORIZONTAL ?
            shipsToPlace[i]->getSize() * Board::CELL_SIZE : Board::CELL_SIZE;
        int height = shipsToPlace[i]->getDirection() == Ship::Direction::HORIZONTAL ?
            Board::CELL_SIZE : shipsToPlace[i]->getSize() * Board::CELL_SIZE;

        sf::RectangleShape ship(sf::Vector2f(width - 4, height - 4));
        ship.setPosition(shipSelectionX, startY + static_cast<int>(i) * 50);

        // Выделяем выбранный корабль другим цветом
        if (shipsToPlace[i] == selectedShip) {
            ship.setFillColor(sf::Color(100, 200, 100));  // Зеленый для выбранного
        }
        else {
            ship.setFillColor(sf::Color(150, 150, 150));  // Серый для остальных
        }

        ship.setOutlineThickness(2);
        ship.setOutlineColor(sf::Color::Black);
        window.draw(ship);
    }

    // Рисуем кнопки
    drawButton(placeButtons[0], "Randomize", 20);
    drawButton(placeButtons[1], "Rotate", 20);
    drawButton(placeButtons[2], "Start Game", 20);

    // Показываем предпросмотр выбранного корабля на поле
    if (selectedShip) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        if (mousePos.x >= 50 && mousePos.x < 50 + Board::GRID_SIZE * Board::CELL_SIZE &&
            mousePos.y >= 50 && mousePos.y < 50 + Board::GRID_SIZE * Board::CELL_SIZE) {

            int gridX = (mousePos.x - 50) / Board::CELL_SIZE;
            int gridY = (mousePos.y - 50) / Board::CELL_SIZE;

            // Корректировка позиции для краев поля
            if (selectedShip->getDirection() == Ship::Direction::HORIZONTAL) {
                if (gridX + selectedShip->getSize() > Board::GRID_SIZE) {
                    gridX = Board::GRID_SIZE - selectedShip->getSize();
                }
            }
            else {
                if (gridY + selectedShip->getSize() > Board::GRID_SIZE) {
                    gridY = Board::GRID_SIZE - selectedShip->getSize();
                }
            }

            int width = selectedShip->getDirection() == Ship::Direction::HORIZONTAL ?
                selectedShip->getSize() * Board::CELL_SIZE : Board::CELL_SIZE;
            int height = selectedShip->getDirection() == Ship::Direction::HORIZONTAL ?
                Board::CELL_SIZE : selectedShip->getSize() * Board::CELL_SIZE;

            sf::RectangleShape shipPreview(sf::Vector2f(width - 4, height - 4));
            shipPreview.setPosition(50 + gridX * Board::CELL_SIZE, 50 + gridY * Board::CELL_SIZE);

            // Проверяем, можно ли разместить корабль в этой позиции
            Ship tempShip = *selectedShip;
            tempShip.setPosition(gridX, gridY);
            if (currentBoard.isValidPlacement(tempShip)) {
                shipPreview.setFillColor(sf::Color(100, 200, 100, 180));  // Зеленый - можно разместить
            }
            else {
                shipPreview.setFillColor(sf::Color(200, 100, 100, 180));  // Красный - нельзя разместить
            }

            shipPreview.setOutlineThickness(2);
            shipPreview.setOutlineColor(sf::Color::Black);
            window.draw(shipPreview);
        }
    }

    // Отображаем таймер, если игра началась (для второго игрока)
    if (gameStarted) {
        // Рисуем фон для таймера
        sf::RectangleShape timerBackground(sf::Vector2f(120, 30));
        timerBackground.setFillColor(sf::Color(240, 240, 240, 200)); // Полупрозрачный светлый фон
        timerBackground.setPosition(795, 5);
        window.draw(timerBackground);

        // Отображаем таймер
        window.draw(timerText);
    }
}

void Game::renderPlaying() {
    std::string playerTextStr, enemyTextStr;
    Board* ownBoard = nullptr;
    Board* enemyBoardPtr = nullptr;
    bool showOwnShips = false;  // По умолчанию скрываем корабли
    bool showEnemyShips = false; // По умолчанию скрываем корабли противника

    if (gameState == GameState::PLAYING_VS_AI) {
        playerTextStr = "Your Board";
        enemyTextStr = "Computer Board";
        ownBoard = &playerBoard;
        enemyBoardPtr = &enemyBoard;
        showOwnShips = true;  // В режиме против компьютера показываем свои корабли
    }
    else {
        if (currentPlayer == 0) {
            playerTextStr = "Player 1 Board";
            enemyTextStr = "Player 2 Board";
            ownBoard = &playerBoard;
            enemyBoardPtr = &player2Board;
        }
        else {
            playerTextStr = "Player 2 Board";
            enemyTextStr = "Player 1 Board";
            ownBoard = &player2Board;
            enemyBoardPtr = &playerBoard;
        }
        // В режиме PvP не показываем корабли ни на одном поле
        showOwnShips = false;
        showEnemyShips = false;
    }

    // Рисуем поле игрока
    sf::Text playerText(playerTextStr, font, 30);
    playerText.setFillColor(sf::Color::Blue);
    playerText.setPosition(50, 10);
    window.draw(playerText);
    drawGrid(50, 50, *ownBoard, showOwnShips);

    // Рисуем поле противника
    sf::Text enemyText(enemyTextStr, font, 30);
    enemyText.setFillColor(sf::Color::Blue);
    enemyText.setPosition(550, 10);
    window.draw(enemyText);
    drawGrid(550, 50, *enemyBoardPtr, showEnemyShips);

    // Индикатор хода
    std::string turnTextStr;
    if (gameState == GameState::PLAYING_VS_AI) {
        turnTextStr = playerTurn ? "Your Turn" : "Computer's Turn";
    }
    else {
        turnTextStr = playerTurn ? (currentPlayer == 0 ? "Player 1 Turn" : "Player 2 Turn") : "Waiting...";
    }

    sf::Text turnText(turnTextStr, font, 30);
    turnText.setPosition(500 - turnText.getLocalBounds().width / 2, 500);
    turnText.setFillColor(playerTurn ? sf::Color::Green : sf::Color::Red);
    window.draw(turnText);

    sf::RectangleShape timerBackground(sf::Vector2f(120, 30));
    timerBackground.setFillColor(sf::Color(240, 240, 240, 200)); // Полупрозрачный светлый фон
    timerBackground.setPosition(795, 5);
    window.draw(timerBackground);

    // Отображаем таймер
    window.draw(timerText);
}

void Game::renderGameOver() {
    // Заголовок
    sf::Text gameOverText(gameOverTitle, font, 60);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setStyle(sf::Text::Bold);
    gameOverText.setPosition(500 - gameOverText.getLocalBounds().width / 2, 100);
    window.draw(gameOverText);

    // Победитель
    sf::Text winner(winnerMessage, font, 40);
    winner.setFillColor(sf::Color::Blue);
    winner.setStyle(sf::Text::Bold);
    winner.setPosition(500 - winner.getLocalBounds().width / 2, 200);
    window.draw(winner);

    // Время игры
    int totalSeconds = static_cast<int>(gameDuration.asSeconds());
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    std::stringstream timeString;
    timeString << "Game duration: "
        << minutes << " minute" << (minutes != 1 ? "s" : "") << " and "
        << seconds << " second" << (seconds != 1 ? "s" : "");

    sf::Text timeText(timeString.str(), font, 30);
    timeText.setFillColor(sf::Color::Black);
    timeText.setPosition(500 - timeText.getLocalBounds().width / 2, 280);
    window.draw(timeText);

    // Кнопка возврата в меню
    drawButton(gameOverButton, "Back to Menu", 24);
}

void Game::drawGrid(int offsetX, int offsetY, Board& board, bool showShips) {
    for (int y = 0; y < Board::GRID_SIZE; y++) {
        for (int x = 0; x < Board::GRID_SIZE; x++) {
            sf::RectangleShape cell(sf::Vector2f(Board::CELL_SIZE - 2, Board::CELL_SIZE - 2));
            cell.setPosition(offsetX + x * Board::CELL_SIZE, offsetY + y * Board::CELL_SIZE);
            cell.setOutlineThickness(1);
            cell.setOutlineColor(sf::Color::Black);

            Board::CellState state = board.getCellState(x, y);
            switch (state) {
            case Board::CellState::EMPTY:
                cell.setFillColor(sf::Color(100, 149, 237)); // CornflowerBlue
                break;
            case Board::CellState::SHIP:
                cell.setFillColor(showShips ? sf::Color(70, 70, 70) : sf::Color(100, 149, 237));
                break;
            case Board::CellState::MISS:
                cell.setFillColor(sf::Color::White);
                break;
            case Board::CellState::SHIP_HIT:
                cell.setFillColor(sf::Color::Red);
                break;
            default:
                cell.setFillColor(sf::Color::Black);
            }

            window.draw(cell);
        }
    }
}

bool Game::isMouseOver(sf::RectangleShape& button, sf::Vector2i mousePos) {
    return button.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
}