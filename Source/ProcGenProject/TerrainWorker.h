#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "TerrainTile.h"

class PROCGENPROJECT_API FTerrainWorker : public FRunnable
{
public:
	FTerrainWorker(TArray<ATerrainTile*> tiles);
	virtual ~FTerrainWorker();

	//Overridden from parent
	bool Init() override; //Setup
	uint32 Run() override; //Main
	void Stop() override; //Clean

	FRunnableThread* Thread;
	bool RunThread = true;
	bool InputReady = false;
	bool ThreadComplete = false;

	TArray<ATerrainTile*> Tiles;

private:
};
