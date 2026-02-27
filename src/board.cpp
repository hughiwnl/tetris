#include "board.h"
#include <algorithm>

Board::Board() {
    reset();
}

void Board::reset() {
    for (auto& row : m_cells)
        row.fill(EMPTY_COLOR);
}

bool Board::isInBounds(int col, int row) const {
    return col >= 0 && col < BOARD_COLS && row >= 0 && row < BOARD_ROWS_TOTAL;
}

bool Board::isOccupied(int col, int row) const {
    if (!isInBounds(col, row)) return true; // treat out-of-bounds as occupied
    return m_cells[row][col] != EMPTY_COLOR;
}

sf::Color Board::cellColor(int col, int row) const {
    if (!isInBounds(col, row)) return EMPTY_COLOR;
    return m_cells[row][col];
}

bool Board::isValidPosition(const Tetromino& piece,
                             sf::Vector2i    testPos,
                             int             testRotation) const {
    const auto cells = piece.worldCellsAt(testPos, testRotation);
    for (const auto& c : cells) {
        if (!isInBounds(c.x, c.y)) return false;
        if (m_cells[c.y][c.x] != EMPTY_COLOR) return false;
    }
    return true;
}

int Board::lockPiece(const Tetromino& piece) {
    for (const auto& c : piece.worldCells())
        if (isInBounds(c.x, c.y))
            m_cells[c.y][c.x] = piece.color();

    auto fullRows = findFullRows();

    // Clear from bottom to top to avoid index drift
    std::sort(fullRows.begin(), fullRows.end(), std::greater<int>());
    for (int row : fullRows)
        clearRow(row);

    return static_cast<int>(fullRows.size());
}

int Board::ghostDropDistance(const Tetromino& piece) const {
    int dist = 0;
    while (dist < BOARD_ROWS_TOTAL) {
        sf::Vector2i testPos = piece.position() + sf::Vector2i{0, dist + 1};
        if (!isValidPosition(piece, testPos, piece.rotationState()))
            break;
        ++dist;
    }
    return dist;
}

std::vector<int> Board::findFullRows() const {
    std::vector<int> full;
    for (int r = 0; r < BOARD_ROWS_TOTAL; ++r) {
        bool rowFull = true;
        for (int c = 0; c < BOARD_COLS; ++c) {
            if (m_cells[r][c] == EMPTY_COLOR) { rowFull = false; break; }
        }
        if (rowFull) full.push_back(r);
    }
    return full;
}

void Board::clearRow(int row) {
    // Shift all rows above down by one
    for (int r = row; r > 0; --r)
        m_cells[r] = m_cells[r - 1];
    // Top row becomes empty
    m_cells[0].fill(EMPTY_COLOR);
}
