#include "TerrainWorker.h"

#pragma region Main Thread

FTerrainWorker::FTerrainWorker(TArray<ATerrainTile*> tiles)
{
	Tiles = tiles;
	Thread = FRunnableThread::Create(this, TEXT("Thread"));
}

FTerrainWorker::~FTerrainWorker()
{
	if (Thread)
	{	
		//Wait for finish and destroy
		Thread->Kill();
		UE_LOG(LogTemp, Warning, TEXT("Terrain Thread Deleted"));
		delete Thread;
	}
}
#pragma endregion

bool FTerrainWorker::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("Terrain Thread Initialised"));
	return true;
}

uint32 FTerrainWorker::Run()
{
	
	while (RunThread)
	{
		if (!ThreadComplete)
		{
			if (CriticalSection.TryLock())
			{
				for (auto& tile : Tiles)
				{
					if (!tile->MeshCreated)
					{
						tile->GenerateTerrain();
					}
				}
				InputReady = false;
				ThreadComplete = true;
			}
			CriticalSection.Unlock();
		}		
	}
	return 0;
}

void FTerrainWorker::Stop()
{
	//clean memory
	RunThread = false;
}

void FTerrainWorker::InputTiles(TArray<ATerrainTile*> tiles)
{
	Tiles = tiles;
}
