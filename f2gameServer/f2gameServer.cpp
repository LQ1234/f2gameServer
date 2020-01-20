#define ASIO_STANDALONE
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>
#include <unordered_map> 

/*
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

*/



#include "Block.h"

#include "settings.h"
#include "TerrainGeneration.h"
#include "gameObjectData.h"
#include "Bullet.h"
#include "Chunk.h"
#include "Item.h"
#include "Player.h"
#include "Potion.h"

#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

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

class viewportObjectCallback : public b2QueryCallback {
public:
	std::vector<b2Body*> fvp[GAMEOBJECTTYPESCOUNT_PP];

	bool ReportFixture(b2Fixture* fixture) {
		fvp[static_cast<int>(static_cast<gameObjectDat*>(fixture->GetBody()->GetUserData())->type)].push_back(fixture->GetBody());
		return(true);
	}
};

auto toDestroyComparator = [](gameObjectDat* a, gameObjectDat* b)->bool {
	return (*a) < (*b);
};

class CollisionListener : public b2ContactListener
{
public:
	std::set<gameObjectDat*, decltype(toDestroyComparator)>* toDestroy;

private:
	
	void BeginContact(b2Contact* contact) {
		Contact(contact,true);
	}
	void EndContact(b2Contact* contact) {
		Contact(contact, false);
	}
	void Contact(b2Contact* contact,bool isBegin) {
		b2Fixture* fixA = contact->GetFixtureA();
		b2Fixture* fixB = contact->GetFixtureB();

		bool isA = fixA->IsSensor();
		bool isB = fixB->IsSensor();

		gameObjectDat* datA = static_cast<gameObjectDat*>(fixA->GetBody()->GetUserData());
		gameObjectDat* datB = static_cast<gameObjectDat*>(fixB->GetBody()->GetUserData());
		if (!datA) return;
		if (!datB) return;

		if (PlayerItemHandler(datA, datB, fixA, fixB, isA, isB, isBegin))return;
		if (PlayerItemHandler(datB, datA, fixB, fixA, isB, isA, isBegin))return;

		if (PlayerEnviromentHandler(datA, datB, fixA, fixB, isA, isB, isBegin))return;
		if (PlayerEnviromentHandler(datB, datA, fixB, fixA, isB, isA, isBegin))return;
	}
	bool PlayerItemHandler(gameObjectDat* datA, gameObjectDat* datB, b2Fixture* fixA , b2Fixture* fixB,bool isASensor, bool isBSensor, bool isBegin) {
		if (datA->type != gameObjectType::ITEMTYPE)return false;
		if (datB->type != gameObjectType::PLAYERTYPE)return false;
		if (isASensor)return false;
		if (!isBSensor)return false;


		Item* itm = static_cast<Item*>(datA->obj);
		Player* plyr = static_cast<Player*>(datB->obj);

		if (plyr->itemCollectionFixture != fixB) return false;
		if (!isBegin)return false;
		
		plyr->items[itm->itemtype] += itm->count;
		toDestroy->insert(new gameObjectDat(gameObjectType::ITEMTYPE, itm));

		return false;
	}
	bool PlayerEnviromentHandler(gameObjectDat* datA, gameObjectDat* datB, b2Fixture* fixA, b2Fixture* fixB, bool isASensor, bool isBSensor, bool isBegin) {
		if (datA->type != gameObjectType::PLAYERTYPE)return false;
		if (datB->type == gameObjectType::ITEMTYPE)return false;

		if (!isASensor)return false;

		Player* plyr = static_cast<Player*>(datA->obj);
		if (fixA == plyr->groundDetectFixture) {
			plyr->onground += isBegin ? 1 : -1;
		}
		else if (fixA == plyr->wallJumpLeftFixture) {
			plyr->wallJumpLeft += isBegin ? 1 : -1;
		}
		else if (fixA == plyr->wallJumpRightFixture) {
			plyr->wallJumpRight += isBegin ? 1 : -1;
		}
		std::cout << "onground " << plyr->onground << " wallJumpLeft " << plyr->wallJumpLeft << " wallJumpRight " << plyr->wallJumpRight << "\n";
		return false;
	}
};


