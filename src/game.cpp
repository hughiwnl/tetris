#include "game.h"
#include <algorithm>
#include <cmath>

// NES-style line clear score multipliers
static constexpr int LINE_MULTIPLIERS[] = {0, 40, 100, 300, 1200};

// Gravity intervals from Tetris Guideline (seconds per row)
float Game::gravityInterval(int level) const {
    if (level <= 0) level = 1;
    if (level > 20) level = 20;
    // Formula: (0.8 - (level-1)*0.007)^(level-1)
    float base = 0.8f - (level - 1) * 0.007f;
    return std::pow(base, level - 1);
}

// ---------------------------------------------------------------------------
// Construction / reset
// ---------------------------------------------------------------------------

Game::Game() {
    std::random_device rd;
    m_rng.seed(rd());
    reset();
}

void Game::reset() {
    m_board.reset();
    m_held.reset();
    m_holdUsed = false;
    m_score    = {};
    m_state    = GameState::Playing;

    m_gravityAccum    = 0.f;
    m_gravityInterval = gravityInterval(1);
    m_lockTimer       = 0.f;
    m_onGround        = false;

    // Initialize both halves with shuffled bags
    m_bagIndex = 0;
    std::array<TetrominoType, 7> types = {
        TetrominoType::I, TetrominoType::J, TetrominoType::L,
        TetrominoType::O, TetrominoType::S, TetrominoType::T,
        TetrominoType::Z
    };
    std::shuffle(types.begin(), types.end(), m_rng);
    for (int i = 0; i < 7; ++i) m_bag[i] = types[i];
    std::shuffle(types.begin(), types.end(), m_rng);
    for (int i = 0; i < 7; ++i) m_bag[i + 7] = types[i];

    spawnPiece(drawFromBag());
}

// ---------------------------------------------------------------------------
// Bag randomizer
// ---------------------------------------------------------------------------

void Game::refillBag() {
    // Shift second half -> first half
    for (int i = 0; i < 7; ++i)
        m_bag[i] = m_bag[i + 7];

    // Refill second half with a fresh shuffle
    std::array<TetrominoType, 7> fresh = {
        TetrominoType::I, TetrominoType::J, TetrominoType::L,
        TetrominoType::O, TetrominoType::S, TetrominoType::T,
        TetrominoType::Z
    };
    std::shuffle(fresh.begin(), fresh.end(), m_rng);
    for (int i = 0; i < 7; ++i)
        m_bag[i + 7] = fresh[i];
}

TetrominoType Game::drawFromBag() {
    if (m_bagIndex >= 7) {
        refillBag();
        m_bagIndex = 0;
    }
    return m_bag[m_bagIndex++];
}

std::array<TetrominoType, 3> Game::nextPieces() const {
    // Peek next 3 from bag without consuming (always valid since bag has 14 slots)
    std::array<TetrominoType, 3> next{};
    for (int i = 0; i < 3; ++i)
        next[i] = m_bag[m_bagIndex + i];
    return next;
}

// ---------------------------------------------------------------------------
// Piece management
// ---------------------------------------------------------------------------

void Game::spawnPiece(TetrominoType type) {
    m_current = std::make_unique<Tetromino>(type);
    // Spawn at top-center (hidden rows 0-1, visible starts at row 2)
    int spawnCol = BOARD_COLS / 2 - 1; // col 4 for 10-wide board
    m_current->setPosition({spawnCol, 1});

    m_lockTimer = 0.f;
    m_onGround  = false;

    // Game over if spawn position is already blocked
    if (!m_board.isValidPosition(*m_current, m_current->position(), 0)) {
        m_state = GameState::GameOver;
    }

    updateGhost();
}

void Game::updateGhost() {
    m_ghostRow = m_board.ghostDropDistance(*m_current);
}

bool Game::isOnGround() const {
    return !m_board.isValidPosition(*m_current,
                                    m_current->position() + sf::Vector2i{0, 1},
                                    m_current->rotationState());
}

// ---------------------------------------------------------------------------
// Movement
// ---------------------------------------------------------------------------

void Game::tryMove(int dx, int dy) {
    sf::Vector2i newPos = m_current->position() + sf::Vector2i{dx, dy};
    if (m_board.isValidPosition(*m_current, newPos, m_current->rotationState())) {
        m_current->setPosition(newPos);
        if (dy == 0) m_lockTimer = 0.f; // move reset on lateral movement
        updateGhost();
    }
}

