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
	std::unordered_map <gameObjectType, std::vector<b2Body*>> fvp;

	bool ReportFixture(b2Fixture* fixture) {
		fvp[static_cast<gameObjectDat*>(fixture->GetBody()->GetUserData())->type].push_back(fixture->GetBody());
		return(true);
	}
};
//

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
					players[e.hdl]->movementkeyboard[0] = strpl[0] == 1;
					players[e.hdl]->movementkeyboard[1] = strpl[1] == 1;
					players[e.hdl]->movementkeyboard[2] = strpl[2] == 1;
					players[e.hdl]->movementkeyboard[3] = strpl[3] == 1;
					std::cout<< players[e.hdl]->movementkeyboard[1]<<"\n";
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
				float xaccel = .4;

				if (thisplayer->movementkeyboard[0]&&(!thisplayer->movementImpulseJumpLast) && thisplayer->physBody->GetLinearVelocity().y <8) {//W
					thisplayer->physBody->ApplyLinearImpulse(b2Vec2(0, 8), thisplayer->physBody->GetWorldCenter(),true);
				}
				thisplayer->movementImpulseJumpLast = thisplayer->movementkeyboard[0];

				if (thisplayer->movementkeyboard[1]&& thisplayer->physBody->GetLinearVelocity().x>-maxXVol) {//A
					thisplayer->physBody->ApplyLinearImpulse(b2Vec2(-xaccel, 0), thisplayer->physBody->GetWorldCenter(), true);
				}
				if (thisplayer->movementkeyboard[3] && thisplayer->physBody->GetLinearVelocity().x < maxXVol) {//D
					thisplayer->physBody->ApplyLinearImpulse(b2Vec2(xaccel, 0), thisplayer->physBody->GetWorldCenter(), true);
				}
				/*
				float currangle = -thisplayer->physBody->GetAngle();//0 is desired angle
				currangle = std::fmod(currangle, M_PI * 2);
				if (currangle > M_PI)currangle -= 2 * M_PI;
				currangle *= std::abs(currangle) * 12;
				currangle -= thisplayer->physBody->GetAngularVelocity();
				thisplayer->physBody->ApplyTorque(currangle, true);
				*/
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

					for (b2Body* bod : cb.fvp[gameObjectType::PLAYERTYPE]) {

						gameObjectDat* dat = static_cast<gameObjectDat*>(bod->GetUserData());

						ls.addObjectAttributes((static_cast<Player*>(dat->obj)->getAttributes()));

					}

					ls.setClassAttributes(Item::getAttributeTypes());

					for (b2Body* bod : cb.fvp[gameObjectType::ITEMTYPE]) {

						gameObjectDat* dat = static_cast<gameObjectDat*>(bod->GetUserData());

						ls.addObjectAttributes((static_cast<Item*>(dat->obj)->getAttributes()));

					}
					ls.setClassAttributes(Bullet::getAttributeTypes());

					for (b2Body* bod : cb.fvp[gameObjectType::BULLETTYPE]) {

						gameObjectDat* dat = static_cast<gameObjectDat*>(bod->GetUserData());

						ls.addObjectAttributes((static_cast<Bullet*>(dat->obj)->getAttributes()));

					}
					ls.setClassAttributes(Potion::getAttributeTypes());

					for (b2Body* bod : cb.fvp[gameObjectType::POTIONTYPE]) {

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
					std::set<Chunk*> newViewChunks;

					for (int ychunk = cychunk - viewportYMinus; ychunk <= cychunk + viewportYPlus; ychunk++) {


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
				try {
					ws_server.send(thisplayer->hdl, ls.serialize(), websocketpp::frame::opcode::binary);
				}
				catch (websocketpp::exception const& e) {
					std::cout << "Websocket error:" << e.what() << std::endl;

				}
			}
			updatedChunks.clear();
			//PHYSICS
			//std::cout << phys_world.GetBodyCount() << "\n";



			phys_world.Step(std::min(70,lastlooplengthms)/1000.0, 8, 3, 3);//do not simulate over 70ms
			auto t2 = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
			lastlooplengthms = std::max(static_cast< int>(targetMSPT), static_cast<int>(duration));
			timetillnextsec -= lastlooplengthms;
			if (timetillnextsec<0) {
				timetillnextsec = 1000;
				std::cout << "MSTP:"<< duration<<"/"<< targetMSPT <<"\n";
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(targetMSPT- duration));
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
