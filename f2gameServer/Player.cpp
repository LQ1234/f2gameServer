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
	std::cout << "IMport " << this->onground << "\n ";
	hdl = conhdl;
	name = nme;
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.bullet = true;
	bodyDef.position.Set(worldxChunks * 10 / 2, TerrainGeneration::getGrassLevel(worldxChunks * 10 / 2) + 5);
	physBody = world.CreateBody(&bodyDef);


	b2FixtureDef fixtureDef;
	fixtureDef.density = .1f;
	fixtureDef.friction = .1f;
	fixtureDef.restitution = 0;

	b2CircleShape playerHitboxA;
	playerHitboxA.m_radius = .4;
	playerHitboxA.m_p.Set(0, .4);
	fixtureDef.shape = &playerHitboxA;
	physBody->CreateFixture(&fixtureDef);

	b2CircleShape playerHitboxB;
	playerHitboxB.m_radius = .4;
	playerHitboxB.m_p.Set(0, 1.4);
	fixtureDef.shape = &playerHitboxB;
	physBody->CreateFixture(&fixtureDef);

	b2PolygonShape playerHitboxC;
	playerHitboxC.SetAsBox(.4,.5,b2Vec2(0,.9),0);
	fixtureDef.shape = &playerHitboxC;
	physBody->CreateFixture(&fixtureDef);
	//Item collition
	b2FixtureDef fixtureDef2;
	b2PolygonShape itemCollectionBox;
	itemCollectionBox.SetAsBox(.4+.2, .5+.4+.2, b2Vec2(0, .9), 0);
	fixtureDef2.shape = &itemCollectionBox;
	fixtureDef2.isSensor = true;
	fixtureDef2.filter.maskBits = 1;
	itemCollectionFixture=physBody->CreateFixture(&fixtureDef2);

	b2FixtureDef fixtureDef3;
	b2PolygonShape groundDetectBox;
	groundDetectBox.SetAsBox(.3,  .2, b2Vec2(0, 0), 0);
	fixtureDef3.shape = &groundDetectBox;
	fixtureDef3.isSensor = true;
	fixtureDef3.filter.maskBits = 1;
	groundDetectFixture = physBody->CreateFixture(&fixtureDef3);

	b2PolygonShape wallJumpLeftBox;
	wallJumpLeftBox.SetAsBox(.2, .2, b2Vec2(-.4, .2), 0);
	fixtureDef3.shape = &wallJumpLeftBox;
	wallJumpLeftFixture = physBody->CreateFixture(&fixtureDef3);

	b2PolygonShape wallJumpRightBox;
	wallJumpRightBox.SetAsBox(.2, .2, b2Vec2(.4, .2), 0);
	fixtureDef3.shape = &wallJumpRightBox;
	wallJumpRightFixture = physBody->CreateFixture(&fixtureDef3);

	physBody->SetLinearDamping(.4);

	items[Item::GUN] = 1;
	items[Item::PICKAXE] = 1;
	items[Item::DIRTPIECE] = 127;

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
	b2Vec2 vo = physBody->GetLinearVelocity();
	xv = vo.x;
	yv = vo.y;
	rot = physBody->GetAngle();
}
void** Player::getAttributes() {
	void** attrDat = new void* [7];
	updatePosition();
	attrDat[0] = &x;
	attrDat[1] = &y;
	attrDat[2] = &xv;
	attrDat[3] = &yv;
	attrDat[4] = &rot; 
	attrDat[5] = &health;
	attrDat[6] = &name;

	return(attrDat);
}
std::vector < ListSerializer::dataType > Player::getAttributeTypes() {
	std::vector < ListSerializer::dataType > attrtypes = { ListSerializer::FLOAT, ListSerializer::FLOAT,ListSerializer::FLOAT, ListSerializer::FLOAT,ListSerializer::FLOAT, ListSerializer::FLOAT,ListSerializer::STRING };
	return(attrtypes);
};


ClientThisPlayer::ClientThisPlayer(Player* tp) :tplayer(tp) {
}
void** ClientThisPlayer::getAttributes() {
	void** attrDat = new void* [2+ Item::itemtypes.size()];
	tplayer->updatePosition();
	attrDat[0] = &(tplayer->x);
	attrDat[1] = &(tplayer->y);
	for (size_t i = 0; i < Item::itemtypes.size(); i++)
	{
		attrDat[i+2] = &(tplayer->items[(Item::itemtypes[i])]);
	}
	//attrDat[3] = &health;
	//attrDat[4] = &name;

	return(attrDat);
}
std::vector < ListSerializer::dataType > ClientThisPlayer::getAttributeTypes() {
	std::vector < ListSerializer::dataType > attrtypes = { ListSerializer::FLOAT, ListSerializer::FLOAT};
	attrtypes.reserve(attrtypes.size() + Item::itemtypes.size());
	std::vector <ListSerializer::dataType> atmints(Item::itemtypes.size(), ListSerializer::UINT);
	attrtypes.insert(attrtypes.end(), atmints.begin(), atmints.end());
	return(attrtypes);
};
