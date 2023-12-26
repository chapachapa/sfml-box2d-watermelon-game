#pragma once

#include <SFML/Main.hpp>
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <iostream>
#include <fstream>
using namespace std;

#include "Fruit.h"
#include "Wall.h"

enum GameStates {
	play,
	gameover
};

class GameScene {
public:
	// constructor and basic lifecycle methods
	GameScene();
	void update(float dt, sf::RenderWindow& window);
	void render(sf::RenderWindow& window, sf::Font& font);

	// variables for helper methods
	// and unit conversions
	float pixelsPerMeter;
	sf::Vector2f sceneSizeInPixels;
	float MeterToPixel(float val);
	float PixelToMeter(float val);
	sf::Vector2f PhysicsCoordToDisplayCoord(b2Vec2 coord);
	b2Vec2 DisplayCoordToPhysicsCoord(sf::Vector2f coord);
	void drawFruit(sf::RenderWindow& window, float radius, float angle, sf::Vector2f pos, sf::Color color);
	void renderGameoverScreen(sf::RenderWindow& window, sf::Font& font);

protected:
	GameStates currentGameState;
	b2Vec2 gravity;
	b2World* world;
	b2Body* groundBody;
	b2PolygonShape groundBox;
	//b2Body* testBody;
	vector<Fruit*> fruitsInScene;
	vector<Wall*> walls;
	float dropTime = 0;
	bool mouseIsPressed = false;
	bool mouseJustPressed = false;
	bool enterIsPressed = false;
	bool enterJustPressed = false;

	// Cursor Variables
	sf::Vector2f mousePosition;
	sf::Vector2f cursorPosition;
	FruitType nextFruitId;
	bool mouseIsPressedInWindow();
	float clampFloat(float val, float min, float max);

	// variables for dynamic box testing
	int velocityIterations = 8;
	int positionIterations = 3;
};