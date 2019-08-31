#include "ListSerializer.h"
#include <Box2D/Box2D.h>
#include "settings.h"
#include "gameObjectData.h"
#include "Bullet.h"



Bullet::Bullet(b2World& world, float xx, float yy, float dir, float force) {

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.bullet = true;
	bodyDef.position.Set(xx, yy);
	bodyDef.angle = dir;
	bodyDef.linearVelocity = b2Vec2(std::cos(dir) * force, std::sin(dir) * force);
	bodyDef.fixedRotation = true;
	physBody = world.CreateBody(&bodyDef);


	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(objecthitboxinfo::bullet.x / 2, objecthitboxinfo::bullet.y / 2);

	b2FixtureDef fixtureDef;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 1.f;
	fixtureDef.shape = &dynamicBox;
	fixtureDef.restitution = 0;

	physBody->CreateFixture(&fixtureDef);
	gameObjectDat* x = new gameObjectDat(gameObjectType::BULLETTYPE, this);
	physBody->SetUserData(x);
	wrd = &world;

}
Bullet::~Bullet() {
	delete physBody->GetUserData();
	physBody->SetUserData(NULL);

	wrd->DestroyBody(physBody);
	physBody = NULL;
}
void Bullet::updatePosition() {
	b2Vec2 ps = physBody->GetPosition();
	x = ps.x;
	y = ps.y;
	rot = physBody->GetAngle();
}
void** Bullet::getAttributes() {
	void** attrDat = new void* [3];
	updatePosition();
	attrDat[0] = &x;
	attrDat[1] = &y;
	attrDat[2] = &rot;

	return(attrDat);
}
std::vector < ListSerializer::dataType > Bullet::getAttributeTypes() {
	std::vector < ListSerializer::dataType > attrtypes = { ListSerializer::FLOAT, ListSerializer::FLOAT,ListSerializer::FLOAT };
	return(attrtypes);
};

