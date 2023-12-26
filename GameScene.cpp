#include "GameScene.h"

const float FRUIT_RADII[11] = {
	22, 26, 35, 45, 57, 70, 85, 100, 108, 142, 163
};

const sf::Color FRUIT_COLORS[11] = {
	sf::Color(231, 95, 57),
	sf::Color(244, 10, 9),
	sf::Color(169, 107, 255),
	sf::Color(254, 183, 3),
	sf::Color(253, 133, 17),
	sf::Color(245, 22, 22),
	sf::Color(253, 241, 124),
	sf::Color(253, 202, 196),
	sf::Color(239, 241, 22),
	sf::Color(152, 232, 34),
	sf::Color(23, 159, 12)
};

const int COMBO_POINTS[11] = {1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66};

const sf::Vector2f LEFT_WALL_COORD(500-25, 500);
const sf::Vector2f RIGHT_WALL_COORD(1100+25, 500);
const sf::Vector2f BOTTOM_WALL_COORD(800, 850);
const sf::Vector2f VERTICAL_WALL_DIM(50, 660);
const sf::Vector2f HORIZONTAL_WALL_DIM(600+100, 50);

const float CURSOR_LEFT_BOUND = 500.0f + 55;
const float CURSOR_RIGHT_BOUND = 1100.0f - 55;
const float CURSOR_HEIGHT = 123.0f;

const int DROPPABLE_FRUIT_ID_MAX = 5;

const float FRUIT_DROP_COOLDOWN = 0.6;
const float MERGE_COOLDOWN = 0.3;
const float FRUIT_GRACE_PERIOD = 1.0;
const float GAMEOVER_HEIGHT = 170;

struct queuedFruit {
	FruitType fruitId;
	b2Vec2 fruitPos;
};

vector<queuedFruit> fruitsToBeRendered;

int totalPoints = 0;
int highScore = 0;

class FruitContactListener : public b2ContactListener {
	void BeginContact(b2Contact* contact) {
		b2BodyUserData* bodyAUserData = &contact->GetFixtureA()->GetBody()->GetUserData();
		b2BodyUserData* bodyBUserData = &contact->GetFixtureB()->GetBody()->GetUserData();
		if (bodyAUserData && bodyBUserData) {
			CollidableObject* objectA = (CollidableObject*)(bodyAUserData->pointer);
			CollidableObject* objectB = (CollidableObject*)(bodyBUserData->pointer);

			if ( objectA != nullptr && objectB != nullptr
				&& objectA->objectId == ObjectType::fruit && objectB->objectId == ObjectType::fruit) {
				
				Fruit* fruitA = (Fruit*)objectA;
				Fruit* fruitB = (Fruit*)objectB;

				if ( fruitA->fruitId == fruitB->fruitId
					&& !fruitA->markedForDeletion
					&& !fruitB->markedForDeletion) {
					fruitA->markedForDeletion = true;
					fruitB->markedForDeletion = true;
					b2Vec2 posA = contact->GetFixtureA()->GetBody()->GetPosition();
					b2Vec2 posB = contact->GetFixtureB()->GetBody()->GetPosition();
					totalPoints += COMBO_POINTS[fruitA->fruitId];
					if (fruitA->fruitId < 10) {
						// if you merge 2 watermelons, there is no next fruit, just poofs
						fruitsToBeRendered.push_back({
							(FruitType)(fruitA->fruitId + 1),
							b2Vec2((posA.x + posB.x) / 2, (posA.y + posB.y) / 2) });
					}
				}
			}
		}
	}
};

