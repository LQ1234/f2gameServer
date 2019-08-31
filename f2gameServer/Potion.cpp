#include "ListSerializer.h"
#include <Box2D/Box2D.h>
#include "gameObjectData.h"
#include "Potion.h"



Potion::Potion(b2World& world, PotionType pt, float xx, float yy, float dir, float force) :potiontype(pt) {

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.bullet = true;
	bodyDef.position.Set(xx, yy);
	bodyDef.linearVelocity = b2Vec2(std::cos(dir) * force, std::sin(dir) * force);
	bodyDef.fixedRotation = true;
	bodyDef.gravityScale = .5;
	physBody = world.CreateBody(&bodyDef);


	b2CircleShape circleShape;
	circleShape.m_p.Set(0, 0);
	circleShape.m_radius = 1;

	b2FixtureDef fixtureDef;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 1.f;
	fixtureDef.shape = &circleShape;
	fixtureDef.restitution = 0;

	physBody->CreateFixture(&fixtureDef);
	gameObjectDat* x = new gameObjectDat(gameObjectType::POTIONTYPE, this);
	physBody->SetUserData(x);
	wrd = &world;

}
Potion::~Potion() {
	delete physBody->GetUserData();
	physBody->SetUserData(NULL);

	wrd->DestroyBody(physBody);
	physBody = NULL;
}
void Potion::updatePosition() {
	b2Vec2 ps = physBody->GetPosition();
	x = ps.x;
	y = ps.y;
}
void** Potion::getAttributes() {
	void** attrDat = new void* [3];
	updatePosition();
	attrDat[0] = &x;
	attrDat[1] = &y;
	attrDat[2] = &potiontype;

	return(attrDat);
}
std::vector < ListSerializer::dataType > Potion::getAttributeTypes() {
	std::vector < ListSerializer::dataType > attrtypes = { ListSerializer::FLOAT, ListSerializer::FLOAT,ListSerializer::UINT };
	return(attrtypes);
};


