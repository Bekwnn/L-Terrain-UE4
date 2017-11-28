#include "LTerrainEditor.h"
#include "LFoliageTask.h"

struct Coords
{
public:
	Coords() : x(-1), y(-1) {}
	Coords(int x, int y) : x(x), y(y) {}
	int x;
	int y;
};

void FLFoliageTask::DoWork()
{
	FRandomStream stream = FRandomStream(FP.RNGSeed);
	//get the main foliage actor for all foliage instances
	AInstancedFoliageActor* foliageActor = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(SP.terrain->GetWorld(), true);

	///START Implementation of Fast Poisson Disk Sampling in Arbitrary Dimensions - R. Bridson (2007)
	//pick our initial point near the middle
	TArray<FVector2D> acceptedPointLocations = TArray<FVector2D>();

	//initialize our cell grid
	int uniqueVertCountWidth = SP.landscapeComponentCountSqrt*(SP.ComponentSizeVerts - 1); //width in verts, not counting overlaps at seams
	float realWidthcm = SP.terrain->GetActorScale().X*uniqueVertCountWidth;
	if (FP.objectScatter->minRadius < 0.25f) FP.objectScatter->minRadius = 0.25f;
	float minRadiuscm = FP.objectScatter->minRadius*100.f;
	float maxRadiuscm = FP.objectScatter->maxRadius*100.f;
	float cellSize = minRadiuscm / 1.41421f; //sqrt(n) for n-dimensional
	float cellSizeInv = 1.f / cellSize;
	int gridCount = (realWidthcm / cellSize) + 1; //totalTerrainSize(cm)/cellSize(cm), plus 1 padding
	TArray<TArray<int>> grid = TArray<TArray<int>>(); //2D array of size gridCount x gridCount
	grid.Init(TArray<int>(), gridCount);
	for (int i = 0; i < gridCount; ++i)
	{
		grid[i].Init(-1, gridCount);
	}

	//create and insert the initial point
	//active list points to grid list, grid list points to accepted list
	TArray<Coords> activePoints = TArray<Coords>();
	acceptedPointLocations.Add(FVector2D(
		stream.FRandRange(realWidthcm*0.25f, realWidthcm*0.75f),
		stream.FRandRange(realWidthcm*0.25f, realWidthcm*0.75f)));
	Coords i0gridIdx = Coords((int)(acceptedPointLocations[0].X*cellSizeInv), (int)(acceptedPointLocations[0].Y*cellSizeInv));
	grid[i0gridIdx.x][i0gridIdx.y] = 0; //point to initial point
	activePoints.Add(i0gridIdx);

	while (activePoints.Num() > 0)
	{
		//pick random active point
		int randActivePointIdx = stream.RandRange(0, activePoints.Num() - 1);
		FVector2D activePoint = acceptedPointLocations[grid
			[activePoints[randActivePointIdx].x][activePoints[randActivePointIdx].y]];
		FVector2D candidate;
		float radDist;
		bool candidateFound = false;
		Coords canGridIdx;
		for (int candidateCount = 0; candidateCount < 50; ++candidateCount)
		{
			radDist = stream.FRandRange(minRadiuscm, minRadiuscm*2.f);
			candidate = activePoint + FVector2D(radDist, 0.f).GetRotated(stream.FRandRange(0.f, 359.99f));

			// if we're out of bounds, throw candidate out
			if (candidate.X < 0.f || candidate.X >= realWidthcm || candidate.Y < 0.f || candidate.Y >= realWidthcm)
				continue;

			canGridIdx = Coords((int)(candidate.X*cellSizeInv), (int)(candidate.Y*cellSizeInv));

			if (grid[canGridIdx.x][canGridIdx.y] >= 0)
			{
				continue; //if this grid spot is occupied, it's too close, throw candidate out
			}

			//check neighbor of 8 cells
			candidateFound = true; //mark true until failure found
			for (int i = -2; i <= 2; ++i)
			{
				for (int j = -2; j <= 2; ++j)
				{
					if (i == j && i == 0)
						continue;
					if (canGridIdx.x + i < 0 || canGridIdx.x + i >= gridCount ||
						canGridIdx.y + j < 0 || canGridIdx.y + j >= gridCount)
						continue;

					if (grid[canGridIdx.x + i][canGridIdx.y + j] >= 0) //if neighbor point found
					{
						float neighborDist = FVector2D::Distance(candidate, acceptedPointLocations[grid[canGridIdx.x + i][canGridIdx.y + j]]);
						if (neighborDist < minRadiuscm)
						{
							candidateFound = false;
							goto exitloop;
						}
					}
				}
			}
			if (candidateFound) break;
		exitloop:;
		}

		//valid point, add to accepted list and active point list
		if (candidateFound)
		{
			int addedIdx = acceptedPointLocations.Add(candidate);
			grid[canGridIdx.x][canGridIdx.y] = addedIdx;
			activePoints.Add(canGridIdx);
		}
		else //failed to find new acceptable point, remove from active list
		{
			activePoints.RemoveAt(randActivePointIdx);
		}
	}
	///END Implementation of Fast Poisson Disk Sampling in Arbitrary Dimensions - R. Bridson (2007)

	//trim accepted points based on blend map and obtain Z coordinate
	FP.acceptedLocations3D = TArray<FVector>();
	float U16ToMeters = 1.f / SP.metersToU16;
	for (int i = 0; i < acceptedPointLocations.Num(); ++i)
	{
		const FVector2D& point = acceptedPointLocations[i];
		int patchIdx = SP.lSystem->patches.Find(FP.patch);
		//blend data coords as XX.YY, where XX is the landscape component, YY is cordinates within component
		float coordX = (point.X / (realWidthcm / SP.landscapeComponentCountSqrt));
		float coordY = (point.Y / (realWidthcm / SP.landscapeComponentCountSqrt));
		int compIdx = FMath::FloorToInt(coordY) * SP.landscapeComponentCountSqrt + FMath::FloorToInt(coordX); //landscape component
		int subIdx = FMath::FloorToInt(FMath::Frac(coordY) * SP.ComponentSizeVerts) * SP.ComponentSizeVerts + FMath::FloorToInt(FMath::Frac(coordX) * SP.ComponentSizeVerts);//idx inside landscape component
		bool removeInstance = false;
		float blendVal = SP.patchBlendData[compIdx][patchIdx][subIdx];

		//use RNG to toss out some % of instances based on blendVal linearly from 1.0 to 0.5
		if (blendVal < 0.5f) continue;
		else if (blendVal < 0.95f && (stream.GetFraction() / 2.f) + 0.5f > blendVal) continue;
		else
		{
			int sIntHeight = ((int)SP.heightMaps[compIdx][subIdx].R << 8) | ((int)SP.heightMaps[compIdx][subIdx].G);
			float terrainZ = (float)(sIntHeight - SP.zeroHeight) * U16ToMeters * 100.f; //to cm
			FP.acceptedLocations3D.Add(FVector(point, terrainZ));
		}
	}
	onCompletion.ExecuteIfBound(true); //succeeded in process
}