void Game::tryRotate(int direction) {
    int fromState = m_current->rotationState();
    int toState   = (fromState + direction + 4) % 4;

    // O-piece: skip rotation
    if (m_current->type() == TetrominoType::O) return;

    const KickData* kickData = nullptr;
    if (m_current->type() == TetrominoType::I) {
        kickData = (direction > 0) ? &SRS_KICKS_I_CW : &SRS_KICKS_I_CCW;
    } else {
        kickData = (direction > 0) ? &SRS_KICKS_JLSTZ_CW : &SRS_KICKS_JLSTZ_CCW;
    }

    for (int k = 0; k < 5; ++k) {
        int kx = kickData->offsets[fromState][k][0];
        int ky = kickData->offsets[fromState][k][1];
        sf::Vector2i testPos = m_current->position() + sf::Vector2i{kx, ky};
        if (m_board.isValidPosition(*m_current, testPos, toState)) {
            m_current->setPosition(testPos);
            m_current->setRotation(toState);
            m_lockTimer = 0.f; // move reset
            updateGhost();
            return;
        }
    }
    // All kicks failed — rotation is silent no-op
}

void Game::hardDrop() {
    int dist = m_board.ghostDropDistance(*m_current);
    m_current->setPosition(m_current->position() + sf::Vector2i{0, dist});
    // Hard drop scoring: 2 points per row
    m_score.score += 2 * dist;
    lockCurrent();
}

// ---------------------------------------------------------------------------
// Hold
// ---------------------------------------------------------------------------

void Game::activateHold() {
    if (m_holdUsed) return;
    m_holdUsed = true;

    TetrominoType currentType = m_current->type();

    if (!m_held) {
        // First hold: stash current, spawn next from bag
        m_held = std::make_unique<Tetromino>(currentType);
        spawnPiece(drawFromBag());
    } else {
        // Swap current with held
        TetrominoType swapType = m_held->type();
        m_held = std::make_unique<Tetromino>(currentType);
        spawnPiece(swapType);
    }
}

// ---------------------------------------------------------------------------
// Locking and scoring
// ---------------------------------------------------------------------------

void Game::lockCurrent() {
    int cleared = m_board.lockPiece(*m_current);
    addScore(cleared);
    m_holdUsed = false; // allow hold again on new piece
    spawnPiece(drawFromBag());
}

void Game::addScore(int lines) {
    if (lines > 0 && lines <= 4) {
        m_score.score += LINE_MULTIPLIERS[lines] * m_score.level;
        m_score.combo++;
        m_score.score += 50 * m_score.combo * m_score.level;
    } else {
        m_score.combo = 0;
    }

    m_score.lines += lines;
    int newLevel = m_score.lines / 10 + 1;
    if (newLevel > m_score.level) {
        m_score.level     = newLevel;
        m_gravityInterval = gravityInterval(newLevel);
    }
}

// ---------------------------------------------------------------------------
// Main update
// ---------------------------------------------------------------------------

bool Game::update(InputHandler& input, float dt) {
    if (input.isJustPressed(Action::Quit)) return false;

    if (input.isJustPressed(Action::Pause)) {
        if (m_state == GameState::Playing)
            m_state = GameState::Paused;
        else if (m_state == GameState::Paused)
            m_state = GameState::Playing;
    }

    if (m_state == GameState::GameOver) {
        if (input.isJustPressed(Action::HardDrop)) reset();
        return true;
    }

    if (m_state == GameState::Paused) return true;

    // --- Input ---
    if (input.isActive(Action::MoveLeft))  tryMove(-1, 0);
    if (input.isActive(Action::MoveRight)) tryMove( 1, 0);
    if (input.isJustPressed(Action::RotateCW))  tryRotate( 1);
    if (input.isJustPressed(Action::RotateCCW)) tryRotate(-1);
    if (input.isJustPressed(Action::Hold))      activateHold();
    if (input.isJustPressed(Action::HardDrop))  { hardDrop(); return true; }

    // Soft drop: accelerate gravity
    float effectiveInterval = m_gravityInterval;
    if (input.isHeld(Action::SoftDrop)) {
        effectiveInterval = std::min(effectiveInterval, 0.05f);
        m_score.score += 1; // 1 point per soft-drop row (handled via gravity below)
    }

    // --- Gravity ---
    m_gravityAccum += dt;
    while (m_gravityAccum >= effectiveInterval) {
        m_gravityAccum -= effectiveInterval;
        sf::Vector2i below = m_current->position() + sf::Vector2i{0, 1};
        if (m_board.isValidPosition(*m_current, below, m_current->rotationState())) {
            m_current->setPosition(below);
            updateGhost();
        }
    }

    // --- Lock delay ---
    m_onGround = isOnGround();
    if (m_onGround) {
        m_lockTimer += dt;
        if (m_lockTimer >= m_lockDelay) {
            lockCurrent();
        }
    } else {
        m_lockTimer = 0.f;
    }

    return true;
}
