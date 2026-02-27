#pragma once
#include <SFML/Window.hpp>
#include <array>

enum class Action {
    MoveLeft,
    MoveRight,
    SoftDrop,
    HardDrop,
    RotateCW,
    RotateCCW,
    Hold,
    Pause,
    Quit,
    Count
};

constexpr int ACTION_COUNT = static_cast<int>(Action::Count);

class InputHandler {
public:
    // Delayed Auto Shift constants
    static constexpr float DAS_DELAY    = 0.150f; // seconds before repeating
    static constexpr float DAS_INTERVAL = 0.050f; // repeat rate once triggered

    InputHandler();

    // Call for each SFML event inside the poll loop
    void handleEvent(const sf::Event& event);

    // Call once per frame — advances DAS timers using dt
    void update(float dt);

    // True only on the first frame the key was pressed
    bool isJustPressed(Action a) const;

    // True if action should fire this frame (DAS-aware for movement)
    bool isActive(Action a) const;

    bool isHeld(Action a) const;

private:
    struct KeyState {
        bool  held           = false;
        bool  justPressed    = false;
        bool  firedThisFrame = false;
        float holdTimer      = 0.f;
        bool  dasActive      = false;
        float dasTimer       = 0.f;
    };

    static sf::Keyboard::Key bindingFor(Action a);
    static bool              usesDAS(Action a);

    std::array<KeyState, ACTION_COUNT> m_states{};
};
