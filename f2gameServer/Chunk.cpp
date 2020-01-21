#include <Box2D/Box2D.h>
#include <vector>
#include <map>
#include "clipper.hpp"

#include "settings.h"
#include "Chunk.h"

#include "gameObjectData.h"

#include <stdlib.h> 


Chunk::Chunk(int x, int y, b2World& world, b2BodyDef* terrainBodyDef, std::map<Material, ClipperLib::Paths*>& wholeMaterialShape, signed char locInfo) :blocks(), chunkx(x), chunky(y) {
	if (locInfo == 0) {

		ClipperLib::Paths toclip(1);
		toclip[0] << ClipperLib::IntPoint(clippperPrecision * (10 * x), clippperPrecision * (10 * y)) << ClipperLib::IntPoint(clippperPrecision * (10 * x + 10), clippperPrecision * (10 * y)) << ClipperLib::IntPoint(clippperPrecision * (10 * x + 10), clippperPrecision * (10 * y + 10)) << ClipperLib::IntPoint(clippperPrecision * (10 * x), clippperPrecision * (10 * y + 10));

		for (Material matType : materials)
		{
			materialShapes[matType] = new ClipperLib::Paths();
			ClipperLib::Clipper c;
			c.AddPaths(*wholeMaterialShape[matType], ClipperLib::ptSubject, true);
			c.AddPaths(toclip, ClipperLib::ptClip, true);
			c.Execute(ClipperLib::ctIntersection, *materialShapes[matType], ClipperLib::pftNonZero, ClipperLib::pftNonZero);
		}
	}
	else if (locInfo == 1) {


		for (Material matType : materials)
		{
			materialShapes[matType] = new ClipperLib::Paths(0);
		}
	}
	else if (locInfo == -1) {


		for (Material matType : materials)
		{
			if (matType == STONE) {
				materialShapes[STONE] = new ClipperLib::Paths(1);
				(*materialShapes[STONE])[0] << ClipperLib::IntPoint(clippperPrecision * (10 * x), clippperPrecision * (10 * y)) << ClipperLib::IntPoint(clippperPrecision * (10 * x + 10), clippperPrecision * (10 * y)) << ClipperLib::IntPoint(clippperPrecision * (10 * x + 10), clippperPrecision * (10 * y + 10)) << ClipperLib::IntPoint(clippperPrecision * (10 * x), clippperPrecision * (10 * y + 10));
			}
			else {
				materialShapes[matType] = new ClipperLib::Paths(0);
			}
		}
	}


	terrainBodyDef->position.Set(0, 0);

	physBody = world.CreateBody(terrainBodyDef);
	physBody->SetUserData(new gameObjectDat(gameObjectType::CHUNKTYPE, this));
	createBody();
	for (size_t i = 0; i < 10; i++)
	{
		//blocks.push_back(new Block(static_cast<Block::BlockType>(rand() % 8+1) , 10 * x+ rand() % 10, 10 * y + rand() % 10, world));

	}
	/*
	if (chunkInfo==-1) {
	}
	else if (chunkInfo == 0) {
		ClipperLib::Paths whole(1);
		whole[0] << ClipperLib::IntPoint(clippperPrecision * (10 * x),0) << ClipperLib::IntPoint(clippperPrecision * (10 * x + 10), 0) ;
		for (int i =(10 * terrainPrecision-1+1); i >=0; i--)
		{
			//std::cout << "pt " << i << "\n";
			whole[0] << ClipperLib::IntPoint(clippperPrecision * (10 * x+i/ static_cast<float>(terrainPrecision)), clippperPrecision*(terrainHere[i]));

		}

		shape = new ClipperLib::Paths();

		ClipperLib::Clipper c;
		c.AddPaths(whole, ClipperLib::ptSubject, true);
		c.AddPaths(toclip, ClipperLib::ptClip, true);
		c.Execute(ClipperLib::ctIntersection, *shape, ClipperLib::pftNonZero, ClipperLib::pftNonZero);


	}
	else if (chunkInfo == 1) {
		shape = new ClipperLib::Paths(1);
		(*shape)[0]<< ClipperLib::IntPoint(clippperPrecision*(10*x), clippperPrecision * (10 * y)) << ClipperLib::IntPoint(clippperPrecision*(10 * x+10), clippperPrecision * (10 * y)) << ClipperLib::IntPoint(clippperPrecision*(10 * x+10), clippperPrecision * (10 * y+10)) << ClipperLib::IntPoint(clippperPrecision * (10 * x), clippperPrecision * (10 * y+10));
	}
	physBody = world.CreateBody(terrainBodyDef);
	createBody();
	*/

}

void Chunk::createBody() {//Optimise later

	physBody->GetFixtureList();
	while (physBody->GetFixtureList()) {
		physBody->DestroyFixture(physBody->GetFixtureList());
	}
	ClipperLib::Paths tosum(0);
	for (Material matType : materials)
	{
		ClipperLib::Clipper c;
		c.AddPaths(tosum, ClipperLib::ptSubject, true);
		c.AddPaths(*materialShapes[matType], ClipperLib::ptClip, true);
		c.Execute(ClipperLib::ctUnion, tosum, ClipperLib::pftNonZero, ClipperLib::pftNonZero);

	}

	for (ClipperLib::Path pth : tosum)
	{


		b2Vec2* pat = new b2Vec2[pth.size()];
		for (std::size_t i = 0; i < pth.size(); i++) {

			pat[i].Set(pth[i].X / static_cast<float>(clippperPrecision), pth[i].Y / static_cast<float>(clippperPrecision));
		}

		b2ChainShape chain;
		chain.CreateLoop(pat, pth.size());

		b2FixtureDef fixtureDef;
		fixtureDef.density = 1.0f;
		fixtureDef.friction = 0.3f;
		fixtureDef.shape = &chain;
		fixtureDef.restitution = 0;
		physBody->CreateFixture(&fixtureDef);
		delete[] pat;

	}

}



const std::vector< Chunk::Material> Chunk::materials = { Chunk::GRASS,  Chunk::DIRT,  Chunk::STONE };


VisualChunk::VisualChunk(Chunk& orig):chunkx(orig.chunkx), chunky(orig.chunky) {

	for (auto const& mat : orig.materialShapes)
	{
		materialShapes[mat.first] = new ClipperLib::Paths(*mat.second);
	}
}


ClientChunkPiece::ClientChunkPiece(ClipperLib::Path* tc, unsigned char mT, unsigned int cx, unsigned int cy,unsigned char ib) : chunkx(cx), chunky(cy) {
	if (tc != NULL) {

		isWhole = ClipperLib::Orientation(*tc);
		isBackground = ib;
		materialType = mT;
		for (size_t i = 0; i < (*tc).size(); i++)
		{
			asVec.push_back((*tc)[i].X / static_cast<float>(clippperPrecision));
			asVec.push_back((*tc)[i].Y / static_cast<float>(clippperPrecision));

		}
	}
}
void** ClientChunkPiece::getAttributes() {
	void** attrDat = new void* [6];
	attrDat[0] = &isWhole;
	attrDat[1] = &materialType;
	attrDat[2] = &asVec;
	attrDat[3] = &chunkx;
	attrDat[4] = &chunky;
	attrDat[5] = &isBackground;


	return(attrDat);
}
std::vector < ListSerializer::dataType > ClientChunkPiece::getAttributeTypes() {
	std::vector < ListSerializer::dataType > attrtypes = { ListSerializer::BYTE,ListSerializer::BYTE, ListSerializer::XYPAIRS, ListSerializer::UINT, ListSerializer::UINT ,ListSerializer::BYTE };
	return(attrtypes);
};

