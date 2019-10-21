#pragma once
#include <vector> 
#define GAMEOBJECTTYPES_PP(f) f CHUNKTYPE, f PLAYERTYPE, f ITEMTYPE, f BULLETTYPE, f POTIONTYPE, f BLOCKTYPE
#define GAMEOBJECTTYPESCOUNT_PP 6

enum class gameObjectType {
	GAMEOBJECTTYPES_PP()
};
const std::vector < gameObjectType > gameObjectTypes = { GAMEOBJECTTYPES_PP(gameObjectType::) };

struct gameObjectDat {
	gameObjectType type;
	void* obj;//note that when ~gameObjectDat, delete obj is not called
	gameObjectDat(gameObjectType t, void* o) : type(t), obj(o)
	{
	};
	bool operator > (gameObjectDat const& othr) {
		return(obj>othr.obj);
	}
	bool operator < (gameObjectDat const& othr) {
		return(obj < othr.obj);
	}
	bool operator == (gameObjectDat const& othr) {
		return(obj == othr.obj);
	}
};