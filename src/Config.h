#pragma once

namespace sf {
    class RenderWindow;
}

namespace Config {
    constexpr int  windowWidth   = 1200;
    constexpr int  windowHeight  = 900;
    constexpr char windowTitle[] = "SFML Game";
    constexpr bool kDebugDraw    = false;

    // Set by main after creating the window, used for input utilities.
    inline sf::RenderWindow* gWindow = nullptr;
} // namespace Config
