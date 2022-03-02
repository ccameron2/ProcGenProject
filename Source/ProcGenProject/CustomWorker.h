#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "TerrainTile.h"

class PROCGENPROJECT_API FCustomWorker : public FRunnable
{
public:
	FCustomWorker(FAxisAlignedBox3d boundingBox);
	virtual ~FCustomWorker();

	//Overridden from parent
	bool Init() override; //Setup
	uint32 Run() override; //Main
	void Stop() override; //Clean

	FRunnableThread* Thread;
	FCriticalSection Lock_Location;
	bool RunThread;
	bool InputReady;
	FAxisAlignedBox3d BoundingBox;
	TArray<FIndex3i> mcTriangles;
	TArray<FVector3d> mcVertices;

private:
};
