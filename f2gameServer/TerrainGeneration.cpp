#include <vector>
#include "SimplexNoise.h"

#include "settings.h"
#include <iostream>

#include "TerrainGeneration.h"

std::vector<float> TerrainGeneration::getBiomePercentages(float x) {

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

	return(biomePercentages);

	std::vector<float> ret = { 1,0,0 };
	return(ret);

}

float TerrainGeneration::getStoneLevelOfPlains(float x) {
	float val = SimplexNoise::noise(x * .01 + noiseRandomSeperation * 15) * 3 + worldyChunks * 10 / 2 - 10;
	return(val);
}
float TerrainGeneration::getDirtLevelOfPlains(float x) {
	float val = getStoneLevelOfPlains(x);
	val += SimplexNoise::noise(x * .02 + noiseRandomSeperation * 5) * .5 + 7;
	//val += SimplexNoise::noise(x + noiseRandomSeperation * 6) * .1;

	return(val);
}
float TerrainGeneration::getGrassLevelOfPlains(float x) {
	float val = getDirtLevelOfPlains(x);
	val += .3;
	return(val);
}
//MOUNTIAINS

float TerrainGeneration::getStoneLevelOfMountains(float x) {
	float val = SimplexNoise::noise(x * .01) * 70 + worldyChunks * 10 / 2 + 30;
	val += SimplexNoise::noise(x * .03 + noiseRandomSeperation * 7) * .1;

	return(val);
}
float TerrainGeneration::getDirtLevelOfMountains(float x) {
	float val = getStoneLevelOfMountains(x);
	val += 0;
	return(val);
}
float TerrainGeneration::getGrassLevelOfMountains(float x) {
	float val = getDirtLevelOfMountains(x);
	val += 0;
	return(val);
}
//OCEANS
float TerrainGeneration::getStoneLevelOfOceans(float x) {
	float val = SimplexNoise::noise(x * .005) * 3 + worldyChunks * 10 / 2 - 30;

	return(val);
}
float TerrainGeneration::getDirtLevelOfOceans(float x) {
	float val = getStoneLevelOfOceans(x);
	val += 2;
	return(val);
}
float TerrainGeneration::getGrassLevelOfOceans(float x) {
	float val = getDirtLevelOfOceans(x);
	val += 0;
	return(val);
}
//GENERAL
float TerrainGeneration::getStoneLevel(float x) {
	std::vector<float> bvals = getBiomePercentages(x);
	float val = (getStoneLevelOfPlains(x) * bvals[0] + getStoneLevelOfMountains(x) * bvals[1] + getStoneLevelOfOceans(x) * bvals[2]);
	//std::cout << "stone" << val<<"\n";
	return(val);
}
float TerrainGeneration::getDirtLevel(float x) {
	std::vector<float> bvals = getBiomePercentages(x);
	float val = (getDirtLevelOfPlains(x) * bvals[0] + getDirtLevelOfMountains(x) * bvals[1] + getDirtLevelOfOceans(x) * bvals[2]);
	return(val);
}
float TerrainGeneration::getGrassLevel(float x) {
	std::vector<float> bvals = getBiomePercentages(x);
	float val = (getGrassLevelOfPlains(x) * bvals[0] + getGrassLevelOfMountains(x) * bvals[1] + getGrassLevelOfOceans(x) * bvals[2]);
	return(val);
}
