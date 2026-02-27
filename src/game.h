#pragma once
#include <SFML/System.hpp>
#include <memory>
#include <optional>
#include <array>
#include <random>
#include "board.h"
#include "tetromino.h"
#include "input.h"

enum class GameState {
    Playing,
    Paused,
    GameOver,
};

struct ScoreState {
    int score = 0;
    int level = 1;
    int lines = 0;
    int combo = 0;
};

class Game {
public:
    static constexpr int CELL_PX = 32;

    Game();

    void reset();

    // Returns false when the game requests the window to close (Quit action)
    bool update(InputHandler& input, float dt);

    // Read-only accessors for Renderer
    const Board&      board()    const { return m_board; }
    const Tetromino&  current()  const { return *m_current; }
    const ScoreState& score()    const { return m_score; }
    GameState         state()    const { return m_state; }
    int               ghostRow() const { return m_ghostRow; }
    bool              holdUsed() const { return m_holdUsed; }

    // nullptr if nothing is held
    const Tetromino* held() const { return m_held.get(); }

    // Next 3 upcoming pieces (lookahead into the bag)
    std::array<TetrominoType, 3> nextPieces() const;

private:
    Board                      m_board;
    std::unique_ptr<Tetromino> m_current;
    std::unique_ptr<Tetromino> m_held;
    bool                       m_holdUsed = false;

    // 7-bag randomizer
    std::array<TetrominoType, 14> m_bag; // two bags buffered for lookahead
    int                           m_bagIndex = 14;
    std::mt19937                  m_rng;

    ScoreState m_score;
    GameState  m_state = GameState::Playing;

    float m_gravityAccum    = 0.f;
    float m_gravityInterval = 1.f;

    float m_lockTimer = 0.f;
    float m_lockDelay = 0.5f;
    bool  m_onGround  = false;

    int m_ghostRow = 0;

    void          refillBag();
    TetrominoType drawFromBag();
    void          spawnPiece(TetrominoType type);
    void          tryMove(int dx, int dy);
    void          tryRotate(int direction); // +1 CW, -1 CCW
    void          hardDrop();
    void          activateHold();
    void          lockCurrent();
    void          updateGhost();
    void          addScore(int linesCleared);
    float         gravityInterval(int level) const;
    bool          isOnGround() const;
};
