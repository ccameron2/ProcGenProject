#include "NormalsWorker.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"

#pragma region Main Thread

FNormalsWorker::FNormalsWorker(TArray<FVector> vertices, TArray<int32> triangles, TArray<FIndex3i> mcTriangles)
{
	Vertices = vertices;
	Triangles = triangles;
	MCTriangles = mcTriangles;
	Thread = FRunnableThread::Create(this, TEXT("Thread"));
}

FNormalsWorker::~FNormalsWorker()
{
	if (Thread)
	{	
		//Wait for finish and destroy
		Thread->Kill();
		UE_LOG(LogTemp, Warning, TEXT("Normals Thread Deleted"));
		delete Thread;
	}
}
#pragma endregion

bool FNormalsWorker::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("Normals Thread Initialised"));
	return true;
}

uint32 FNormalsWorker::Run()
{
	while (RunThread)
	{
		if (InputReady)
		{

			Normals.Init({ 0,0,0 }, Vertices.Num());

			//TArray< FVector2D > UV0;
			//TArray <FProcMeshTangent> Tangents;

			//UKismetProceduralMeshLibrary* kismet;
			//kismet->CalculateTangentsForMesh(Vertices, Triangles, UV0, Normals, Tangents);

			// Map of vertex to triangles in Triangles array
			TArray<TArray<int32>> VertToTriMap;
			VertToTriMap.Init(TArray<int32>{int32{ -1 }, int32{ -1 }, int32{ -1 },
											int32{ -1 }, int32{ -1 }, int32{ -1 },
											int32{ -1 }, int32{ -1 } }, Vertices.Num());

			// For each triangle for each vertex add triangle to vertex array entry
			for (int i = 0; i < Triangles.Num(); i++)
			{
				for (int j = 0; j < 8; j++)
				{
					if (VertToTriMap[Triangles[i]][j] < 0)
					{
						VertToTriMap[Triangles[i]][j] = i / 3;
						break;
					}
				}
			}

			//convert to findex3i to reuse normals code

			//TArray<FIndex3i> triangleIndexes;
			//triangleIndexes.Init(FIndex3i{ 0,0,0 }, Triangles.Num() / 3);

			//int triIndex = 0;
			//for (int i = 0; i < Triangles.Num() / 3; i+=3)
			//{
			//	triangleIndexes[triIndex].A = Triangles[i];
			//	triangleIndexes[triIndex].B = Triangles[i+1];
			//	triangleIndexes[triIndex].C = Triangles[i+2];
			//	triIndex++;
			//}

			//For each vertex collect the triangles that share it and calculate the face normal
			for (int i = 0; i < Vertices.Num(); i++)
			{
				for (auto& triangle : VertToTriMap[i])
				{
					//This shouldnt happen
					if (triangle < 0)
					{
						continue;
					}
					auto A = Vertices[MCTriangles[triangle].A];
					auto B = Vertices[MCTriangles[triangle].B];
					auto C = Vertices[MCTriangles[triangle].C];
					auto E1 = A - B;
					auto E2 = C - B;
					auto Normal = E1 ^ E2;
					Normal.Normalize();
					Normals[i] += Normal;
				}
			}

			//Average the face normals
			for (auto& normal : Normals)
			{
				normal.Normalize();
			}

			UE_LOG(LogTemp, Warning, TEXT("Normals Thread Finished"));
			InputReady = false;

			FPlatformProcess::Sleep(0.01f);
			
			RunThread = false;

		}
	}
	return 0;
}

void FNormalsWorker::Stop()
{
	//clean memory
	RunThread = false;
}