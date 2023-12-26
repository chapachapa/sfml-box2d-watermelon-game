#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Main.hpp>

#include <Box2D/Box2D.h>

#include "GameScene.h"

#include <iostream>
using namespace std;

int main()
{
    sf::RenderWindow window(sf::VideoMode(1600, 900), "Watermelon Game");

    static sf::Font font;
    if (!font.loadFromFile("visitor1.ttf")) {
        std::cout << "unable to load font. Sometimes this happens please try again" << std::endl;
    }

    GameScene gameScene;
    sf::Clock deltaSrc;

    srand(time(NULL));

    while (window.isOpen())
    {
        sf::Time dt_time = deltaSrc.restart();
        float dt = dt_time.asSeconds();

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();

        gameScene.update(dt, window);
        gameScene.render(window, font);

        window.display();
    }

    return 0;
}