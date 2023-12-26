#include "fruit.h"

Fruit::Fruit(b2World* world, b2Vec2 pos, float radiusInMeters, FruitType fruitId) {
	this->radiusInMeters = radiusInMeters;
	this->fruitId = fruitId;

	fruitBodyDef = new b2BodyDef();
	fruitBodyDef->type = b2_dynamicBody;
	fruitBodyDef->position.Set(pos.x, pos.y);
	fruitBodyDef->linearDamping = 0.6f;
	fruitBody = world->CreateBody(fruitBodyDef);

	dynamicCircle = new b2CircleShape();
	dynamicCircle->m_radius = radiusInMeters;

	// used to attach a shape to a body for collision detection
	fixtureDef = new b2FixtureDef();
	fixtureDef->shape = dynamicCircle;
	fixtureDef->density = 100.0f;
	fixtureDef->friction = 0.1f;
	fixtureDef->restitution = 0.1f;

	fruitBody->CreateFixture(fixtureDef);
	fruitBody->GetUserData().pointer = (uintptr_t) this;
}

Fruit::~Fruit() {
	fruitBody->GetWorld()->DestroyBody(fruitBody);
}