#include <Box2D/Box2D.h>
#include "ListSerializer.h"
#include "settings.h"
#include "gameObjectData.h"
#include "Item.h"

#include "Player.h"


Item::Item(b2World& world, ItemType it, unsigned int ct, float xx, float yy) :itemtype(it), count(ct){

	b2BodyDef bodyDef;
	bodyDef.allowSleep = true;

	bodyDef.type = b2_dynamicBody;
	bodyDef.bullet = false;
	bodyDef.position.Set(xx, yy);

	bodyDef.fixedRotation = false;
	bodyDef.gravityScale = .5;
	physBody = world.CreateBody(&bodyDef);


	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(objecthitboxinfo::item.x / 2, objecthitboxinfo::item.y / 2);

	b2FixtureDef fixtureDef;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 1.f;
	fixtureDef.shape = &dynamicBox;
	fixtureDef.restitution = 0;
	fixtureDef.filter.categoryBits = 1;

	physBody->CreateFixture(&fixtureDef);
	gameObjectDat* x = new gameObjectDat(gameObjectType::ITEMTYPE, this);
	physBody->SetUserData(x);
	wrd = &world;

}
Item::~Item() {

	delete physBody->GetUserData();
	physBody->SetUserData(NULL);

	wrd->DestroyBody(physBody);


	physBody = NULL;
}
void Item::updatePosition() {
	b2Vec2 ps = physBody->GetPosition();
	x = ps.x;
	y = ps.y;
	rot = physBody->GetAngle();

}
void** Item::getAttributes() {
	void** attrDat = new void* [5];
	updatePosition();
	attrDat[0] = &x;
	attrDat[1] = &y;
	attrDat[2] = &rot;
	attrDat[3] = &itemtype;
	attrDat[4] = &count;

	return(attrDat);
}
std::vector < ListSerializer::dataType > Item::getAttributeTypes() {
	std::vector < ListSerializer::dataType > attrtypes = { ListSerializer::FLOAT, ListSerializer::FLOAT, ListSerializer::FLOAT,ListSerializer::UINT, ListSerializer::UINT };
	return(attrtypes);
};


const std::vector<Item::ItemType> Item::itemtypes = { Item::SHOVEL,Item::AXE,Item::PICKAXE,Item::GUN,Item::POTION,Item::GRASSPIECE,Item::DIRTPIECE,Item::STONEPIECE };
