#pragma once
#include "LTerrainEditor.h"
#include "LTerrainGeneration.h"

#include "Async/AsyncWork.h"

DECLARE_DELEGATE_OneParam(FOnCompletion, bool)

class FLTerrainComponentMainTask : public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FLTerrainComponentMainTask>;

public:
	FLTerrainComponentMainTask(int32 compIdx, LSharedTaskParams& SP, FOnCompletion onCompletion);

protected:
	void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FLTerrainComponentMainTask, STATGROUP_ThreadPoolAsyncTasks);
	}

protected:
	int32 compIdx;
	FOnCompletion onCompletion;
	LSharedTaskParams& SP;
};