GameScene::GameScene() {
	pixelsPerMeter = 50;
	sceneSizeInPixels = sf::Vector2f(1600, 900);
	world = new b2World(b2Vec2(0.0f, -10.0f)); // gravity

	b2BodyDef* groundBodyDef = new b2BodyDef();
	groundBodyDef->position.Set(PixelToMeter(sceneSizeInPixels.x/2), PixelToMeter(-25));
	groundBody = world->CreateBody(groundBodyDef);

	groundBox.SetAsBox(PixelToMeter(1600/2), PixelToMeter(50/2));
	groundBody->CreateFixture(&groundBox, 0.0f);

	walls.push_back(new Wall(world,
		DisplayCoordToPhysicsCoord(LEFT_WALL_COORD),
		b2Vec2(PixelToMeter(VERTICAL_WALL_DIM.x), PixelToMeter(VERTICAL_WALL_DIM.y))));
	walls.push_back(new Wall(world,
		DisplayCoordToPhysicsCoord(RIGHT_WALL_COORD),
		b2Vec2(PixelToMeter(VERTICAL_WALL_DIM.x), PixelToMeter(VERTICAL_WALL_DIM.y))));
	walls.push_back(new Wall(world,
		DisplayCoordToPhysicsCoord(BOTTOM_WALL_COORD),
		b2Vec2(PixelToMeter(HORIZONTAL_WALL_DIM.x), PixelToMeter(HORIZONTAL_WALL_DIM.y))));

	FruitContactListener* fruitContactListener = new FruitContactListener();
	world->SetContactListener(fruitContactListener);

	nextFruitId = (FruitType)floor(rand() % DROPPABLE_FRUIT_ID_MAX);
	currentGameState = GameStates::play;

	std::ifstream highscoreSaveFile("highscore.txt");
	string scoreStr;
	if (highscoreSaveFile.is_open())
	{
		getline(highscoreSaveFile, scoreStr);
		highScore = stoi(scoreStr);
		highscoreSaveFile.close();
	}
	else {
		std::cout << "Unable to open file" << std::endl;
	}
}

void GameScene::update(float dt, sf::RenderWindow &window) {
	switch (currentGameState) {
	case GameStates::play:
		mousePosition = sf::Vector2f(sf::Mouse::getPosition(window));
		cursorPosition = sf::Vector2f(
			clampFloat(mousePosition.x, CURSOR_LEFT_BOUND, CURSOR_RIGHT_BOUND),
			CURSOR_HEIGHT);

		if (mouseIsPressedInWindow()) {
			if (!mouseIsPressed) {
				mouseJustPressed = true;
			}
			mouseIsPressed = true;
		}
		else {
			mouseIsPressed = false;
		}

		dropTime -= dt;
		if (dropTime < 0 && mouseJustPressed) {
			Fruit* newFruit = new Fruit(
				world,
				DisplayCoordToPhysicsCoord(cursorPosition),
				PixelToMeter(FRUIT_RADII[nextFruitId]),
				nextFruitId
			);
			fruitsInScene.push_back(newFruit);

			dropTime = FRUIT_DROP_COOLDOWN;
			nextFruitId = (FruitType)floor(rand() % DROPPABLE_FRUIT_ID_MAX);
		}

		// update fruits - delete any marked for deletion
		//					then create if any new ones need to be made
		for (int i = fruitsInScene.size() - 1; i >= 0; i--) {
			fruitsInScene[i]->timeOnScreen += dt;
			if (fruitsInScene[i]->timeOnScreen > FRUIT_GRACE_PERIOD) {
				float y = PhysicsCoordToDisplayCoord(fruitsInScene[i]->fruitBody->GetPosition()).y
					- fruitsInScene[i]->radiusInMeters;
				if (y < GAMEOVER_HEIGHT) {
					// IT'S OVER!!
					currentGameState = GameStates::gameover;
				}
			}
			if (fruitsInScene[i]->markedForDeletion) {
				Fruit* toBeDeleted = fruitsInScene[i];
				fruitsInScene.erase(fruitsInScene.begin() + i);
				delete toBeDeleted;
				toBeDeleted = nullptr;
			}
		}
		for (int i = fruitsToBeRendered.size() - 1; i >= 0; i--) {
			Fruit* newFruit = new Fruit(
				world,
				fruitsToBeRendered[i].fruitPos,
				PixelToMeter(FRUIT_RADII[fruitsToBeRendered[i].fruitId]),
				fruitsToBeRendered[i].fruitId
			);
			fruitsInScene.push_back(newFruit);
			fruitsToBeRendered.erase(fruitsToBeRendered.begin() + i);
		}

		world->Step(dt, velocityIterations, positionIterations);

		mouseJustPressed = false;

		break;
	case GameStates::gameover:
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
			if (!enterIsPressed) {
				enterJustPressed = true;
			}
			enterIsPressed = true;
		}
		else {
			enterIsPressed = false;
		}
		if (enterJustPressed) {
			if (totalPoints > highScore) {
				highScore = totalPoints;
			}
			for (int i = fruitsInScene.size() - 1; i >= 0; i--) {
				Fruit* toBeDeleted = fruitsInScene[i];
				fruitsInScene.erase(fruitsInScene.begin() + i);
				delete toBeDeleted;
				toBeDeleted = nullptr;
			}
			for (int i = fruitsToBeRendered.size() - 1; i >= 0; i--) {
				fruitsToBeRendered.erase(fruitsToBeRendered.begin() + i);
			}
			currentGameState = GameStates::play;
			enterJustPressed = false;

			ofstream highscoreSaveFile("highscore.txt");
			if (highscoreSaveFile.is_open())
			{
				highscoreSaveFile << to_string(highScore);
				highscoreSaveFile.close();
			}
			else {
				std::cout << "Unable to open file" << std::endl;
			}
		}
		break;
	default:
		cout << "Update::Unknown State" << endl;
		break;
	}
}

