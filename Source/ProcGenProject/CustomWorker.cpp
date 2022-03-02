#include "CustomWorker.h"
#include "TerrainTile.h"
#include "Generators/MarchingCubes.h"
#include "Misc/ScopeLock.h"

#pragma region Main Thread

FCustomWorker::FCustomWorker(FAxisAlignedBox3d boundingBox)
{
	Thread = FRunnableThread::Create(this, TEXT("Thread"));
	BoundingBox = boundingBox;
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
			//Marching Cubes
			FMarchingCubes MarchingCubes;
			MarchingCubes.Bounds = BoundingBox;
			MarchingCubes.bParallelCompute = true;
			MarchingCubes.Implicit = ATerrainTile::PerlinWrapper;
			MarchingCubes.CubeSize = 8;
			MarchingCubes.IsoValue = 0;
			MarchingCubes.Generate();
			mcTriangles = MarchingCubes.Triangles;
			mcVertices = MarchingCubes.Vertices;
			UE_LOG(LogTemp, Warning, TEXT("Thread Finished"));
			InputReady = false;
			FPlatformProcess::Sleep(0.01f);
			
			RunThread = false;
		}
	}
	return 0;
}

void FCustomWorker::Stop()
{
	//clean memory
	RunThread = false;
}