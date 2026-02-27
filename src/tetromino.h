#pragma once
#include <array>
#include <SFML/Graphics.hpp>

enum class TetrominoType : int {
    I = 0, J, L, O, S, T, Z,
    Count
};

// Static data uses plain C-arrays to avoid nested std::array brace-init issues.
// rotations[state][cell][0=x, 1=y]
struct TetrominoData {
    int       rotations[4][4][2];
    sf::Color color;
};

// kick offsets[from_state][attempt][0=x, 1=y] — 5 attempts per transition
struct KickData {
    int offsets[4][5][2];
};

extern const TetrominoData TETROMINO_DATA[7];

extern const KickData SRS_KICKS_JLSTZ_CW;
extern const KickData SRS_KICKS_JLSTZ_CCW;
extern const KickData SRS_KICKS_I_CW;
extern const KickData SRS_KICKS_I_CCW;

class Tetromino {
public:
    explicit Tetromino(TetrominoType type);

    TetrominoType type()          const { return m_type; }
    int           rotationState() const { return m_rotation; }
    sf::Vector2i  position()      const { return m_pos; }
    sf::Color     color()         const;

    std::array<sf::Vector2i, 4> worldCells() const;
    std::array<sf::Vector2i, 4> worldCellsAt(sf::Vector2i pos, int rotation) const;

    void setPosition(sf::Vector2i pos) { m_pos = pos; }
    void setRotation(int state)        { m_rotation = state & 3; }

private:
    TetrominoType m_type;
    sf::Vector2i  m_pos;
    int           m_rotation;
};
