#pragma once
#include <Box2D/Box2D.h>

//USE INLINE VARS IN C++17 (REMEMBER TO CHANGE!)
static constexpr unsigned int targetMSPT = 30;

static constexpr int viewportX = 2;//chunks
static constexpr int viewportYPlus = 20;//chunks
static constexpr int viewportYMinus = 2;//chunkss

static constexpr unsigned int worldxChunks = 40;//set to 100 later
static constexpr unsigned int worldyChunks = 20;
static constexpr unsigned int terrainPrecision = 3;//per block
static constexpr unsigned int clippperPrecision = 6;//per block
static constexpr float noiseRandomSeperation = 10 * 100;


struct objecthitboxinfo {
	static const b2Vec2 player;
	static const b2Vec2 item;
	static const b2Vec2 bullet;
	static const float potion;
};

