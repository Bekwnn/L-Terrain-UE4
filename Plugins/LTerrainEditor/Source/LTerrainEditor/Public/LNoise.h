#pragma once
#include "LTerrainEditor.h"
#include <random>

class LNoise;
class LNoiseObject;

typedef TSharedPtr<LNoise, ESPMode::ThreadSafe> LNoisePtr;
typedef TSharedPtr<LNoiseObject, ESPMode::ThreadSafe> LNoiseObjectPtr;

enum class ENoiseType : uint8
{
	WHITE,
	PINK,
	BLUE,
	PERLIN,
};

class LNoise
{
public:
	LNoise(ENoiseType noiseType);
	LNoise(ENoiseType noiseType, int32 seedVal);
	void Reseed();
	void Reseed(int32 seedVal);
	ENoiseType GetNoiseType();
	float Noise(float x, float y);

private:
	void InitNoise(int32 seedVal);

public:
	float frequency; //working on a scale where 10 meters is one 1.0 frequency
	float amplitude; //in world meters, values above 0.5 raise, below 0.5 lowers

private:
	ENoiseType noiseType;
	LNoiseObjectPtr noiseObject;
};

class LNoiseObject
{
public:
	virtual float Noise(float x, float y) = 0;
	virtual void Initialize(int32 seedVal) = 0;

protected:
	int32 seed;
	FCriticalSection RNGLock;
};

class LColoredNoise : public LNoiseObject
{
public:
	LColoredNoise(float exponent);
	virtual float Noise(float x, float y) override;
	virtual void Initialize(int32 seedVal) override;

private:
	//-1 exponent: favors low frequencies
	// 0 exponent: even weighted frequencies
	//+1 exponent: favors high frequencies
	float exponent;
	std::default_random_engine generatorX;
	std::default_random_engine generatorY;
	std::uniform_real_distribution<float> tauShiftDistribution;
	std::uniform_int_distribution<int> generate2ndSeed;
	static float TAU;
	int32 seed2;
};

class LPerlinNoise : public LNoiseObject
{
public:
	LPerlinNoise();
	virtual float Noise(float x, float y) override;
	virtual void Initialize(int32 seedVal) override;

private:
	float DotGrad(int ix, int iy, float x, float y);
	float EaseFunction(float t);

private:
	std::uniform_real_distribution<float> gradVecRotDistribution;
	std::default_random_engine generator;
};
