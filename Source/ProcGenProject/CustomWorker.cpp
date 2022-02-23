#include "CustomWorker.h"

#pragma region Main Thread

FCustomWorker::FCustomWorker(ATerrainTile* tile)
{
	Thread = FRunnableThread::Create(this, TEXT("Thread"));
	Tile = tile;
}

FCustomWorker::~FCustomWorker()
{
	if (Thread)
	{		
		//Wait for finish and destroy
		Thread->Kill();
		delete Thread;
	}
}
#pragma endregion

bool FCustomWorker::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("Thread Initialised"));

	return true;
}

uint32 FCustomWorker::Run()
{
	while (RunThread)
	{
		if (InputReady)
		{
			Tile->CreateMesh();
			InputReady = false;
			FPlatformProcess::Sleep(0.01f);
		}
	}
	return 0;
}

void FCustomWorker::Stop()
{
	//clean memory
	RunThread = false;
}