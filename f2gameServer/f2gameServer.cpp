#define ASIO_STANDALONE
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>

#include <Box2D/Box2D.h>

#include <set>
#include <iostream>
#include <vector>
#include <sstream>
#include <utility> 
#include <string> 
#include <stdlib.h>
#include <time.h> 
#include <chrono>
#include <thread>
#include <algorithm>
#define _USE_MATH_DEFINES

#include <math.h>

#include "ListSerializer.h"

#include "SimplexNoise.h"

#include "clipper.hpp"


typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using websocketpp::lib::thread;
using websocketpp::lib::mutex;
using websocketpp::lib::lock_guard;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;

constexpr  int viewportX = 2;//chunks
constexpr  int viewportY = 2;//chunks

constexpr unsigned int worldxChunks = 40;//set to 100 later
constexpr unsigned int worldyChunks = 20;
constexpr unsigned int terrainPrecision = 5;//per block
constexpr unsigned int clippperPrecision = 20;//per block
constexpr float noiseRandomSeperation = 10 * 100;
struct objecthitboxinfo {
	static const b2Vec2 player;
};
const b2Vec2 objecthitboxinfo::player = b2Vec2(1, 1);

struct TerrainGeneration {

	static std::vector<float> getBiomePercentages(float x) {
		
		std::vector<float> biomePercentages(3);
		biomePercentages[0] = (SimplexNoise::noise(x * .01 + noiseRandomSeperation * 1) + 1.1);	//PLAINS
		biomePercentages[1] = (SimplexNoise::noise(x * .01 + noiseRandomSeperation * 2) + 1.1);	//MOUNTAINS
		biomePercentages[2] = (SimplexNoise::noise(x * .01 + noiseRandomSeperation * 3) + 1.1); //OCEANS
		float avr = 0;

		for (float ts : biomePercentages)
			avr += ts;

		avr /= biomePercentages.size();

		for (size_t i = 0; i < biomePercentages.size(); i++)
		{
			biomePercentages[i] /= avr;
		}

		for (size_t i = 0; i < biomePercentages.size(); i++)
		{
			biomePercentages[i] = std::pow(biomePercentages[i], 100);
		}

		float total = 0;

		for (float ts : biomePercentages)
			total += ts;
		//std::cout << "at:"<<x << "\n";

		for (size_t i = 0; i < biomePercentages.size(); i++)
		{
			biomePercentages[i] /= total;
			//std::cout << biomePercentages[i] << "\n";
		}

		//return(biomePercentages);
		
		std::vector<float> ret = {1,0,0};
		return(ret);

	}
	//PLAINS
	static float getStoneLevelOfPlains(float x) {
		float val= SimplexNoise::noise(x * .01 + noiseRandomSeperation * 15) * 3 + worldyChunks * 10 / 2-10;
		return(val);
	}
	static float getDirtLevelOfPlains(float x) {
		float val = getStoneLevelOfPlains(x);
		val += SimplexNoise::noise(x * .02 + noiseRandomSeperation * 5) * .5 + 7;
		val += SimplexNoise::noise(x + noiseRandomSeperation * 6) * .1;

		return(val);
	}
	static float getGrassLevelOfPlains(float x) {
		float val = getDirtLevelOfPlains(x);
		val += .3;
		return(val);
	}
	//MOUNTIAINS

	static float getStoneLevelOfMountains(float x) {
		float val = SimplexNoise::noise(x * .01) * 70 + worldyChunks * 10 / 2+30;
		val += SimplexNoise::noise(x*.03 + noiseRandomSeperation * 7) * .1;

		return(val);
	}
	static float getDirtLevelOfMountains(float x) {
		float val = getStoneLevelOfMountains(x);
		val += 0;
		return(val);
	}
	static float getGrassLevelOfMountains(float x) {
		float val = getDirtLevelOfMountains(x);
		val += 0;
		return(val);
	}
	//OCEANS
	static float getStoneLevelOfOceans(float x) {
		float val = SimplexNoise::noise(x * .005) * 3 + worldyChunks * 10 / 2-30;

		return(val);
	}
	static float getDirtLevelOfOceans(float x) {
		float val = getStoneLevelOfOceans(x);
		val += 2;
		return(val);
	}
	static float getGrassLevelOfOceans(float x) {
		float val = getDirtLevelOfOceans(x);
		val += 0;
		return(val);
	}
	//GENERAL
	static float getStoneLevel(float x) {
		std::vector<float> bvals = getBiomePercentages(x);
		float val=(getStoneLevelOfPlains(x)*bvals[0]+ getStoneLevelOfMountains(x) * bvals[1] + getStoneLevelOfOceans(x) * bvals[2]);
		//std::cout << "stone" << val<<"\n";
		return(val);
	}
	static float getDirtLevel(float x) {
		std::vector<float> bvals = getBiomePercentages(x);
		float val = (getDirtLevelOfPlains(x) * bvals[0] + getDirtLevelOfMountains(x) * bvals[1] + getDirtLevelOfOceans(x) * bvals[2]);
		return(val);
	}
	static float getGrassLevel(float x) {
		std::vector<float> bvals = getBiomePercentages(x);
		float val = (getGrassLevelOfPlains(x) * bvals[0] + getGrassLevelOfMountains(x) * bvals[1] + getGrassLevelOfOceans(x) * bvals[2]);
		return(val);
	}
};

