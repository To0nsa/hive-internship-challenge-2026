#include "Config.h"
#include "ResourceManager.h"
#include "gamestates/IState.h"
#include "gamestates/StateMenu.h"
#include "gamestates/StateStack.h"

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <memory>
#include <optional>
#include <stack>
#include <iostream>

int main(int argc, char* argv[]) try {
    (void)argc;

    // ResourceManager must be instantiated here -- DO NOT CHANGE
    ResourceManager::init(argv[0]);

    sf::RenderWindow window(sf::VideoMode({Config::windowWidth, Config::windowHeight}),
                            Config::windowTitle);
    Config::gWindow = &window;
    window.setKeyRepeatEnabled(false);
    window.setVerticalSyncEnabled(true);

    StateStack gamestates;
    gamestates.push<StateMenu>();

    sf::Clock clock;
    while (window.isOpen()) {
        sf::Time elapsedTime = clock.restart();

        IState* pState = gamestates.getCurrentState();
        if (!pState)
            return EXIT_FAILURE;

        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // Dispatch event to current state
            pState->handleEvent(event.value());
        }

        pState->update(elapsedTime.asSeconds());

        window.clear();
        pState->render(window);
        window.display();

        gamestates.performPendingPops();
    }

    return EXIT_SUCCESS;

} catch (const std::exception& e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
} catch (...) {
    std::cerr << "Fatal error: unknown exception occurred" << std::endl;
    return EXIT_FAILURE;
}
