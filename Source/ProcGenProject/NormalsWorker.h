#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "VectorTypes.h"
#include "IndexTypes.h"

class PROCGENPROJECT_API FNormalsWorker : public FRunnable
{
public:
	FNormalsWorker(TArray<FVector> vertices, TArray<int32> triangles, TArray<FIndex3i> mcTriangles);
	virtual ~FNormalsWorker();

	//Overridden from parent
	bool Init() override; //Setup
	uint32 Run() override; //Main
	void Stop() override; //Clean

	FRunnableThread* Thread;
	bool RunThread;
	bool InputReady = false;
	bool ThreadComplete = false;

	TArray<FVector> Normals;
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FIndex3i> MCTriangles;
private:
};
