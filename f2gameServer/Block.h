#pragma once
#include <vector>
#include "ListSerializer.h"
#include <Box2D/Box2D.h>
#include "gameObjectData.h"
#include <set>


#define BLOCK_TYPES_PP NULLBLOCK, STONEBLOCK, PLANKBLOCK, STONEBRICKBLOCK, LOGVBLOCK, LOGHBLOCK, GLASSBLOCK, TORCHBLOCK, LEAFBLOCK
#define BLOCK_HITABLE_PP STONEBLOCK, PLANKBLOCK, STONEBRICKBLOCK, LOGVBLOCK, LOGHBLOCK, GLASSBLOCK

class Block :Serializable {
public:
	b2Body* physBody;
	b2World* wrd;

	enum BlockType { BLOCK_TYPES_PP };
	static const std::vector<BlockType> blockTypes;
	static const std::set<BlockType> hitableBlockTypes;

	Block(BlockType bt,int x,int y, b2World& world);
	~Block();
	
	BlockType thisBlockType;
	int x, y;

	void** getAttributes();
	static std::vector < ListSerializer::dataType > getAttributeTypes();
};