void GameScene::render(sf::RenderWindow& window, sf::Font& font) {

	// draw ground
	b2Vec2 groundPos = groundBody->GetPosition();
	sf::RectangleShape groundRect(sf::Vector2f(1600, 50));
	groundRect.setOrigin(800.0f, 25.0f);
	groundRect.setFillColor(sf::Color::White);
	groundRect.setPosition(PhysicsCoordToDisplayCoord(groundPos));
	window.draw(groundRect);

	// draw limit
	sf::RectangleShape limit(sf::Vector2f(600, 3));
	limit.setPosition(500, GAMEOVER_HEIGHT);
	limit.setOrigin(sf::Vector2f(0, 1.5));
	limit.setFillColor(sf::Color(255, 0, 0, 100));
	window.draw(limit);

	// draw beaker
	for (int i = 0; i < walls.size(); i++) {
		sf::Vector2f posInPixels = PhysicsCoordToDisplayCoord(walls[i]->wallBody->GetPosition());
		sf::Vector2f dimInPixels(MeterToPixel(walls[i]->dimensions.x), 
			MeterToPixel(walls[i]->dimensions.y));

		sf::RectangleShape wallRect(dimInPixels);
		wallRect.setOrigin(dimInPixels.x / 2, dimInPixels.y / 2);
		wallRect.setPosition(posInPixels);
		wallRect.setFillColor(sf::Color::White);
		window.draw(wallRect);
	}

	// draw cursor
	sf::RectangleShape guide(sf::Vector2f(3, VERTICAL_WALL_DIM.y));
	guide.setPosition(cursorPosition);
	guide.setOrigin(sf::Vector2f(1.5, 0));
	guide.setFillColor(sf::Color(255, 255, 255, 50));
	window.draw(guide);

	// draw fruit to be dropped
	if (dropTime <= 0) {
		drawFruit(window,
			FRUIT_RADII[nextFruitId],
			0,
			cursorPosition,
			FRUIT_COLORS[nextFruitId]
		);
	}
	
	// draw objects
	for (int i = 0; i < fruitsInScene.size(); i++) {
		b2Vec2 pos = fruitsInScene[i]->fruitBody->GetPosition();
		float angle = fruitsInScene[i]->fruitBody->GetAngle();
		float radiusInPixels = MeterToPixel(fruitsInScene[i]->radiusInMeters);

		drawFruit(window,
			radiusInPixels,
			angle,
			PhysicsCoordToDisplayCoord(pos),
			FRUIT_COLORS[fruitsInScene[i]->fruitId]
		);
	}

	// score and high score
	sf::Text scoreText("Score:", font, 60);
	scoreText.setFillColor(sf::Color::White);
	scoreText.setPosition(250, 300);
	scoreText.setOrigin(scoreText.getLocalBounds().width / 2, scoreText.getLocalBounds().height / 2);
	window.draw(scoreText);
	sf::Text scoreValueText(to_string(totalPoints), font, 120);
	scoreValueText.setFillColor(sf::Color::White);
	scoreValueText.setPosition(250, 350);
	scoreValueText.setOrigin(scoreValueText.getLocalBounds().width / 2, scoreValueText.getLocalBounds().height / 2);
	window.draw(scoreValueText);

	sf::Text highScoreText("High Score:", font, 60);
	highScoreText.setFillColor(sf::Color::White);
	highScoreText.setPosition(250, 600);
	highScoreText.setOrigin(highScoreText.getLocalBounds().width / 2, highScoreText.getLocalBounds().height / 2);
	window.draw(highScoreText);
	sf::Text highScoreValueText(to_string(highScore), font, 120);
	highScoreValueText.setFillColor(sf::Color::White);
	highScoreValueText.setPosition(250, 650);
	highScoreValueText.setOrigin(highScoreValueText.getLocalBounds().width / 2, highScoreValueText.getLocalBounds().height / 2);
	window.draw(highScoreValueText);

	switch (currentGameState) {
	case GameStates::play:
		break;
	case GameStates::gameover:
		// display gameover screen
		renderGameoverScreen(window, font);
		break;
	default:
		cout << "Render::Unknown State" << endl;
		break;
	}
}