enum ws_event_type {
	CONNECT,
	DISCONNECT,
	MESSAGE
};

struct ws_event {
	ws_event(ws_event_type t, connection_hdl h) : type(t), hdl(h) {}
	ws_event(ws_event_type t, connection_hdl h, server::message_ptr m)
		: type(t), hdl(h), msg(m) {}

	ws_event_type type;
	websocketpp::connection_hdl hdl;
	server::message_ptr msg;
};


enum gameObjectType {
	CHUNKTYPE,
	PLAYERTYPE,
};
struct gameObjectDat {
	gameObjectType type;
	void* obj;//note that when ~gameObjectDat, delete obj is not called
	gameObjectDat(gameObjectType t, void* o) : type(t), obj(o)
	{
	};

};
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



	Chunk(int x, int y, b2World& world, b2BodyDef* terrainBodyDef, std::map<Material, ClipperLib::Paths*>& wholeMaterialShape, signed char locInfo) : chunkx(x), chunky(y) {
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
		physBody->SetUserData(new gameObjectDat(CHUNKTYPE, this));
		createBody();
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
	void createBody() {//Optimise later

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


};
const std::vector< Chunk::Material> Chunk::materials = { Chunk::GRASS,  Chunk::DIRT,  Chunk::STONE };

class ClientChunkPiece :Serializable {
public:
	unsigned char isWhole;
	unsigned char materialType;
	std::vector<float> asVec;
	unsigned int chunkx = 0;
	unsigned int chunky = 0;

	ClientChunkPiece(ClipperLib::Path* tc, unsigned char mT, unsigned int cx, unsigned int cy) : chunkx(cx), chunky(cy) {
		if (tc != NULL) {

			isWhole = ClipperLib::Orientation(*tc);

			materialType = mT;
			for (size_t i = 0; i < (*tc).size(); i++)
			{
				asVec.push_back((*tc)[i].X / static_cast<float>(clippperPrecision));
				asVec.push_back((*tc)[i].Y / static_cast<float>(clippperPrecision));

			}
		}
	}
	void** getAttributes() {
		void** attrDat = new void* [5];
		attrDat[0] = &isWhole;
		attrDat[1] = &materialType;
		attrDat[2] = &asVec;
		attrDat[3] = &chunkx;
		attrDat[4] = &chunky;


		return(attrDat);
	}
	std::vector < ListSerializer::dataType > getAttributeTypes() {
		std::vector < ListSerializer::dataType > attrtypes = { ListSerializer::BYTE,ListSerializer::BYTE, ListSerializer::XYPAIRS, ListSerializer::UINT, ListSerializer::UINT };
		return(attrtypes);
	};
};


class Player :Serializable {
public:
	b2Body* physBody;
	b2World* wrd;
	websocketpp::connection_hdl hdl;
	std::string name;
	float health = 100;
	std::set<Chunk*> viewChunks;
	float x = 0;
	float y = 0;
	float rot = 0;



	bool buttons = new bool[4];
	Player(b2World& world, std::string nme, websocketpp::connection_hdl conhdl) {
		hdl = conhdl;
		name = nme;
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.bullet = true;
		bodyDef.position.Set(worldxChunks * 10 / 2, TerrainGeneration::getGrassLevel(worldxChunks * 10 / 2)+5);
		physBody = world.CreateBody(&bodyDef);

		b2PolygonShape dynamicBox;
		dynamicBox.SetAsBox(objecthitboxinfo::player.x / 2, objecthitboxinfo::player.y / 2);

		b2FixtureDef fixtureDef;
		fixtureDef.density = 1.0f;
		fixtureDef.friction = 1.f;
		fixtureDef.shape = &dynamicBox;
		fixtureDef.restitution = 0;

		physBody->CreateFixture(&fixtureDef);
		gameObjectDat* x = new gameObjectDat(PLAYERTYPE, this);
		physBody->SetUserData(x);
		wrd = &world;

	}
	~Player() {
		delete physBody->GetUserData();
		physBody->SetUserData(NULL);

		wrd->DestroyBody(physBody);
		physBody = NULL;
	}
	void updatePosition() {
		b2Vec2 ps = physBody->GetPosition();
		x = ps.x;
		y = ps.y;
		rot = physBody->GetAngle();
	}
	void** getAttributes() {
		void** attrDat = new void* [5];
		updatePosition();
		attrDat[0] = &x;
		attrDat[1] = &y;
		attrDat[2] = &rot;
		attrDat[3] = &health;
		attrDat[4] = &name;

		return(attrDat);
	}
	std::vector < ListSerializer::dataType > getAttributeTypes() {
		std::vector < ListSerializer::dataType > attrtypes = { ListSerializer::FLOAT, ListSerializer::FLOAT,ListSerializer::FLOAT, ListSerializer::FLOAT,ListSerializer::STRING };
		return(attrtypes);
	};
};
class viewportObjectCallback : public b2QueryCallback {
public:
	std::set<b2Body*> fvp;//found in viewport

	bool ReportFixture(b2Fixture* fixture) {
		fvp.insert(fixture->GetBody());
		return true;//keep going to find all fixtures in the query area
	}
};

class game_server {
public:


	void game_loop() {


		b2Vec2 phys_gravity(0.0f, -10.0f);
		b2World phys_world(phys_gravity);

		b2BodyDef terrainBodyDef;
		terrainBodyDef.type = b2_staticBody;
		//chunk x,y goes from x,y to x+10,y+10
		Chunk* chunk_array[worldyChunks][worldxChunks];
		{

			float terrainMins[worldxChunks];
			for (size_t i = 0; i < worldxChunks; i++)
			{
				terrainMins[i] = worldyChunks * 10;
			}
			float terrainMaxs[worldxChunks];
			auto minmax = [&terrainMaxs, &terrainMins](float x, float y)//both in blocks
			{
				int indx = x / 10;
				terrainMaxs[indx] = std::max(terrainMaxs[indx], y);
				terrainMins[indx] = std::min(terrainMins[indx], y);
			};
			std::cout << "worldgen\n";
			std::map<Chunk::Material, ClipperLib::Paths*> wholeMaterialShape;

			ClipperLib::Paths* stonePath = new ClipperLib::Paths(1);

			(*stonePath)[0] << ClipperLib::IntPoint(0, 0) << ClipperLib::IntPoint(clippperPrecision * (worldxChunks * 10), 0);

			for (int i = (worldxChunks * 10 * terrainPrecision); i >= 0; i--)
			{
				float val = TerrainGeneration::getStoneLevel(static_cast<float>(i) / terrainPrecision);
				minmax(i / terrainPrecision, val);
				(*stonePath)[0] << ClipperLib::IntPoint(clippperPrecision * (static_cast<float>(i) / terrainPrecision), clippperPrecision * (val));

			}
			wholeMaterialShape[Chunk::STONE] = stonePath;

			ClipperLib::Paths* dirtPath = new ClipperLib::Paths(1);

			for (int i = 0; i <= (worldxChunks * 10 * terrainPrecision); i++)
			{
				//std::cout << "pt " << i << "\n";
				float val = TerrainGeneration::getStoneLevel(static_cast<float>(i) / terrainPrecision);
				minmax(i / terrainPrecision, val);
				(*dirtPath)[0] << ClipperLib::IntPoint(clippperPrecision * (static_cast<float>(i) / terrainPrecision), clippperPrecision * (val));

			}
			for (int i = (worldxChunks * 10 * terrainPrecision); i >= 0; i--)
			{
				//std::cout << "pt " << i << "\n";
				float val = TerrainGeneration::getDirtLevel(static_cast<float>(i) / terrainPrecision);
				minmax(i / terrainPrecision, val);
				(*dirtPath)[0] << ClipperLib::IntPoint(clippperPrecision * (static_cast<float>(i) / terrainPrecision), clippperPrecision * (val));

			}
			wholeMaterialShape[Chunk::DIRT] = dirtPath;

			ClipperLib::Paths* grassPath = new ClipperLib::Paths(1);

			for (int i = 0; i <= (worldxChunks * 10 * terrainPrecision); i++)
			{
				//std::cout << "pt " << i << "\n";
				float val = TerrainGeneration::getDirtLevel(static_cast<float>(i) / terrainPrecision);
				minmax(i / terrainPrecision, val);
				(*grassPath)[0] << ClipperLib::IntPoint(clippperPrecision * (static_cast<float>(i) / terrainPrecision), clippperPrecision * (val));

			}
			for (int i = (worldxChunks * 10 * terrainPrecision); i >= 0; i--)
			{
				//std::cout << "pt " << i << "\n";
				float val = TerrainGeneration::getGrassLevel(static_cast<float>(i) / terrainPrecision);
				minmax(i / terrainPrecision, val);
				(*grassPath)[0] << ClipperLib::IntPoint(clippperPrecision * (static_cast<float>(i) / terrainPrecision), clippperPrecision * (val));

			}
			wholeMaterialShape[Chunk::GRASS] = grassPath;

			for (size_t x = 0; x < worldxChunks; x++)
			{
				for (size_t y = 0; y < worldyChunks; y++)
				{
					float thischunkmin = 10 * y;
					float thischunkmax = 10 * y + 10;
					if (thischunkmin > terrainMaxs[x]) {
						chunk_array[y][x] = new Chunk(x, y, phys_world, &terrainBodyDef, wholeMaterialShape, 1);

					}
					else if (thischunkmax < terrainMins[x]) {
						chunk_array[y][x] = new Chunk(x, y, phys_world, &terrainBodyDef, wholeMaterialShape, -1);

					}
					else {
						chunk_array[y][x] = new Chunk(x, y, phys_world, &terrainBodyDef, wholeMaterialShape, 0);

					}
					//std::cout << "at "<<x<<", "<<y<<"\n";



				}
				std::cout << "at " << x << "\n";

			}
			std::cout << "worldgen done";


		}


		while (1) {

			//EVENTS
			while (event_queue.size()) {
				unique_lock<mutex> lock(event_queue_lock);
				ws_event e = event_queue.front();
				event_queue.pop();
				lock.unlock();

				if (e.type == CONNECT) {
					connections.insert(e.hdl);
					players[e.hdl] = new Player(phys_world, "test", e.hdl);
				}
				else if (e.type == DISCONNECT) {
					connections.erase(e.hdl);
					delete players[e.hdl];
					players.erase(e.hdl);
				}
				else if (e.type == MESSAGE) {







					try {
						//ws_server.send(e.hdl, ls.serialize(), websocketpp::frame::opcode::binary);
					}
					catch (websocketpp::exception const& e) {
						std::cout << "ERROR:" << e.what() << std::endl;

					}
					/*
					con_list::iterator it;


					//lock_guard<mutex> guard(connection_lock);

					std::cout << e.msg;

					//con_list::iterator it;
					//for (it = m_connections.begin(); it != m_connections.end(); ++it) {

					std::string srd = e.msg->get_payload();

					const int bytelen = srd.size();

					const char* datchar = srd.data();




					float* datdbl = new float[bytelen / 4];
					std::memcpy(datdbl, datchar, bytelen);
					for (size_t i = 0; i < bytelen / 4; i++)
					{

						datdbl[i] *= datdbl[i];

					}
					std::cout << datdbl << "\n";
					ws_server.send(e.hdl, datdbl, bytelen, websocketpp::frame::opcode::binary);
					*/



				}

			}
			//PLAYER PHYSICS
			for (auto const& pr : players)
			{

				Player* thisplayer = pr.second;
				float currangle = -thisplayer->physBody->GetAngle();//0 is desired angle
				currangle = std::fmod(currangle, M_PI*2);
				if (currangle > M_PI)currangle -= 2 * M_PI;
				currangle *= std::abs(currangle) * 12;
				currangle -= thisplayer->physBody->GetAngularVelocity();
				thisplayer->physBody->ApplyTorque(currangle,true);

			}
			//PLAYER INFO UPDATE


			for (auto const& pr : players)
			{

				Player* thisplayer = pr.second;
				//std::cout << "xy:"<<thisplayer->x<<","<< thisplayer->y <<"\n";
				ListSerializer ls;
				//PLAYER PLAYER UPDATE

				{
					{
						ls.setClassAttributes(thisplayer->getAttributeTypes());//just because its handy, no other reasons
					}
					viewportObjectCallback cb;
					b2AABB aabb;
					thisplayer->updatePosition();
					b2Vec2 xy1(thisplayer->x - 10 * viewportX, thisplayer->y - 10 * viewportY);
					b2Vec2 xy2(thisplayer->x + 10 * viewportX, thisplayer->y + 10 * viewportY);
					aabb.lowerBound = xy1;
					aabb.upperBound = xy2;
					phys_world.QueryAABB(&cb, aabb);

					for (b2Body* bod : cb.fvp) {

						gameObjectDat* dat = static_cast<gameObjectDat*>(bod->GetUserData());
						if (dat->type == CHUNKTYPE) {
							//dont do anything, updated later
						}
						else if (dat->type == PLAYERTYPE) {
							ls.addObjectAttributes((static_cast<Player*>(dat->obj)->getAttributes()));
						}
					}
				}

				//PLAYER CHUNK UPDATE
				{
					{
						ClientChunkPiece n(NULL, 0, 0, 0);
						ls.setClassAttributes(n.getAttributeTypes());
					}

					const int cxchunk = (pr.second->x) / 10;
					const int cychunk = (pr.second->y) / 10;
					std::set<Chunk*> newViewChunks;

					for (int ychunk = cychunk - viewportY; ychunk <= cychunk + viewportY; ychunk++) {


						for (int xchunk = cxchunk - viewportX; xchunk <= cxchunk + viewportX; xchunk++) {

							if (xchunk < 0 || ychunk < 0 || xchunk >= worldxChunks || ychunk >= worldyChunks) {
								continue;
							}
							Chunk* chunkhere = chunk_array[ychunk][xchunk];
							newViewChunks.insert(chunkhere);
							for (b2Fixture* f = chunkhere->physBody->GetFixtureList(); f; f = f->GetNext())
							{
								b2ChainShape* cs = (b2ChainShape*)f->GetShape();

								for (int i = 0; i < cs->GetChildCount(); i++) {
									b2EdgeShape n;
									cs->GetChildEdge(&n, i);
								}

							}
							if (updatedChunks.count(chunkhere) || (!(thisplayer->viewChunks.count(chunkhere)))) {
								for (auto const& x : chunkhere->materialShapes)
								{
									Chunk::Material mat = x.first;
									ClipperLib::Paths* pats = x.second;

									for (ClipperLib::Path& pat : (*pats)) {
										ClientChunkPiece ccp(&pat, mat, xchunk, ychunk);
										ls.addObjectAttributes(ccp.getAttributes());

									}
								}
							}

						}
					}
					thisplayer->viewChunks = newViewChunks;

				}

				//SEND TO PLAYER (WS)
				//std::cout<<"'" << ls.serialize() << "'\n";
				try {
					ws_server.send(thisplayer->hdl, ls.serialize(), websocketpp::frame::opcode::binary);
				}
				catch (websocketpp::exception const& e) {
					std::cout << "ERROR:" << e.what() << std::endl;

				}
			}
			updatedChunks.clear();
			//PHYSICS
			//std::cout << phys_world.GetBodyCount() << "\n";



			phys_world.Step(1.0 / 60.0, 8, 3, 3);

			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
		}
	}
	game_server() {
		ws_server.init_asio();
		ws_server.clear_access_channels(websocketpp::log::alevel::all);
		ws_server.clear_error_channels(websocketpp::log::elevel::all);

		ws_server.set_open_handler(bind(&game_server::on_open, this, ::_1));
		ws_server.set_close_handler(bind(&game_server::on_close, this, ::_1));
		ws_server.set_message_handler(bind(&game_server::on_message, this, ::_1, ::_2));
	}

	void ws_server_start(uint16_t port) {
		ws_server.listen(port);

		ws_server.start_accept();

		try {
			ws_server.run();
		}
		catch (const std::exception& e) {
			std::cout << e.what() << std::endl;
		}
	}

	void on_open(connection_hdl hdl) {
		{
			lock_guard<mutex> guard(event_queue_lock);
			event_queue.push(ws_event(CONNECT, hdl));
		}
	}

	void on_close(connection_hdl hdl) {
		{
			lock_guard<mutex> guard(event_queue_lock);
			event_queue.push(ws_event(DISCONNECT, hdl));
		}
	}

	void on_message(connection_hdl hdl, server::message_ptr msg) {
		{
			lock_guard<mutex> guard(event_queue_lock);
			event_queue.push(ws_event(MESSAGE, hdl, msg));
		}
	}

private:
	typedef std::set<connection_hdl, std::owner_less<connection_hdl> > con_list;
	typedef std::map<connection_hdl, Player*, std::owner_less<connection_hdl>> player_map;

	server ws_server;
	con_list connections;
	player_map players;
	std::queue<ws_event> event_queue;
	std::set<Chunk*> updatedChunks;
	mutex event_queue_lock;
};

int main() {
	try {
		game_server server_instance;

		// Start a thread to run the processing loop
		thread t(bind(&game_server::game_loop, &server_instance));

		// Run the asio loop with the main thread
		server_instance.ws_server_start(9002);

		t.join();

	}
	catch (websocketpp::exception const& e) {
		std::cout << e.what() << std::endl;
	}

}
