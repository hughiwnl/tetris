#include <SFML/Graphics.hpp>
#include "game.h"
#include "renderer.h"
#include "input.h"

int main() {
    // Window: 160 (hold) + 320 (board) + 160 (next/score) = 640 wide
    //         40 (top margin) + 640 (board) + 40 (bottom) = 720 tall
    constexpr unsigned WIN_W = 640;
    constexpr unsigned WIN_H = 720;
    constexpr int BOARD_ORIGIN_X = 160; // left edge of the play field
    constexpr int BOARD_ORIGIN_Y = 40;

    sf::RenderWindow window(sf::VideoMode({WIN_W, WIN_H}), "Tetris",
                            sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(60);

    Game         game;
    InputHandler input;
    Renderer     renderer(window, BOARD_ORIGIN_X, BOARD_ORIGIN_Y);

    // Try to load a system font. Fall back gracefully if not found.
    if (!renderer.loadFont("/System/Library/Fonts/Helvetica.ttc")) {
        // Try common Linux/Windows paths
        renderer.loadFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    }

    sf::Clock clock;

    while (window.isOpen()) {
        // Reset per-frame justPressed state before processing events
        // (input.update() is called after events so DAS timers use real dt)
        float dt = clock.restart().asSeconds();
        dt = std::min(dt, 0.05f); // clamp to avoid spiral-of-death

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                break;
            }
            input.handleEvent(*event);
        }

        input.update(dt);

        if (!game.update(input, dt)) {
            window.close();
            break;
        }

        window.clear(sf::Color(10, 10, 18));
        renderer.drawAll(game);
        window.display();
    }

    return 0;
}
