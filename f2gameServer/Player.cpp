#pragma once
#include <set>

#include <Box2D/Box2D.h>
#include "ListSerializer.h"
#include <websocketpp/common/thread.hpp>
#include <websocketpp/server.hpp>
#include "Chunk.h"
#include "TerrainGeneration.h"
#include "gameObjectData.h"
#include "Player.h"



Player::Player(b2World& world, std::string nme, websocketpp::connection_hdl conhdl) {
	hdl = conhdl;
	name = nme;
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.bullet = true;
	bodyDef.position.Set(worldxChunks * 10 / 2, TerrainGeneration::getGrassLevel(worldxChunks * 10 / 2) + 5);
	physBody = world.CreateBody(&bodyDef);

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(objecthitboxinfo::player.x / 2, objecthitboxinfo::player.y / 2);

	b2FixtureDef fixtureDef;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 1.f;
	fixtureDef.shape = &dynamicBox;
	fixtureDef.restitution = 0;

	physBody->CreateFixture(&fixtureDef);
	gameObjectDat* x = new gameObjectDat(gameObjectType::PLAYERTYPE, this);
	physBody->SetUserData(x);
	wrd = &world;

}
Player::~Player() {
	delete physBody->GetUserData();
	physBody->SetUserData(NULL);

	wrd->DestroyBody(physBody);
	physBody = NULL;
}
void Player::updatePosition() {
	b2Vec2 ps = physBody->GetPosition();
	x = ps.x;
	y = ps.y;
	rot = physBody->GetAngle();
}
void** Player::getAttributes() {
	void** attrDat = new void* [5];
	updatePosition();
	attrDat[0] = &x;
	attrDat[1] = &y;
	attrDat[2] = &rot;
	attrDat[3] = &health;
	attrDat[4] = &name;

	return(attrDat);
}
std::vector < ListSerializer::dataType > Player::getAttributeTypes() {
	std::vector < ListSerializer::dataType > attrtypes = { ListSerializer::FLOAT, ListSerializer::FLOAT,ListSerializer::FLOAT, ListSerializer::FLOAT,ListSerializer::STRING };
	return(attrtypes);
};

ClientThisPlayer::ClientThisPlayer(Player* tp) :tplayer(tp) {
}
void** ClientThisPlayer::getAttributes() {
	void** attrDat = new void* [2];
	tplayer->updatePosition();
	attrDat[0] = &(tplayer->x);
	attrDat[1] = &(tplayer->y);
	//attrDat[3] = &health;
	//attrDat[4] = &name;

	return(attrDat);
}
std::vector < ListSerializer::dataType > ClientThisPlayer::getAttributeTypes() {
	std::vector < ListSerializer::dataType > attrtypes = { ListSerializer::FLOAT, ListSerializer::FLOAT,ListSerializer::FLOAT, ListSerializer::FLOAT,ListSerializer::STRING };
	return(attrtypes);
};
