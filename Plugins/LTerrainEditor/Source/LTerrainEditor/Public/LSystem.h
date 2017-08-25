#pragma once
#include "LTerrainEditor.h"

class LPatch;
class LRule;
class LSymbol;
class LGroundTexture;
class LObjectScatter;
class LNoise;

typedef TSharedPtr<LSymbol> LSymbolPtr;
typedef TSharedPtr<LRule> LRulePtr;
typedef TSharedPtr<LPatch> LPatchPtr;
typedef TArray<TArray<TSharedPtr<LSymbol>>> LSymbol2DMap;
typedef TSharedPtr<LSymbol2DMap> LSymbol2DMapPtr;

class LSystem
{
public:
	void Reset();
	void GenerateSomeDefaults();
	LSymbol2DMapPtr IterateLString(LSymbol2DMapPtr source);
	LRulePtr GetLRuleMatch(LSymbol2DMapPtr map, int xIdx, int yIdx);
	LPatchPtr GetLPatchMatch(LSymbolPtr toMatch);
	static LSymbolPtr GetMapSymbolFrom01Coords(LSymbol2DMapPtr map, float xPercCoord, float yPercCoord);
	LSymbolPtr GetDefaultSymbol();

public:
	TArray<LRulePtr> rules;
	TArray<LSymbolPtr> symbols;
	TArray<LPatchPtr> patches;
	TArray<LSymbol2DMapPtr> lSystemLoDs;

	static const int DIMS = 5;
};

class LSymbol
{
public:
	LSymbol(char symbol, FString name);
	static LSymbol2DMapPtr CreateLSymbolMap(int inner, int outer);

	//special static instance of LSymbol to represent "any symbol"
	static LSymbolPtr MatchAny() { return _matchAny; }

	char symbol;
	FString name;
	FAssetData texture;

protected:
	static LSymbolPtr _matchAny;
};

class LRule
{
public:
	static LRulePtr CreateRule(LSymbolPtr matchVal, LSymbol2DMapPtr replacementVals);
	static LRulePtr CreatePropegateRule(LSymbolPtr matchVal, LSymbolPtr propegateVal);


protected:
	LRule(LSymbolPtr matchVal, LSymbol2DMapPtr replacementVals)
	{
		this->matchVal = matchVal;
		this->replacementVals = replacementVals;

		bMatchNeighbors = false;
		matchNeighborsMap = LSymbol::CreateLSymbolMap(3, 3);
		(*matchNeighborsMap)[0] = { LSymbol::MatchAny(), LSymbol::MatchAny(), LSymbol::MatchAny() };
		(*matchNeighborsMap)[1] = { LSymbol::MatchAny(),            matchVal, LSymbol::MatchAny() };
		(*matchNeighborsMap)[2] = { LSymbol::MatchAny(), LSymbol::MatchAny(), LSymbol::MatchAny() };
	}

public:
	FString name;
	LSymbolPtr matchVal;
	LSymbol2DMapPtr replacementVals;
	bool bMatchNeighbors;
	LSymbol2DMapPtr matchNeighborsMap;

};

class LPatch
{
public:
	LPatch()
	{
		name = "default patch";
		matchVal = LSymbolPtr();
		minHeight = 0.f;
		maxHeight = 0.f;
		noiseMaps = TArray<LNoise*>();
		objectScatters = TArray<LObjectScatter*>();
		groundTextures = TArray<LGroundTexture*>();
	}

	FString name;
	LSymbolPtr matchVal;
	float minHeight;
	float maxHeight;
	TArray<LNoise*> noiseMaps;
	TArray<LObjectScatter*> objectScatters;
	TArray<LGroundTexture*> groundTextures;
};

class LNoise
{
	float frequency;
	float amplitude; //in meters
	//TODO: has a delegate which returns 01 heightmap value?
};

class LObjectScatter
{
	float frequency;
	//TODO: distribution method

};

class LGroundTexture
{

};