class game_server {
public:


	void game_loop() {


		b2Vec2 phys_gravity(0.0f, -12.0f);
		b2World phys_world(phys_gravity);

		b2BodyDef terrainBodyDef;
		terrainBodyDef.type = b2_staticBody;
		//chunk x,y goes from x,y to x+10,y+10
		Chunk* chunk_array[worldyChunks][worldxChunks];
		auto addBlock = [this, &chunk_array](Block* newblock) {
			updatedBlocks.insert(newblock);
			int cx = newblock->x / 10, cy = newblock->y / 10;
			if (0 <= cx && cx < worldxChunks && 0 <= cy && cy < worldyChunks) {
				Chunk* tc = chunk_array[cy][cx];
				tc->blocks.push_back(newblock);

			}
		};
		auto detectBlock = [this, &chunk_array](int x, int y)->bool {
			int cx = x / 10, cy = y / 10;
			if (0 <= cx && cx < worldxChunks && 0 <= cy && cy < worldyChunks) {
				Chunk* tc = chunk_array[cy][cx];
				for (size_t i = 0; i < tc->blocks.size(); i++)
				{
					if (tc->blocks[i]->x == x && tc->blocks[i]->y == y) {
						return(true);
					}
				}
			}
			return(false);

		};
		auto removeBlock = [this, &chunk_array](int x, int y) {
			int cx = x / 10, cy = y / 10;
			if (0 <= cx && cx < worldxChunks && 0 <= cy && cy < worldyChunks) {
				Chunk* tc = chunk_array[cy][cx];
				for (size_t i = 0; i < tc->blocks.size(); i++)
				{
					if (tc->blocks[i]->x == x && tc->blocks[i]->y == y) {
						Block* tb = tc->blocks[i];
						tb->thisBlockType = Block::NULLBLOCK;
						updatedBlocks.insert(tb);
						tc->blocks.erase(tc->blocks.begin()+i);
						break;
					}
				}
			}
		};
		auto removeMatterChunk = [this, &chunk_array](ClipperLib::Paths& sub, Chunk::Material mat, Chunk* thischk, bool updatePhys) {

			ClipperLib::Clipper c;
			c.AddPaths(*(thischk->materialShapes[mat]), ClipperLib::ptSubject, true);

			c.AddPaths(sub, ClipperLib::ptClip, true);
			ClipperLib::Paths* res = new ClipperLib::Paths();
			c.Execute(ClipperLib::ctDifference, *res, ClipperLib::pftNonZero, ClipperLib::pftNonZero);
			delete thischk->materialShapes[mat];
			thischk->materialShapes[mat] = res;
			updatedChunks.insert(thischk);
			if (updatePhys)thischk->createBody();
		};
		auto removeMatter= [this, &chunk_array, removeMatterChunk](ClipperLib::Paths &toRemove,Chunk::Material matt,int xmin,int ymin,int xmax,int ymax,bool updatePhys) {
			for (int chx = (int)(xmin / clippperPrecision / 10); chx <= (int)(xmax / clippperPrecision / 10); chx++) {
				for (int chy = (int)(ymin / clippperPrecision / 10); chy <= (int)(ymax / clippperPrecision / 10); chy++) {
					if (0 <= chx && chx < worldxChunks && 0 <= chy && chy < worldyChunks) {
						Chunk* thiscthk = chunk_array[chy][chx];
						removeMatterChunk(toRemove, matt, thiscthk, updatePhys);
					}
				}
			}
		};
		auto addMatterChunk = [this, &chunk_array](ClipperLib::Paths& sub, Chunk::Material mat, Chunk* thischk, bool updatePhys) {

			ClipperLib::Clipper c;
			c.AddPaths(*(thischk->materialShapes[mat]), ClipperLib::ptSubject, true);

			c.AddPaths(sub, ClipperLib::ptClip, true);
			ClipperLib::Paths* res = new ClipperLib::Paths();
			c.Execute(ClipperLib::ctUnion, *res, ClipperLib::pftNonZero, ClipperLib::pftNonZero);
			delete thischk->materialShapes[mat];
			thischk->materialShapes[mat] = res;
			updatedChunks.insert(thischk);
			if (updatePhys)thischk->createBody();
		};
		auto addMatter = [this, &chunk_array, addMatterChunk](ClipperLib::Paths& toAdd, Chunk::Material matt, int xmin, int ymin, int xmax, int ymax, bool updatePhys) {
			for (int chx = (int)(xmin / clippperPrecision / 10); chx <= (int)(xmax / clippperPrecision / 10); chx++) {
				for (int chy = (int)(ymin / clippperPrecision / 10); chy <= (int)(ymax / clippperPrecision / 10); chy++) {
					if (0 <= chx && chx < worldxChunks && 0 <= chy && chy < worldyChunks) {
						Chunk* thiscthk = chunk_array[chy][chx];
						addMatterChunk(toAdd, matt, thiscthk, updatePhys);
					}
				}
			}
		};
		auto queryMatterChunk = [this, &chunk_array](ClipperLib::Paths& sub, Chunk::Material mat, Chunk* thischk) -> double {

			ClipperLib::Clipper c;
			c.AddPaths(*(thischk->materialShapes[mat]), ClipperLib::ptSubject, true);

			c.AddPaths(sub, ClipperLib::ptClip, true);
			ClipperLib::Paths res;
			c.Execute(ClipperLib::ctIntersection,res, ClipperLib::pftNonZero, ClipperLib::pftNonZero);

			double t = 0;
			for (int i = 0; i < res.size(); i++)
				t += ClipperLib::Area(res[i]);
			return t;
		};
		auto queryMatter = [this, &chunk_array, queryMatterChunk](ClipperLib::Paths& toAdd, Chunk::Material matt, int xmin, int ymin, int xmax, int ymax)-> double {
			double total = 0;
			for (int chx = (int)(xmin / clippperPrecision / 10); chx <= (int)(xmax / clippperPrecision / 10); chx++) {
				for (int chy = (int)(ymin / clippperPrecision / 10); chy <= (int)(ymax / clippperPrecision / 10); chy++) {
					if (0 <= chx && chx < worldxChunks && 0 <= chy && chy < worldyChunks) {
						Chunk* thiscthk = chunk_array[chy][chx];
						total+=queryMatterChunk(toAdd, matt, thiscthk);
					}
				}
			}
			return(total);
		};
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

		

		std::set<gameObjectDat*, decltype(toDestroyComparator)> toDestroy(toDestroyComparator);

		CollisionListener collisionListener;
		collisionListener.toDestroy = &toDestroy;
		phys_world.SetContactListener(&collisionListener);
		int timetillnextsec = 0;
		int lastlooplengthms = 0;

		while (1) {
			auto t1 = std::chrono::high_resolution_clock::now();

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
					std::string strpl = e.msg->get_payload();
					if (strpl.size() == 13) {
						players[e.hdl]->movementkeyboard[0] = strpl[0] == 1;
						players[e.hdl]->movementkeyboard[1] = strpl[1] == 1;
						players[e.hdl]->movementkeyboard[2] = strpl[2] == 1;
						players[e.hdl]->movementkeyboard[3] = strpl[3] == 1;
						{
							float f;
							*((unsigned char*)(&f) + 0) = strpl[7];
							*((unsigned char*)(&f) + 1) = strpl[6];
							*((unsigned char*)(&f) + 2) = strpl[5];
							*((unsigned char*)(&f) + 3) = strpl[4];
							players[e.hdl]->mouseposition[0] = f;
						}
						{
							float f;
							*((unsigned char*)(&f) + 0) = strpl[7 + 4];
							*((unsigned char*)(&f) + 1) = strpl[6 + 4];
							*((unsigned char*)(&f) + 2) = strpl[5 + 4];
							*((unsigned char*)(&f) + 3) = strpl[4 + 4];
							players[e.hdl]->mouseposition[1] = f;
						}
						players[e.hdl]->mousedown = strpl[12] == 1;
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
				float maxXVol = 6.5;
				float xaccel = .2;

				if (thisplayer->movementkeyboard[0] && (!thisplayer->movementImpulseJumpLast)) {
					if (thisplayer->onground > 0) {
						if ( thisplayer->physBody->GetLinearVelocity().y < 8) {//W
							thisplayer->physBody->ApplyLinearImpulse(b2Vec2(0, 1.3), thisplayer->physBody->GetWorldCenter(), true);
						}
					}
					else {
						if ( thisplayer->wallJumpLeft > 0) {
							thisplayer->physBody->ApplyLinearImpulse(b2Vec2(.9, .9), thisplayer->physBody->GetWorldCenter(), true);
						}
						if ( thisplayer->wallJumpRight > 0) {
							thisplayer->physBody->ApplyLinearImpulse(b2Vec2(-.9, .9), thisplayer->physBody->GetWorldCenter(), true);

						}
					}
				}
				thisplayer->movementImpulseJumpLast = thisplayer->movementkeyboard[0];

				if (thisplayer->movementkeyboard[1] && thisplayer->physBody->GetLinearVelocity().x > -maxXVol) {//A
					thisplayer->physBody->ApplyLinearImpulse(b2Vec2(-xaccel, 0), thisplayer->physBody->GetWorldCenter(), true);
				}
				if (thisplayer->movementkeyboard[3] && thisplayer->physBody->GetLinearVelocity().x < maxXVol) {//D
					thisplayer->physBody->ApplyLinearImpulse(b2Vec2(xaccel, 0), thisplayer->physBody->GetWorldCenter(), true);
				}


				float currangle = -thisplayer->physBody->GetAngle();//0 is desired angle
				currangle = std::fmod(currangle, M_PI * 2);
				if (currangle > M_PI)currangle -= 2 * M_PI;
				currangle *= std::abs(currangle) * 15;
				currangle -= thisplayer->physBody->GetAngularVelocity();
				thisplayer->physBody->ApplyTorque(currangle, true);


				if (thisplayer->mousedown) {
					if(detectBlock(thisplayer->mouseposition[0], thisplayer->mouseposition[1])){
						removeBlock(thisplayer->mouseposition[0], thisplayer->mouseposition[1]);
					}
					else {
						//new Item(phys_world,Item::DIRTPIECE,1, thisplayer->mouseposition[0], thisplayer->mouseposition[1]);

						ClipperLib::Paths sub(1);
						for (float i = 0; i < M_PI * 2; i += .5) {
							sub[0] << ClipperLib::IntPoint((cos(i) + thisplayer->mouseposition[0]) * clippperPrecision, (sin(i) + thisplayer->mouseposition[1]) * clippperPrecision);
						}
						int minx = (-1 + thisplayer->mouseposition[0]) * clippperPrecision,
							miny = (-1 + thisplayer->mouseposition[1]) * clippperPrecision,
							maxx = (1 + thisplayer->mouseposition[0]) * clippperPrecision,
							maxy = (1 + thisplayer->mouseposition[1]) * clippperPrecision;
						for (Chunk::Material mat : Chunk::materials) {

							double amt = queryMatter(sub, mat, minx, miny, maxx, maxy);
							amt *= 20;//1000  peice per block
							amt /= clippperPrecision * clippperPrecision;
							Item::ItemType itmtype;
							switch (mat) {
								case Chunk::GRASS: itmtype = Item::GRASSPIECE; break;
								case Chunk::DIRT:itmtype = Item::DIRTPIECE; break;
								case Chunk::STONE: itmtype = Item::STONEPIECE; break;
							}

							//thisplayer->items[itmtype] += amt;

							
							int spawnitemcount = std::min(10, (int)(amt / 5));
							for (size_t i = 0; i < spawnitemcount; i++)
							{
								float randDir = (rand() % static_cast<int>(M_PI * 1000)) / 500.0;
								float randDist = (rand() % 1000) / 1000.0;
								float randxc = cos(randDir) * randDist;
								float randyc = sin(randDir) * randDist;
								new Item(phys_world, itmtype, amt / spawnitemcount, thisplayer->mouseposition[0] + randxc, thisplayer->mouseposition[1] + randyc);
							}
							
						}

						removeMatter(sub, Chunk::GRASS, minx, miny, maxx, maxy, false);
						removeMatter(sub, Chunk::DIRT, minx, miny, maxx, maxy, false);
						removeMatter(sub, Chunk::STONE, minx, miny, maxx, maxy, true);
						//addMatter(sub, Chunk::STONE, minx, miny, maxx, maxy, true);
					}
				}
				//if(thisplayer->mousedown&&!detectBlock(thisplayer->mouseposition[0], thisplayer->mouseposition[1]))addBlock(new Block(Block::STONEBLOCK,thisplayer->mouseposition[0], thisplayer->mouseposition[1],phys_world));

			}

			//PLAYER INFO UPDATE
			/*
			Send Order:

			ClientThisPlayer
			Player
			Item
			Bullet
			Potion
			ClientChunkPiece
			Block
			*/

			for (auto const& pr : players)
			{

				Player* thisplayer = pr.second;
				ListSerializer ls;
				//PLAYER SELF UPDATE
				{
					ls.setClassAttributes(ClientThisPlayer::getAttributeTypes());
					ClientThisPlayer tp(thisplayer);
					ls.addObjectAttributes(tp.getAttributes());
				}

				//PLAYER OBJECT UPDATE

				{

					viewportObjectCallback cb;
					b2AABB aabb;
					thisplayer->updatePosition();
					b2Vec2 xy1(thisplayer->x - 10 * viewportX, thisplayer->y - 10 * viewportYMinus);
					b2Vec2 xy2(thisplayer->x + 10 * viewportX, thisplayer->y + 10 * viewportYPlus);
					aabb.lowerBound = xy1;
					aabb.upperBound = xy2;
					phys_world.QueryAABB(&cb, aabb);

					ls.setClassAttributes(Player::getAttributeTypes());

					for (b2Body* bod : cb.fvp[static_cast<int>(gameObjectType::PLAYERTYPE)]) {

						gameObjectDat* dat = static_cast<gameObjectDat*>(bod->GetUserData());

						ls.addObjectAttributes((static_cast<Player*>(dat->obj)->getAttributes()));

					}

					ls.setClassAttributes(Item::getAttributeTypes());

					for (b2Body* bod : cb.fvp[static_cast<int>(gameObjectType::ITEMTYPE)]) {

						gameObjectDat* dat = static_cast<gameObjectDat*>(bod->GetUserData());
						Item* itm = static_cast<Item*>(dat->obj);
						ls.addObjectAttributes(itm->getAttributes());
						//Accelerate item here for efficiency
						
						b2Vec2 dis(thisplayer->x-itm->x, thisplayer->y-itm->y );
						float len=dis.Normalize()/1.5;
						
						if(len>0)bod->ApplyForce(dis*std::min(1.5f,1/(len * len * len * len * len * len )), bod->GetWorldCenter(),true);
					}
					ls.setClassAttributes(Bullet::getAttributeTypes());

					for (b2Body* bod : cb.fvp[static_cast<int>(gameObjectType::BULLETTYPE)]) {

						gameObjectDat* dat = static_cast<gameObjectDat*>(bod->GetUserData());

						ls.addObjectAttributes((static_cast<Bullet*>(dat->obj)->getAttributes()));

					}
					ls.setClassAttributes(Potion::getAttributeTypes());

					for (b2Body* bod : cb.fvp[static_cast<int>(gameObjectType::POTIONTYPE)]) {

						gameObjectDat* dat = static_cast<gameObjectDat*>(bod->GetUserData());

						ls.addObjectAttributes((static_cast<Potion*>(dat->obj)->getAttributes()));

					}


				}

				//PLAYER CHUNK UPDATE
				{
					{
						ls.setClassAttributes(ClientChunkPiece::getAttributeTypes());
					}

					const int cxchunk = (pr.second->x) / 10;
					const int cychunk = (pr.second->y) / 10;

					for (int ychunk = cychunk - viewportYMinus; ychunk <= cychunk + viewportYPlus; ychunk++) {


						for (int xchunk = cxchunk - viewportX; xchunk <= cxchunk + viewportX; xchunk++) {

							if (xchunk < 0 || ychunk < 0 || xchunk >= worldxChunks || ychunk >= worldyChunks) {
								continue;
							}
							Chunk* chunkhere = chunk_array[ychunk][xchunk];
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

				}

				{
					{
						ls.setClassAttributes(Block::getAttributeTypes());
					}

					const int cxchunk = (pr.second->x) / 10;
					const int cychunk = (pr.second->y) / 10;
					std::set<Chunk*> newViewChunks;

					for (int ychunk = cychunk - viewportYMinus; ychunk <= cychunk + viewportYPlus; ychunk++) {


						for (int xchunk = cxchunk - viewportX; xchunk <= cxchunk + viewportX; xchunk++) {

							if (xchunk < 0 || ychunk < 0 || xchunk >= worldxChunks || ychunk >= worldyChunks) {
								continue;
							}
							Chunk* chunkhere = chunk_array[ychunk][xchunk];
							newViewChunks.insert(chunkhere);
							if (updatedChunks.count(chunkhere) || (!(thisplayer->viewChunks.count(chunkhere)))) {
								for (size_t i = 0; i < chunkhere->blocks.size(); i++)
								{
									ls.addObjectAttributes(chunkhere->blocks[i]->getAttributes());
								}
							}

						}
					}
					thisplayer->viewChunks = newViewChunks;
					for (Block* block : updatedBlocks)
					{
						if(thisplayer->x-viewportX*10 <block->x&& block->x < thisplayer->x + viewportX*10&&
							thisplayer->y - viewportYMinus * 10 < block->y && block->y < thisplayer->y + viewportYPlus * 10)ls.addObjectAttributes(block->getAttributes());
					}

				}

				//SEND TO PLAYER (WS)
				try {
					ws_server.send(thisplayer->hdl, ls.serialize(), websocketpp::frame::opcode::binary);
				}
				catch (websocketpp::exception const& e) {
					std::cout << "Websocket error:" << e.what() << std::endl;

				}
			}
			updatedChunks.clear();

			for (auto it = updatedBlocks.begin(); it != updatedBlocks.end(); ) {

				if ((*it)->thisBlockType== Block::NULLBLOCK) {
					delete (*it);
					it = updatedBlocks.erase(it);
				}
				else {
					++it;
				}
			}
			updatedBlocks.clear();

			//PHYSICS
			//std::cout << phys_world.GetBodyCount() << "\n";
			{

				toDestroy.clear();

				phys_world.Step(std::min(70, lastlooplengthms) / 1000.0, 8, 3, 3);//do not simulate over 70ms
				for (const gameObjectDat* td : toDestroy){
					if (td->type == gameObjectType::ITEMTYPE) {
						delete (static_cast<Item*>(td->obj));
					}
					delete td;
				}

			}
			auto t2 = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
			lastlooplengthms = std::max(static_cast<int>(targetMSPT), static_cast<int>(duration));
			timetillnextsec -= lastlooplengthms;
			if (timetillnextsec < 0) {
				timetillnextsec = 1000;
				std::cout << "MSTP:" << duration << "/" << targetMSPT << "\n";
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(targetMSPT - duration));
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
	std::set <Block*> updatedBlocks;

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
