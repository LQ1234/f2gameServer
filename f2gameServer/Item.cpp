#include <Box2D/Box2D.h>
#include "ListSerializer.h"
#include "settings.h"
#include "gameObjectData.h"
#include "Item.h"



Item::Item(b2World& world, ItemType it, unsigned int ct, float xx, float yy) :itemtype(it), count(ct) {

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.bullet = false;
	bodyDef.position.Set(xx, yy);

	bodyDef.fixedRotation = true;
	bodyDef.gravityScale = .5;
	physBody = world.CreateBody(&bodyDef);


	b2CircleShape circleShape;
	circleShape.m_p.Set(0, 0);
	circleShape.m_radius = objecthitboxinfo::item;

	b2FixtureDef fixtureDef;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 1.f;
	fixtureDef.shape = &circleShape;
	fixtureDef.restitution = 0;

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
}
void** Item::getAttributes() {
	void** attrDat = new void* [4];
	updatePosition();
	attrDat[0] = &x;
	attrDat[1] = &y;
	attrDat[2] = &itemtype;
	attrDat[3] = &count;

	return(attrDat);
}
std::vector < ListSerializer::dataType > Item::getAttributeTypes() {
	std::vector < ListSerializer::dataType > attrtypes = { ListSerializer::FLOAT, ListSerializer::FLOAT,ListSerializer::UINT, ListSerializer::UINT };
	return(attrtypes);
};


const std::vector<Item::ItemType> Item::itemtypes = { Item::SHOVEL,Item::AXE,Item::PICKAXE,Item::GUN,Item::POTION,Item::GRASSPIECE,Item::DIRTPIECE,Item::STONEPIECE };
