#pragma once
#include <set>

#include <Box2D/Box2D.h>
#include "ListSerializer.h"
#include <websocketpp/common/thread.hpp>
#include <websocketpp/server.hpp>
#include "Chunk.h"
#include "TerrainGeneration.h"
#include "gameObjectData.h"
#include "Item.h"

class Player :Serializable {
public:
	b2Body* physBody;
	b2World* wrd;
	websocketpp::connection_hdl hdl;
	std::string name;
	float health = 100;
	std::set<Chunk*> viewChunks;
	float x = 0;
	float y = 0;
	float rot = 0;
	std::map<Item::ItemType, unsigned int> items;

	bool* movementkeyboard = new bool[4];
	bool movementImpulseJumpLast = false;

	Player(b2World& world, std::string nme, websocketpp::connection_hdl conhdl);
	~Player();
	void updatePosition();
	void** getAttributes();
	static std::vector < ListSerializer::dataType > getAttributeTypes();
};

class ClientThisPlayer :Serializable {
public:
	Player* tplayer;
	ClientThisPlayer(Player* tp);
	void** getAttributes() ;
	static std::vector < ListSerializer::dataType > getAttributeTypes();
};