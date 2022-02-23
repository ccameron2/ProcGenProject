#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "TerrainTile.h"

class PROCGENPROJECT_API FCustomWorker : public FRunnable
{
public:
	FCustomWorker(ATerrainTile* tile);
	virtual ~FCustomWorker();

	//Overridden from parent
	bool Init() override; //Setup
	uint32 Run() override; //Main
	void Stop() override; //Clean

	FRunnableThread* Thread;
	bool RunThread;
	bool InputReady;
private:

	ATerrainTile* Tile;
};
