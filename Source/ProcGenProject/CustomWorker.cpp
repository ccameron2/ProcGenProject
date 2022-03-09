#include "CustomWorker.h"

#pragma region Main Thread

FCustomWorker::FCustomWorker(FAxisAlignedBox3d boundingBox)
{
	BoundingBox = boundingBox;
	Thread = FRunnableThread::Create(this, TEXT("Thread"));
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
			MarchingCubes.Bounds = BoundingBox;
			MarchingCubes.bParallelCompute = true;
			MarchingCubes.Implicit = ATerrainTile::PerlinWrapper;
			MarchingCubes.CubeSize = 8;
			MarchingCubes.IsoValue = 0;
			MarchingCubes.Generate();

			numVert = MarchingCubes.Vertices.Num();
			numTri = MarchingCubes.Triangles.Num();

			mcVertices = new FVector3d[numVert];
			mcTriangles = new FIndex3i[numTri];

			for (int i= 0; i < numVert; i++)
			{
				mcVertices[i] = MarchingCubes.Vertices[i];
			}

			for (int i = 0; i < numTri; i++)
			{
				mcTriangles[i] = MarchingCubes.Triangles[i];
			}

			UE_LOG(LogTemp, Warning, TEXT("Thread Finished"));
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