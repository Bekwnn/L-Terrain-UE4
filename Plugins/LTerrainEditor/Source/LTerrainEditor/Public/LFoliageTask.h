#pragma once
#include "LTerrainEditor.h"
#include "LTerrainGeneration.h"

#include "InstancedFoliageActor.h"
#include "FoliageType.h"
#include "InstancedFoliage.h"

#include "Async/AsyncWork.h"

DECLARE_DELEGATE_OneParam(FOnFoliageCompletion, bool)

struct LFoliageParams
{
public:
	UFoliageType* foliageType;
	FFoliageMeshInfo* meshInfo;
	LPatchPtr patch;
	LObjectScatterPtr objectScatter;
	int RNGSeed;
	TArray<FVector> acceptedLocations3D;
};

class FLFoliageTask : public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FLFoliageTask>;

public:
	FLFoliageTask(LFoliageParams& FP, LSharedTaskParams& SP, FOnFoliageCompletion onCompletion) :
		FP(FP),
		SP(SP),
		onCompletion(onCompletion)
	{}

protected:
	void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FLFoliageTask, STATGROUP_ThreadPoolAsyncTasks);
	}

protected:
	FOnFoliageCompletion onCompletion;
	LFoliageParams& FP;
	LSharedTaskParams& SP;
};