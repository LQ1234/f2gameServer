#include "Block.h"

const std::vector< Block::BlockType> Block::blockTypes = { BLOCK_TYPES_PP };
const std::set< Block::BlockType> Block::hitableBlockTypes = { BLOCK_HITABLE_PP };

Block::Block(BlockType bt, int xx, int yy, b2World& world)
{
	x = xx;
	y = yy;
	thisBlockType = bt;

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(xx+.5, yy+.5);
	physBody = world.CreateBody(&bodyDef);

	b2PolygonShape staticBox;
	staticBox.SetAsBox(.5,.5);

	b2FixtureDef fixtureDef;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = .3f;
	fixtureDef.shape = &staticBox;
	fixtureDef.restitution = 0;
	if(!Block::hitableBlockTypes.count(bt))fixtureDef.filter.maskBits = 0;
	physBody->CreateFixture(&fixtureDef);
	gameObjectDat* x = new gameObjectDat(gameObjectType::BLOCKTYPE, this);
	physBody->SetUserData(x);

	wrd = &world;

}
Block::~Block() {
	delete physBody->GetUserData();
	physBody->SetUserData(NULL);

	wrd->DestroyBody(physBody);
	physBody = NULL;
}
void** Block::getAttributes() {
	void** attrDat = new void* [5];
	attrDat[0] = &x;
	attrDat[1] = &y;
	attrDat[2] = &thisBlockType;

	return(attrDat);
}
std::vector < ListSerializer::dataType > Block::getAttributeTypes() {
	std::vector < ListSerializer::dataType > attrtypes = { ListSerializer::INTR,ListSerializer::INTR, ListSerializer::BYTE};
	return(attrtypes);
};

