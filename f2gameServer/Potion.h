#pragma once
#include "ListSerializer.h"
#include <Box2D/Box2D.h>
#include "gameObjectData.h"

class Potion :Serializable {
public:
	b2Body* physBody;
	b2World* wrd;

	float x = 0;
	float y = 0;
	enum PotionType :unsigned int {
		AGILITY,
		INVISIBILITY,
		NIGHTVISION
	};
	PotionType potiontype;

	Potion(b2World& world, PotionType pt, float xx, float yy, float dir, float force);
	~Potion();
	void updatePosition();
	void** getAttributes();
	static std::vector < ListSerializer::dataType > getAttributeTypes();
};

