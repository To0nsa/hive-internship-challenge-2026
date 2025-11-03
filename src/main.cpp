#include "ResourceManager.h"
#include "gamestates/StateStack.h"
#include "gamestates/IState.h"
#include "gamestates/StateMenu.h"
#include "Config.h"
#include <memory>
#include <stack>
#include <optional>
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

int main(int argc, char* argv[])
{
    (void)argc;

    // ResourceManager must be instantiated here -- DO NOT CHANGE
    ResourceManager::init(argv[0]);

    sf::RenderWindow window(sf::VideoMode({Config::windowWidth, Config::windowHeight}), Config::windowTitle);
    window.setKeyRepeatEnabled(false);
    window.setVerticalSyncEnabled(true);

    StateStack gamestates;
    if (!gamestates.push<StateMenu>())
        return -1;

    sf::Clock clock;
    while (window.isOpen())
    {
        sf::Time elapsedTime = clock.restart();

        IState* pState = gamestates.getCurrentState();
        if (!pState) return -1;

        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        pState->update(elapsedTime.asSeconds());
        window.clear();
        pState->render(window);
        window.display();

        gamestates.performDeferredPops();
    }
    
    return 0;
}
