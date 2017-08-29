#pragma once
#include "LTerrainEditor.h"
#include "LNoise.h"

class LPatch;
class LRule;
class LSymbol;
class LGroundTexture;
class LObjectScatter;

typedef TSharedPtr<LSymbol> LSymbolPtr;
typedef TSharedPtr<LRule> LRulePtr;
typedef TSharedPtr<LPatch> LPatchPtr;
typedef TSharedPtr<LGroundTexture> LGroundTexturePtr;
typedef TSharedPtr<LObjectScatter> LObjectScatterPtr;
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
		noiseMaps = TArray<TSharedPtr<LNoise>>();
		groundTextures = TArray<TSharedPtr<LGroundTexture>>();
		objectScatters = TArray<TSharedPtr<LObjectScatter>>();
	}

	FString name;
	LSymbolPtr matchVal;
	float minHeight;
	float maxHeight;
	TArray<TSharedPtr<LNoise>> noiseMaps;
	TArray<TSharedPtr<LGroundTexture>> groundTextures; //first entry should be used as base
	TArray<TSharedPtr<LObjectScatter>> objectScatters;
};

class LObjectScatter
{
public:
	FString name;
	FAssetData object;
	float frequency;
	//TODO: distribution, noise etc

};

class LGroundTexture
{
public:
	FString name;
	FAssetData texture;
	//TODO distribution, noise etc
};