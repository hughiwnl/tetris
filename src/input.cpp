#include "input.h"

sf::Keyboard::Key InputHandler::bindingFor(Action a) {
    switch (a) {
        case Action::MoveLeft:   return sf::Keyboard::Key::Left;
        case Action::MoveRight:  return sf::Keyboard::Key::Right;
        case Action::SoftDrop:   return sf::Keyboard::Key::Down;
        case Action::HardDrop:   return sf::Keyboard::Key::Space;
        case Action::RotateCW:   return sf::Keyboard::Key::Up;
        case Action::RotateCCW:  return sf::Keyboard::Key::Z;
        case Action::Hold:       return sf::Keyboard::Key::C;
        case Action::Pause:      return sf::Keyboard::Key::P;
        case Action::Quit:       return sf::Keyboard::Key::Escape;
        default:                 return sf::Keyboard::Key::Unknown;
    }
}

bool InputHandler::usesDAS(Action a) {
    return a == Action::MoveLeft || a == Action::MoveRight || a == Action::SoftDrop;
}

InputHandler::InputHandler() {
    m_states.fill(KeyState{});
}

void InputHandler::handleEvent(const sf::Event& event) {
    // SFML 3 event handling via std::visit / getIf
    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        for (int i = 0; i < ACTION_COUNT; ++i) {
            Action a = static_cast<Action>(i);
            if (kp->code == bindingFor(a) && !m_states[i].held) {
                m_states[i].held        = true;
                m_states[i].justPressed = true;
                m_states[i].firedThisFrame = true;
                m_states[i].holdTimer   = 0.f;
                m_states[i].dasActive   = false;
                m_states[i].dasTimer    = 0.f;
            }
        }
    } else if (const auto* kr = event.getIf<sf::Event::KeyReleased>()) {
        for (int i = 0; i < ACTION_COUNT; ++i) {
            Action a = static_cast<Action>(i);
            if (kr->code == bindingFor(a)) {
                m_states[i].held        = false;
                m_states[i].dasActive   = false;
                m_states[i].holdTimer   = 0.f;
                m_states[i].dasTimer    = 0.f;
            }
        }
    }
}

void InputHandler::update(float dt) {
    for (int i = 0; i < ACTION_COUNT; ++i) {
        auto& s = m_states[i];
        // justPressed only lasts one frame
        s.justPressed    = false;
        s.firedThisFrame = false;

        if (s.held && usesDAS(static_cast<Action>(i))) {
            s.holdTimer += dt;
            if (s.holdTimer >= DAS_DELAY) {
                s.dasActive = true;
                s.dasTimer += dt;
                if (s.dasTimer >= DAS_INTERVAL) {
                    s.firedThisFrame = true;
                    s.dasTimer = 0.f;
                }
            }
        }
    }
}

bool InputHandler::isJustPressed(Action a) const {
    return m_states[static_cast<int>(a)].justPressed;
}

bool InputHandler::isActive(Action a) const {
    const auto& s = m_states[static_cast<int>(a)];
    return s.justPressed || s.firedThisFrame;
}

bool InputHandler::isHeld(Action a) const {
    return m_states[static_cast<int>(a)].held;
}
