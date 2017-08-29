#pragma once
#include "LTerrainEditor.h"
#include <random>

class LNoise;
class LNoiseObject;

typedef TSharedPtr<LNoise> LNoisePtr;

enum class ENoiseType : uint8
{
	WHITE,
	RED,
	BLUE,
	PERLIN,
};

class LNoise
{
public:
	LNoise(ENoiseType noiseType);
	LNoise(ENoiseType noiseType, int32 seed);
	void Reseed();
	void Reseed(int32 seed);
	ENoiseType GetNoiseType();
	float Noise(float x, float y);

private:
	void InitNoise(int32 seed);

public:
	float frequency; //working on a scale where 10 meters is one 1.0 frequency
	float amplitude; //in world meters, values above 0.5 raise, below 0.5 lowers

private:
	ENoiseType noiseType;
	TSharedPtr<LNoiseObject> noiseObject;
};

class LNoiseObject
{
public:
	virtual float Noise(float x, float y) = 0;
	virtual void Initialize(int32 seed) = 0;

protected:
	int32 seed;
	std::default_random_engine generator;
};

class LColoredNoise : public LNoiseObject
{
public:
	LColoredNoise(float exponent);
	virtual float Noise(float x, float y) override;
	virtual void Initialize(int32 seed) override;

private:
	float exponent; //better understand how different colored noises work, maybe change this
};

class LPerlinNoise : public LNoiseObject
{
public:
	LPerlinNoise();
	virtual float Noise(float x, float y) override;
	virtual void Initialize(int32 seed) override;

private:
	float DotGrad(int ix, int iy, float x, float y);
	float EaseFunction(float t);

private:
	std::uniform_real_distribution<float> gradVecRotDistribution;
};