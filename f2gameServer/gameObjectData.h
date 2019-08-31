#pragma once
#include <vector> 

enum class gameObjectType {
	CHUNKTYPE,
	PLAYERTYPE,
	ITEMTYPE,
	BULLETTYPE,
	POTIONTYPE
};
const std::vector < gameObjectType > gameObjectTypes = { gameObjectType::CHUNKTYPE ,gameObjectType::PLAYERTYPE ,gameObjectType::ITEMTYPE ,gameObjectType::BULLETTYPE ,gameObjectType::POTIONTYPE };

struct gameObjectDat {
	gameObjectType type;
	void* obj;//note that when ~gameObjectDat, delete obj is not called
	gameObjectDat(gameObjectType t, void* o) : type(t), obj(o)
	{
	};

};