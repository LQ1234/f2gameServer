#pragma once
#include <vector>
#include "SimplexNoise.h"

#include "settings.h"
#include <iostream>


class TerrainGeneration {
public:
	static std::vector<float> getBiomePercentages(float x);
	//PLAINS
	static float getStoneLevelOfPlains(float x);
	static float getDirtLevelOfPlains(float x);
	static float getGrassLevelOfPlains(float x);
	//MOUNTIAINS

	static float getStoneLevelOfMountains(float x);
	static float getDirtLevelOfMountains(float x);
	static float getGrassLevelOfMountains(float x);
	//OCEANS
	static float getStoneLevelOfOceans(float x);
	static float getDirtLevelOfOceans(float x);
	static float getGrassLevelOfOceans(float x);
	//GENERAL
	static float getStoneLevel(float x);
	static float getDirtLevel(float x);
	static float getGrassLevel(float x);
};