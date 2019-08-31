#pragma once
#include "ListSerializer.h"
#include <Box2D/Box2D.h>
#include "settings.h"
#include "gameObjectData.h"

class Bullet :Serializable {
public:
	b2Body* physBody;
	b2World* wrd;

	float x = 0;
	float y = 0;

	float rot = 0;


	Bullet(b2World& world, float xx, float yy, float dir, float force);
	~Bullet();
	void updatePosition();
	void** getAttributes();
	static std::vector < ListSerializer::dataType > getAttributeTypes();
};
