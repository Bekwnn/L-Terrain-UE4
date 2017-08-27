#include "LTerrainEditor.h"
#include "LSystem.h"

//LSystem START

void LSystem::Reset()
{
	rules = TArray<LRulePtr>();
	symbols = TArray<LSymbolPtr>();
	patches = TArray<LPatchPtr>();
	lSystemLoDs = TArray<LSymbol2DMapPtr>();

	GenerateSomeDefaults();
}

void LSystem::GenerateSomeDefaults()
{
	LSymbolPtr plains = LSymbolPtr(new LSymbol('p', "Plains"));
	LSymbolPtr hills = LSymbolPtr(new LSymbol('h', "Hills"));
	LSymbolPtr ocean = LSymbolPtr(new LSymbol('o', "Ocean"));

	symbols.Add(plains);
	symbols.Add(hills);
	symbols.Add(ocean);

	LSymbolPtr grass = LSymbolPtr(new LSymbol('g', "Grass"));
	LSymbolPtr sand = LSymbolPtr(new LSymbol('s', "Sand"));
	LSymbolPtr beach = LSymbolPtr(new LSymbol('b', "Beach"));

	symbols.Add(grass);
	symbols.Add(beach);
	symbols.Add(sand);

	lSystemLoDs.Add(LSymbol::CreateLSymbolMap(3, 3));

	(*lSystemLoDs[0])[0] = { ocean, beach, hills };
	(*lSystemLoDs[0])[1] = { ocean, beach, hills };
	(*lSystemLoDs[0])[2] = { ocean, beach, hills };

	LRulePtr plainsToGrass = LRule::CreatePropegateRule(plains, grass);
	plainsToGrass->name = "Plains > Grass";
	rules.Add(plainsToGrass);

	LSymbol2DMapPtr beachRepl = LSymbol::CreateLSymbolMap(5, 5);
	(*beachRepl)[0] = { ocean, sand, sand, grass, grass };
	(*beachRepl)[1] = { ocean, sand, sand, grass, grass };
	(*beachRepl)[2] = { ocean, ocean, sand, grass, grass };
	(*beachRepl)[3] = { ocean, ocean, sand, sand, grass };
	(*beachRepl)[4] = { ocean, sand, sand, grass, grass };

	LRulePtr detailBeach = LRule::CreateRule(beach, beachRepl);
	detailBeach->name = "Detail Beach";
	rules.Add(detailBeach);

	LPatchPtr oceanPatch = LPatchPtr(new LPatch());
	oceanPatch->matchVal = ocean;
	oceanPatch->name = "Ocean Patch";
	oceanPatch->minHeight = -40.f;
	oceanPatch->maxHeight = -1.f;

	patches.Add(oceanPatch);

	LPatchPtr beachPatch = LPatchPtr(new LPatch());
	beachPatch->matchVal = beach;
	beachPatch->name = "Beach Patch";
	beachPatch->minHeight = 0.f;
	beachPatch->maxHeight = 2.f;

	patches.Add(beachPatch);

	LPatchPtr grassPatch = LPatchPtr(new LPatch());
	grassPatch->matchVal = grass;
	grassPatch->name = "Grass Patch";
	grassPatch->minHeight = 2.f;
	grassPatch->maxHeight = 10.f;

	patches.Add(grassPatch);
}

LSymbol2DMapPtr LSystem::IterateLString(LSymbol2DMapPtr source)
{
	int idim = source->Num();
	int jdim = (*source)[0].Num();
	LSymbol2DMapPtr newSystemString = LSymbol::CreateLSymbolMap(idim * LSystem::DIMS, jdim * LSystem::DIMS);

	for (int i = 0; i < idim; ++i)
	{
		for (int j = 0; j < jdim; ++j)
		{
			LRulePtr matchingRule = GetLRuleMatch(source, j, i);

			for (int i2 = 0; i2 < DIMS; ++i2)
			{
				for (int j2 = 0; j2 < DIMS; ++j2)
				{
					(*newSystemString)[i*DIMS + i2][j*DIMS + j2] = (*(matchingRule->replacementVals))[i2][j2];
				}
			}
		}
	}

	return newSystemString;
}

