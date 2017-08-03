#pragma once
#include "LTerrainEditor.h"

class LPatch;
class LRule;
class LSymbol;

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
	LRulePtr GetLRuleMatch(LSymbolPtr toMatch);
	LPatchPtr GetLPatchMatch(LSymbolPtr toMatch);
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

	char symbol;
	FString name;
	FAssetData texture;
};

class LRule
{
public:
	static LRulePtr CreateRule(LSymbolPtr matchVal, LSymbol2DMapPtr replacementVals);
	static LRulePtr CreatePropegateRule(LSymbolPtr matchVal, LSymbolPtr propegateVal);

	//special static instance of LRule to designate "any" LRule for matches
	static LSymbolPtr MatchAny() { return LSymbolPtr(&_matchAny); }

protected:
	LRule(LSymbolPtr matchVal, LSymbol2DMapPtr replacementVals)
	{
		this->matchVal = matchVal;
		this->replacementVals = replacementVals;

		bMatchNeighbors = false;
		matchNeighborsMap = TArray<LSymbolPtr>();
		matchNeighborsMap.Init(LRule::MatchAny(), 8);
	}

public:
	FString name;
	LSymbolPtr matchVal;
	LSymbol2DMapPtr replacementVals;
	bool bMatchNeighbors;
	TArray<LSymbolPtr> matchNeighborsMap;

protected:
	static LSymbol _matchAny;
};

class LPatch
{
public:
	FString name;
	LSymbolPtr matchVal;
	float minHeight;
	float maxHeight;
	//TODO: prefabs, trees, grass, rocks, etc
	//UTexture2D tex;
};