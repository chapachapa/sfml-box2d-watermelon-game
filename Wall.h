#pragma once

#include <box2d/box2d.h>
#include "CollidableObject.h"

class Wall : public CollidableObject {
public:
	b2Body* wallBody;
	b2BodyDef* wallBodyDef;
	float radiusInMeters;
	b2Vec2 dimensions;

	Wall(b2World* world, b2Vec2 pos, b2Vec2 dimensions);
	~Wall();

protected:
	b2PolygonShape* dynamicBox;
	b2FixtureDef* fixtureDef;
};