LRulePtr LSystem::GetLRuleMatch(LSymbol2DMapPtr map, int xIdx, int yIdx)
{
	TArray<LRulePtr> matches = TArray<LRulePtr>();

	LSymbolPtr toMatch = (*map)[yIdx][xIdx]; //middle element

	for (LRulePtr rule : rules)
	{
		if (toMatch == rule->matchVal)
		{
			//add rule to matches if it 
			if (!rule->bMatchNeighbors) matches.Add(rule);
			else
			{
				bool bFailMatch = false;
				for (int i = 0; i < 3; ++i)
				{
					for (int j = 0; j < 3; ++j)
					{
						if (xIdx + j - 1 < 0 || xIdx + j - 1 > (*map).Num() ||
							yIdx + i - 1 < 0 || yIdx + i - 1 > (*map)[0].Num())
						{
							continue;
						}

						if ((*rule->matchNeighborsMap)[i][j] != LSymbol::MatchAny() &&
							(*map)[yIdx + i - 1][xIdx + j - 1] != (*rule->matchNeighborsMap)[i][j])
						{
							bFailMatch = true;
						}
					}
				}

				if (!bFailMatch) matches.Add(rule);
			}
		}
	}

	//if no match found, create a rule to propegate toMatch (default behavior)
	if (matches.Num() == 0)
		return LRule::CreatePropegateRule(toMatch, toMatch);
	else
	{
		//TODO: better metric for figuring out best match from matches array
		//return first neighbor matching rule which matched
		//failing that, return the first non-neighbor matching rule
		for (LRulePtr rule : matches)
		{
			if (rule->bMatchNeighbors) return rule;
		}
		return matches[0];
	}
}

LPatchPtr LSystem::GetLPatchMatch(LSymbolPtr toMatch)
{
	for (LPatchPtr patch : patches)
	{
		if (toMatch == patch->matchVal)
			return patch;
	}

	return LPatchPtr(new LPatch());
}

LSymbolPtr LSystem::GetMapSymbolFrom01Coords(LSymbol2DMapPtr map, float xPercCoord, float yPercCoord)
{
	xPercCoord = FMath::Clamp(xPercCoord, 0.f, 0.99999f);
	yPercCoord = FMath::Clamp(yPercCoord, 0.f, 0.99999f);

	int ydim = (*map).Num();
	int xdim = (*map)[0].Num();

	int xIdx = FMath::FloorToInt(xPercCoord * xdim);
	int yIdx = FMath::FloorToInt(yPercCoord * ydim);
	return (*map)[xIdx][yIdx];
}

LSymbolPtr LSystem::GetDefaultSymbol()
{
	return (symbols.Num() > 0) ? symbols[0] : LSymbolPtr();
}

//LSystem END
//LSymbol START

LSymbolPtr LSymbol::_matchAny = LSymbolPtr(new LSymbol('?', "Match Any"));

LSymbol::LSymbol(char symbol, FString name)
{
	this->symbol = symbol;
	this->name = name;
}

LSymbol2DMapPtr LSymbol::CreateLSymbolMap(int inner, int outer)
{
	LSymbol2DMapPtr map = LSymbol2DMapPtr(new LSymbol2DMap());
	for (int i = 0; i < outer; ++i)
	{
		map->Add(TArray<LSymbolPtr>());
		for (int j = 0; j < inner; ++j)
		{
			(*map)[i].Add(LSymbolPtr());
		}
	}

	return map;
}

//LSymbol END
//LRule START

LRulePtr LRule::CreateRule(LSymbolPtr matchVal, LSymbol2DMapPtr replacementVals)
{
	if (replacementVals->Num() != LSystem::DIMS || (*replacementVals)[0].Num() != replacementVals->Num())
		return LRulePtr();
	else
		return LRulePtr(new LRule(matchVal, replacementVals));
}

LRulePtr LRule::CreatePropegateRule(LSymbolPtr matchVal, LSymbolPtr propegateVal)
{
	LSymbol2DMapPtr replacementVals = LSymbol::CreateLSymbolMap(LSystem::DIMS, LSystem::DIMS);
	for (int i = 0; i < LSystem::DIMS; ++i)
	{
		for (int j = 0; j < LSystem::DIMS; ++j)
		{
			(*replacementVals)[i][j] = propegateVal;
		}
	}
	return CreateRule(matchVal, replacementVals);
}