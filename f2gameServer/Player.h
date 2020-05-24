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
#include <vector>
#include <unordered_map>



class Player :Serializable {
public:
	ListSerializer ls;
	b2Body* physBody;
	b2World* wrd;
	websocketpp::connection_hdl hdl;
	std::string name;
	float health = 100;
	std::set<Chunk*> viewChunks;
	float x = 0;
	float y = 0;
	float xv = 0;
	float yv = 0;
	float rot = 0;
	float sprint = 0;//0 to 1
	std::map<Item::ItemType, unsigned int> items;

	bool movementkeyboard[4];
	float mouseposition[2];
	bool mousedown = false;
	bool movementImpulseJumpLast = false;

	std::unordered_map<b2Body*,int> onground;
	std::unordered_map<b2Body*, int> wallJumpLeft;
	std::unordered_map<b2Body*, int> wallJumpRight;

	b2Fixture* itemCollectionFixture;
	b2Fixture* groundDetectFixture;
	b2Fixture* wallJumpLeftFixture;
	b2Fixture* wallJumpRightFixture;

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