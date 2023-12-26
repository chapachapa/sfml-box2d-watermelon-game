#include "Wall.h"

Wall::Wall(b2World* world, b2Vec2 pos, b2Vec2 dimensions) {
	this->dimensions = dimensions;

	wallBodyDef = new b2BodyDef();
	wallBodyDef->type = b2_staticBody;
	wallBodyDef->position.Set(pos.x, pos.y);
	wallBody = world->CreateBody(wallBodyDef);

	dynamicBox = new b2PolygonShape();
	dynamicBox->SetAsBox(dimensions.x/2, dimensions.y/2);
	wallBody->CreateFixture(dynamicBox, 0.0f);
}

Wall::~Wall() {
}