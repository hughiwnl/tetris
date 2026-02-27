#include "renderer.h"
#include <string>

Renderer::Renderer(sf::RenderWindow& window, int boardOriginX, int boardOriginY)
    : m_window(window), m_originX(boardOriginX), m_originY(boardOriginY)
{}

bool Renderer::loadFont(const std::string& path) {
    if (m_font.openFromFile(path)) {
        m_fontLoaded = true;
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Coordinate helpers
// ---------------------------------------------------------------------------

sf::Vector2f Renderer::boardToScreen(int col, int row) const {
    // Board rows 0-1 are hidden. Visible area starts at row 2.
    return {
        static_cast<float>(m_originX + col * Game::CELL_PX),
        static_cast<float>(m_originY + (row - 2) * Game::CELL_PX)
    };
}

sf::RectangleShape Renderer::makeCell(float x, float y, sf::Color color, uint8_t alpha) const {
    sf::RectangleShape rect({static_cast<float>(Game::CELL_PX - 1),
                              static_cast<float>(Game::CELL_PX - 1)});
    rect.setPosition({x, y});
    color.a = alpha;
    rect.setFillColor(color);
    return rect;
}

// ---------------------------------------------------------------------------
// Text helpers
// ---------------------------------------------------------------------------

void Renderer::drawLabel(const std::string& text, float x, float y, unsigned size) {
    if (!m_fontLoaded) return;
    sf::Text t(m_font, text, size);
    t.setFillColor(sf::Color(180, 180, 180));
    t.setPosition({x, y});
    m_window.draw(t);
}

void Renderer::drawValue(const std::string& text, float x, float y, unsigned size) {
    if (!m_fontLoaded) return;
    sf::Text t(m_font, text, size);
    t.setFillColor(sf::Color::White);
    t.setPosition({x, y});
    m_window.draw(t);
}

// ---------------------------------------------------------------------------
// Draw methods
// ---------------------------------------------------------------------------

void Renderer::drawBackground() {
    // Board area
    sf::RectangleShape boardBg({static_cast<float>(BOARD_W), static_cast<float>(BOARD_H)});
    boardBg.setPosition({static_cast<float>(m_originX), static_cast<float>(m_originY)});
    boardBg.setFillColor(sf::Color(15, 15, 25));
    boardBg.setOutlineColor(sf::Color(60, 60, 80));
    boardBg.setOutlineThickness(2.f);
    m_window.draw(boardBg);

    // Grid lines
    for (int r = 1; r < BOARD_ROWS; ++r) {
        sf::RectangleShape line({static_cast<float>(BOARD_W), 1.f});
        line.setPosition({static_cast<float>(m_originX),
                          static_cast<float>(m_originY + r * Game::CELL_PX)});
        line.setFillColor(sf::Color(30, 30, 45));
        m_window.draw(line);
    }
    for (int c = 1; c < BOARD_COLS; ++c) {
        sf::RectangleShape line({1.f, static_cast<float>(BOARD_H)});
        line.setPosition({static_cast<float>(m_originX + c * Game::CELL_PX),
                          static_cast<float>(m_originY)});
        line.setFillColor(sf::Color(30, 30, 45));
        m_window.draw(line);
    }

    // Left panel (hold)
    sf::RectangleShape leftPanel({static_cast<float>(PANEL_W - 8), 120.f});
    leftPanel.setPosition({static_cast<float>(m_originX - PANEL_W + 4),
                           static_cast<float>(m_originY + 30)});
    leftPanel.setFillColor(sf::Color(20, 20, 35));
    leftPanel.setOutlineColor(sf::Color(60, 60, 80));
    leftPanel.setOutlineThickness(1.f);
    m_window.draw(leftPanel);

    // Right panel (next)
    sf::RectangleShape rightPanel({static_cast<float>(PANEL_W - 8), 360.f});
    rightPanel.setPosition({static_cast<float>(m_originX + BOARD_W + 4),
                            static_cast<float>(m_originY + 30)});
    rightPanel.setFillColor(sf::Color(20, 20, 35));
    rightPanel.setOutlineColor(sf::Color(60, 60, 80));
    rightPanel.setOutlineThickness(1.f);
    m_window.draw(rightPanel);
}

void Renderer::drawBoard(const Board& board) {
    for (int r = 2; r < BOARD_ROWS_TOTAL; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            sf::Color color = board.cellColor(c, r);
            if (color == EMPTY_COLOR) continue;
            auto [sx, sy] = boardToScreen(c, r);
            m_window.draw(makeCell(sx, sy, color));
        }
    }
}

void Renderer::drawGhost(const Tetromino& current, int ghostDist) {
    sf::Vector2i ghostPos = current.position() + sf::Vector2i{0, ghostDist};
    const auto cells = current.worldCellsAt(ghostPos, current.rotationState());
    sf::Color ghostColor = current.color();
    for (const auto& c : cells) {
        if (c.y < 2) continue; // skip hidden rows
        auto [sx, sy] = boardToScreen(c.x, c.y);
        m_window.draw(makeCell(sx, sy, ghostColor, 60));
    }
}

void Renderer::drawPiece(const Tetromino& piece, sf::Vector2i screenOffset, uint8_t alpha) {
    for (const auto& c : piece.worldCells()) {
        if (c.y < 2) continue;
        auto [sx, sy] = boardToScreen(c.x, c.y);
        m_window.draw(makeCell(sx + screenOffset.x, sy + screenOffset.y, piece.color(), alpha));
    }
}

void Renderer::drawPiecePreview(TetrominoType type, sf::Vector2f center, uint8_t alpha) {
    Tetromino tmp(type);
    sf::Color color = tmp.color();
    const auto& rot = TETROMINO_DATA[static_cast<int>(type)].rotations[0];
    for (int i = 0; i < 4; ++i) {
        float px = center.x + rot[i][0] * Game::CELL_PX;
        float py = center.y + rot[i][1] * Game::CELL_PX;
        m_window.draw(makeCell(px - Game::CELL_PX / 2.f,
                                py - Game::CELL_PX / 2.f,
                                color, alpha));
    }
}

void Renderer::drawHoldSlot(const Game& game) {
    float lx = static_cast<float>(m_originX - PANEL_W + 4);
    float ly = static_cast<float>(m_originY);
    drawLabel("HOLD", lx + 8, ly + 6, 14);

    uint8_t alpha = game.holdUsed() ? 80 : 255;

    if (game.held()) {
        drawPiecePreview(game.held()->type(),
                         {lx + PANEL_W / 2.f - 8, ly + 80.f},
                         alpha);
    }
}

void Renderer::drawNextPieces(const std::array<TetrominoType, 3>& next) {
    float rx = static_cast<float>(m_originX + BOARD_W + 4);
    float ry = static_cast<float>(m_originY);
    drawLabel("NEXT", rx + 8, ry + 6, 14);

    for (int i = 0; i < 3; ++i) {
        float cy = ry + 70.f + i * 110.f;
        drawPiecePreview(next[i], {rx + PANEL_W / 2.f - 8, cy});
    }
}

void Renderer::drawUI(const ScoreState& score, GameState state) {
    // Score panel below the right next panel
    float rx = static_cast<float>(m_originX + BOARD_W + 4);
    float ry = static_cast<float>(m_originY + 400);

    drawLabel("SCORE", rx + 8, ry);
    drawValue(std::to_string(score.score), rx + 8, ry + 18);

    drawLabel("LEVEL", rx + 8, ry + 55);
    drawValue(std::to_string(score.level), rx + 8, ry + 73);

    drawLabel("LINES", rx + 8, ry + 110);
    drawValue(std::to_string(score.lines), rx + 8, ry + 128);

    if (state == GameState::Paused) {
        // Dim overlay
        sf::RectangleShape overlay({static_cast<float>(BOARD_W), static_cast<float>(BOARD_H)});
        overlay.setPosition({static_cast<float>(m_originX), static_cast<float>(m_originY)});
        overlay.setFillColor(sf::Color(0, 0, 0, 160));
        m_window.draw(overlay);

        drawLabel("PAUSED", static_cast<float>(m_originX + BOARD_W / 2 - 30),
                  static_cast<float>(m_originY + BOARD_H / 2 - 10), 24);
    }

    if (state == GameState::GameOver) {
        sf::RectangleShape overlay({static_cast<float>(BOARD_W), static_cast<float>(BOARD_H)});
        overlay.setPosition({static_cast<float>(m_originX), static_cast<float>(m_originY)});
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        m_window.draw(overlay);

        drawLabel("GAME OVER", static_cast<float>(m_originX + BOARD_W / 2 - 50),
                  static_cast<float>(m_originY + BOARD_H / 2 - 24), 24);
        drawLabel("SPACE to restart", static_cast<float>(m_originX + BOARD_W / 2 - 65),
                  static_cast<float>(m_originY + BOARD_H / 2 + 10), 16);
    }
}

// ---------------------------------------------------------------------------
// Main draw entry
// ---------------------------------------------------------------------------

void Renderer::drawAll(const Game& game) {
    drawBackground();
    drawBoard(game.board());

    if (game.state() == GameState::Playing || game.state() == GameState::Paused) {
        drawGhost(game.current(), game.ghostRow());
        drawPiece(game.current(), {0, 0});
    }

    drawHoldSlot(game);
    drawNextPieces(game.nextPieces());
    drawUI(game.score(), game.state());
}
