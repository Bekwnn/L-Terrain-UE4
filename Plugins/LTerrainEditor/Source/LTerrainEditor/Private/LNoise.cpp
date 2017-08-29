#include "LTerrainEditor.h"
#include "LNoise.h"

LNoise::LNoise(ENoiseType noiseType)
{
	this->noiseType = noiseType;

	InitNoise(FMath::RandRange(INT32_MIN, INT32_MAX));
}

LNoise::LNoise(ENoiseType noiseType, int32 seed)
{
	this->noiseType = noiseType;

	InitNoise(seed);
}

void LNoise::Reseed()
{
	noiseObject->Initialize(FMath::RandRange(INT32_MIN, INT32_MAX));
}

void LNoise::Reseed(int32 seed)
{
	noiseObject->Initialize(seed);
}

ENoiseType LNoise::GetNoiseType()
{
	return noiseType;
}

float LNoise::Noise(float x, float y)
{
	//returns within range [-amplitude/2, amplitude/2]
	return amplitude * (noiseObject->Noise(frequency*x, frequency*y) - 0.5f);
}

void LNoise::InitNoise(int32 seed)
{
	switch (noiseType)
	{
	case ENoiseType::WHITE:
		noiseObject = TSharedPtr<LNoiseObject>(new LColoredNoise(0.f));
		break;
	case ENoiseType::RED:
		noiseObject = TSharedPtr<LNoiseObject>(new LColoredNoise(-2.f));
		break;
	case ENoiseType::BLUE:
		noiseObject = TSharedPtr<LNoiseObject>(new LColoredNoise(1.f));
		break;
	case ENoiseType::PERLIN:
		noiseObject = TSharedPtr<LNoiseObject>(new LPerlinNoise());
		break;
	default:
		break;
	}

	noiseObject->Initialize(seed);
}

LPerlinNoise::LPerlinNoise()
{
	gradVecRotDistribution = std::uniform_real_distribution<float>(0.f, 360.f);
}

float LPerlinNoise::Noise(float x, float y)
{
	int x0 = FMath::FloorToInt(x);
	int y0 = FMath::FloorToInt(y);
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	//lerp values modified by an ease function
	float xFracEase = EaseFunction(x - x0);
	float yFracEase = EaseFunction(y - y0);

	float dot0, dot1, lerpx0, lerpx1;
	dot0 = DotGrad(x0, y0, x, y);
	dot1 = DotGrad(x1, y0, x, y);
	lerpx0 = FMath::Lerp(dot0, dot1, xFracEase);
	dot0 = DotGrad(x0, y1, x, y);
	dot1 = DotGrad(x1, y1, x, y);
	lerpx1 = FMath::Lerp(dot0, dot1, xFracEase);

	return FMath::Lerp(lerpx0, lerpx1, yFracEase);
}

void LPerlinNoise::Initialize(int32 seed)
{
	this->seed = seed;
}

float LPerlinNoise::DotGrad(int ix, int iy, float x, float y)
{
	//get our vector using our seed
	//to get semi-unique seed val from ix, iy, using: (ix*primeA + ix) + (iy*primeB + iy)
	generator.seed(seed + (ix * 47 + ix) + (iy * 113 + iy));
	FVector2D gridVec = FVector2D(1.f, 0.f).GetRotated(gradVecRotDistribution(generator));

	float dx = x - ix;
	float dy = y - iy;

	return dx*gridVec.X + dy*gridVec.Y;
}

//based off the ease function used by original perlin noise
float LPerlinNoise::EaseFunction(float t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

LColoredNoise::LColoredNoise(float exponent)
{
	this->exponent = exponent;
}

float LColoredNoise::Noise(float x, float y)
{
	return 0.0f;
}

void LColoredNoise::Initialize(int32 seed)
{
	this->seed = seed;
}
