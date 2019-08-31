#pragma once
#include <Box2D/Box2D.h>
#include "ListSerializer.h"
#include "settings.h"
#include "gameObjectData.h"

class Item :Serializable {
public:
	b2Body* physBody;
	b2World* wrd;

	float x = 0;
	float y = 0;
	enum ItemType :unsigned int {
		SHOVEL,
		AXE,
		PICKAXE,
		GUN,
		POTION,
		GRASSPIECE,
		DIRTPIECE,
		STONEPIECE
	};
	static const std::vector<ItemType> itemtypes;


	ItemType itemtype;
	unsigned int count;

	Item(b2World& world, ItemType it, unsigned int ct, float xx, float yy);
	~Item();
	void updatePosition();
	void** getAttributes();
	static std::vector < ListSerializer::dataType > getAttributeTypes();
};
