#pragma once

#include <box2d/box2d.h>
#include "CollidableObject.h"

enum FruitType {
	cherry,
	strawberry,
	grape,
	tangerine,
	orange,
	apple,
	pear,
	peach,
	pineapple,
	melon,
	watermelon
};

class Fruit : public CollidableObject {
public:
	b2Body* fruitBody;
	b2BodyDef* fruitBodyDef;
	float radiusInMeters;
	FruitType fruitId;
	bool markedForDeletion = false;
	float timeOnScreen = 0;

	Fruit(b2World* world, b2Vec2 pos, float radius, FruitType fruitId);
	~Fruit();

protected:
	b2CircleShape* dynamicCircle;
	b2FixtureDef* fixtureDef;
};