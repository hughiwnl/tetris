#pragma once
#include <SFML/Graphics.hpp>
#include "game.h"

class Renderer {
public:
    // boardOriginX/Y: top-left pixel of the visible play field
    explicit Renderer(sf::RenderWindow& window,
                      int boardOriginX = 200,
                      int boardOriginY = 40);

    // Load font — call once before the first drawAll()
    bool loadFont(const std::string& path);

    void drawAll(const Game& game);

private:
    sf::RenderWindow& m_window;
    sf::Font          m_font;
    bool              m_fontLoaded = false;

    int m_originX;
    int m_originY;

    // Panel dimensions
    static constexpr int PANEL_W = 160;
    static constexpr int BOARD_W = BOARD_COLS * Game::CELL_PX; // 320
    static constexpr int BOARD_H = BOARD_ROWS * Game::CELL_PX; // 640

    void drawBackground();
    void drawBoard(const Board& board);
    void drawGhost(const Tetromino& current, int ghostDist);
    void drawPiece(const Tetromino& piece, sf::Vector2i screenOffset, uint8_t alpha = 255);
    void drawPiecePreview(TetrominoType type, sf::Vector2f center, uint8_t alpha = 255);
    void drawHoldSlot(const Game& game);
    void drawNextPieces(const std::array<TetrominoType, 3>& next);
    void drawUI(const ScoreState& score, GameState state);

    // Convert board col/row -> screen pixel position (accounts for 2 hidden rows)
    sf::Vector2f boardToScreen(int col, int row) const;

    sf::RectangleShape makeCell(float x, float y, sf::Color color, uint8_t alpha = 255) const;
    void               drawLabel(const std::string& text, float x, float y, unsigned size = 16);
    void               drawValue(const std::string& text, float x, float y, unsigned size = 20);
};
