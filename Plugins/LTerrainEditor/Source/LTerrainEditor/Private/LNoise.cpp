#include "LTerrainEditor.h"
#include "LNoise.h"

LNoise::LNoise(ENoiseType noiseType)
{
	this->noiseType = noiseType;

	InitNoise(FMath::RandRange(INT32_MIN, INT32_MAX));
}

LNoise::LNoise(ENoiseType noiseType, int32 seedVal)
{
	this->noiseType = noiseType;

	InitNoise(seedVal);
}

void LNoise::Reseed()
{
	noiseObject->Initialize(FMath::RandRange(INT32_MIN, INT32_MAX));
}

void LNoise::Reseed(int32 seedVal)
{
	noiseObject->Initialize(seedVal);
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

void LNoise::InitNoise(int32 seedVal)
{
	switch (noiseType)
	{
	case ENoiseType::WHITE:
		noiseObject = TSharedPtr<LNoiseObject>(new LColoredNoise(0.f));
		break;
	case ENoiseType::PINK:
		noiseObject = TSharedPtr<LNoiseObject>(new LColoredNoise(-1.f));
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

	noiseObject->Initialize(seedVal);
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

void LPerlinNoise::Initialize(int32 seedVal)
{
	this->seed = seedVal;
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

float LColoredNoise::TAU = 2.f*PI;

LColoredNoise::LColoredNoise(float exponent)
{
	this->exponent = exponent;
	tauShiftDistribution = std::uniform_real_distribution<float>(0.f, TAU);
	generate2ndSeed = std::uniform_int_distribution<int>(INT32_MIN, INT32_MAX);
}

float LColoredNoise::Noise(float x, float y)
{

	generatorX.seed(seed + y);
	generatorY.seed(seed2 + x);
	TArray<float> values = TArray<float>();
	TArray<float> weights = TArray<float>();
	float sumWeights = 0.f;
	for (int freq = 1; freq < 31; ++freq)
	{
		//will always generate the same 60 number shift sequence since we seed at start
		float shiftX = tauShiftDistribution(generatorX);
		float shiftY = tauShiftDistribution(generatorY);

		values.Add(FMath::Max(FMath::Sin((TAU*x + shiftX)*freq), FMath::Sin((TAU*y + shiftY)*freq)));

		float weight = FMath::Pow(freq, exponent);
		sumWeights += weight;
		weights.Add(weight);
	}

	float weightedValue = 0.f;
	for (int i = 0; i < values.Num(); ++i)
	{
		weightedValue += values[i] * (weights[i] / sumWeights);
	}

	return weightedValue;
}

void LColoredNoise::Initialize(int32 seedVal)
{
	this->seed = seedVal;
	//generate 2nd seed
	generatorX.seed(seedVal);
	this->seed2 = generate2ndSeed(generatorX);
}
