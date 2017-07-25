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
	LSymbolPtr plains = LSymbolPtr(new LSymbol('p', "plains"));
	LSymbolPtr hills = LSymbolPtr(new LSymbol('h', "hills"));
	LSymbolPtr ocean = LSymbolPtr(new LSymbol('o', "ocean"));

	symbols.Add(plains);
	symbols.Add(hills);
	symbols.Add(ocean);

	LSymbolPtr grass = LSymbolPtr(new LSymbol('g', "grass"));
	LSymbolPtr sand = LSymbolPtr(new LSymbol('s', "sand"));
	LSymbolPtr beach = LSymbolPtr(new LSymbol('b', "beach"));

	symbols.Add(grass);
	symbols.Add(beach);
	symbols.Add(sand);

	lSystemLoDs.Add(LSymbol::CreateLSymbolMap(3, 3));

	(*lSystemLoDs[0])[0] = { ocean, beach, hills };
	(*lSystemLoDs[0])[1] = { ocean, beach, hills };
	(*lSystemLoDs[0])[2] = { ocean, beach, hills };

	LRulePtr plainsToGrass = LRule::CreatePropegateRule(plains, grass);
	plainsToGrass->name = "Pprgate Plains > Grass";
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
			LRulePtr matchingRule = GetLRuleMatch((*source)[i][j]);

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

LRulePtr LSystem::GetLRuleMatch(LSymbolPtr toMatch)
{
	for (LRulePtr rule : rules)
	{
		if (toMatch == rule->matchVal)
			return rule;
	}

	//if no match found, create a rule to propegate toMatch (default behavior)
	return LRule::CreatePropegateRule(toMatch, toMatch);
}

LPatchPtr LSystem::GetLPatchMatch(LSymbolPtr toMatch)
{
	for (LPatchPtr patch : patches)
	{
		if (toMatch == patch->matchVal)
			return patch;
	}

	//if no match found, return null
	return LPatchPtr();
}

LSymbolPtr LSystem::GetDefaultSymbol()
{
	return (symbols.Num() > 0) ? symbols[0] : nullptr;
}

//LSystem END
//LSymbol START

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