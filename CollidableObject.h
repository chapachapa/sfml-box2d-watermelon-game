#pragma once

#include <box2d/box2d.h>

enum ObjectType {
	fruit,
	wall,
	top
};

class CollidableObject {
public:
	ObjectType objectId;
};