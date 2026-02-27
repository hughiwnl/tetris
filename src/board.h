#pragma once
#include <array>
#include <vector>
#include <SFML/Graphics.hpp>
#include "tetromino.h"

constexpr int BOARD_COLS       = 10;
constexpr int BOARD_ROWS       = 20; // visible rows
constexpr int BOARD_ROWS_TOTAL = 22; // +2 hidden spawn rows at top

inline const sf::Color EMPTY_COLOR = sf::Color::Black;

class Board {
public:
    Board();

    void reset();

    bool         isOccupied(int col, int row) const;
    bool         isInBounds(int col, int row) const;
    sf::Color    cellColor(int col, int row)  const;

    // Returns true if all 4 cells of the piece are in bounds and unoccupied
    bool isValidPosition(const Tetromino& piece,
                         sf::Vector2i    testPos,
                         int             testRotation) const;

    // Locks piece into board; returns number of lines cleared
    int lockPiece(const Tetromino& piece);

    // How many rows the piece can drop before hitting something
    int ghostDropDistance(const Tetromino& piece) const;

private:
    // [row][col], row 0 = topmost hidden row
    std::array<std::array<sf::Color, BOARD_COLS>, BOARD_ROWS_TOTAL> m_cells;

    std::vector<int> findFullRows() const;
    void             clearRow(int row);
};
