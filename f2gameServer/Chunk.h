#pragma once
#include <Box2D/Box2D.h>
#include <vector>
#include <map>
#include "clipper.hpp"

#include "settings.h"
#include "ListSerializer.h"
#include "Block.h"

class Chunk {
public:
	enum Material {
		GRASS,
		DIRT,
		STONE,
	};
	static const std::vector<Material> materials;
	const int chunkx, chunky;
	b2Body* physBody;
	std::map<Material, ClipperLib::Paths*> materialShapes;

	std::vector<Block*> blocks;

	Chunk(int x, int y, b2World& world, b2BodyDef* terrainBodyDef, std::map<Material, ClipperLib::Paths*>& wholeMaterialShape, signed char locInfo);
	void createBody();
};

class VisualChunk {
public:
	const int chunkx, chunky;
	std::map<Chunk::Material, ClipperLib::Paths*> materialShapes;
	VisualChunk(Chunk &orig);
};

class ClientChunkPiece :Serializable {
public:
	unsigned char isBackground;

	unsigned char isWhole;
	unsigned char materialType;
	std::vector<float> asVec;
	unsigned int chunkx = 0;
	unsigned int chunky = 0;

	ClientChunkPiece(ClipperLib::Path* tc, unsigned char mT, unsigned int cx, unsigned int cy,unsigned char ib);
	void** getAttributes();
	static std::vector < ListSerializer::dataType > getAttributeTypes();
};