float GameScene::MeterToPixel(float val) {
	return pixelsPerMeter * val;
}

float GameScene::PixelToMeter(float val) {
	return val / pixelsPerMeter;
}

sf::Vector2f GameScene::PhysicsCoordToDisplayCoord(b2Vec2 coord) {
	return sf::Vector2f(MeterToPixel(coord.x), sceneSizeInPixels.y - MeterToPixel(coord.y));
}

b2Vec2 GameScene::DisplayCoordToPhysicsCoord(sf::Vector2f coord) {
	return b2Vec2(PixelToMeter(coord.x), PixelToMeter(sceneSizeInPixels.y - coord.y));
}

bool GameScene::mouseIsPressedInWindow() {

	return 
		sf::Mouse::isButtonPressed(sf::Mouse::Left)
		&& mousePosition.x >= 0
		&& mousePosition.y >= 0
		&& mousePosition.x <= sceneSizeInPixels.x
		&& mousePosition.y <= sceneSizeInPixels.y;
}

float GameScene::clampFloat(float val, float min, float max) {
	if (val < min) {
		return min;
	}
	else if (val > max) {
		return max;
	}
	else {
		return val;
	}
}


void GameScene::drawFruit(sf::RenderWindow& window, float radius, float angle, sf::Vector2f pos, sf::Color color) {
	sf::CircleShape fruitCircle(radius);
	fruitCircle.setOrigin(radius, radius);
	fruitCircle.setRotation(angle);
	fruitCircle.setPosition(pos);
	fruitCircle.setFillColor(color);

	window.draw(fruitCircle);
}

void GameScene::renderGameoverScreen(sf::RenderWindow& window, sf::Font& font) {

	sf::RectangleShape winScreen(sceneSizeInPixels);
	winScreen.setPosition(0, 0);
	winScreen.setFillColor(sf::Color(255, 255, 255, 200));
	window.draw(winScreen);

	std::string winString = "- GAME OVER -";
	sf::Text winText(winString, font, 30);
	winText.setFillColor(sf::Color::Red);
	winText.setPosition(sceneSizeInPixels.x / 2.0f, window.getSize().y / 2.0f);
	winText.setOrigin(winText.getLocalBounds().width / 2, winText.getLocalBounds().height / 2);
	window.draw(winText);

	std::string restartString = "Press ENTER to restart";
	sf::Text restartText(restartString, font, 24);
	restartText.setFillColor(sf::Color::Red);
	restartText.setPosition(sceneSizeInPixels.x / 2.0f, 30.0f + (window.getSize().y / 2.0f));
	restartText.setOrigin(restartText.getLocalBounds().width / 2, restartText.getLocalBounds().height / 2);
	window.draw(restartText);
}