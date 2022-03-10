#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "VectorTypes.h"
#include "Generators/MarchingCubes.h"

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
	bool RunThread;
	bool InputReady = false;
	bool ThreadComplete = false;

	FAxisAlignedBox3d BoundingBox;
	FIndex3i* mcTriangles;
	FVector3d* mcVertices;
	int numTri;
	int numVert;
	FMarchingCubes MarchingCubes;
	

private:
};
