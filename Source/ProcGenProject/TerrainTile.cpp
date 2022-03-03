// Fill out your copyright notice in the DescrSiption page of Project Settings.
#include "TerrainTile.h"
#include <functional>
#include "KismetProceduralMeshLibrary.h"

FCustomWorker* mcWorker = nullptr;
float ATerrainTile::Seed = 0;
int ATerrainTile::Octaves = 0;
float ATerrainTile::SurfaceFrequency = 0;
float ATerrainTile::CaveFrequency = 0;
int ATerrainTile::NoiseScale = 0;
int ATerrainTile::SurfaceLevel = 0;
int ATerrainTile::CaveLevel = 0;

// Sets default values
ATerrainTile::ATerrainTile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create Procedural mesh and attach to root component
	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Procedural Mesh"));
	SetRootComponent(ProcMesh);
	static ConstructorHelpers::FObjectFinder<UMaterial> TerrainMaterial(TEXT("Material'/Game/M_Terrain_Mesh'"));
	Material = TerrainMaterial.Object;
	ProcMesh->SetMaterial(0, Material);

}

// Called when the game starts or when spawned
void ATerrainTile::BeginPlay()
{
	Super::BeginPlay();
}

//void ATerrainTile::CreateMesh()
//{
//	ProcMesh->ClearAllMeshSections();
//	Vertices.Empty();
//	for (int i = 0; i < GridSizeX; i++)
//	{
//		for (int j = 0; j < GridSizeY; j++)
//		{
//			Vertices.Push(FVector{float(i) * Scale,float(j) * Scale,0});
//		}
//	}
//	float counter = 0;
//	for (int i = 0; i < Vertices.Num(); i++)
//	{
//		counter += FMath::FRand();
//		Vertices[i].Z = Scale * FMath::PerlinNoise2D(FVector2D{ Vertices[i].X, Vertices[i].Y });/* FractalBrownianMotion(Vertices[i].X, Vertices[i].Y, Vertices[i].Z, NumOctaves, Lacunarity, Gain); */
//	}
//	AssignTriangles();
//	ProcMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColour, Tangents, CreateCollision);
//}

void ATerrainTile::Init(float seed, bool useCustomMultithreading, int octaves, float surfaceFrequency,
							float caveFrequency, int noiseScale, int surfaceLevel, int caveLevel)
{
	Seed = seed;
	UseCustomMultithreading = useCustomMultithreading;
	Octaves = octaves;
	SurfaceFrequency = surfaceFrequency;
	CaveFrequency = caveFrequency;
	NoiseScale = noiseScale;
	SurfaceLevel = surfaceLevel;
	CaveLevel = caveLevel;
}

double ATerrainTile::PerlinWrapper(FVector3<double> perlinInput)
{
	//Scale noise input
	FVector3d noiseInput = (perlinInput + FVector{ Seed,Seed,0 }) / NoiseScale;

	float density = ( -noiseInput.Z / 14) + 1;

	//Add 3D noise partially
	//density += FractalBrownianMotion(FVector(noiseInput) / 5, 6,0.5);
	
	//Add 2D noise
	density += FractalBrownianMotion(FVector(noiseInput.X / 10, noiseInput.Y / 10, 0), Octaves, SurfaceFrequency);


	//density = FMath::PerlinNoise2D(FVector2D(noiseInput.X, noiseInput.Y));

	float density2 = FractalBrownianMotion(FVector(noiseInput / 5), Octaves, CaveFrequency);

	if (perlinInput.Z < 1)//Cave floors
	{
		return 1;
	}

	if (perlinInput.Z >= SurfaceLevel)//640
	{
		return density;
	}
	else if(perlinInput.Z < CaveLevel)//384
	{
		return density2;
	}
	else
	{
		return FMath::Lerp(density2 + 0.1f, density, (perlinInput.Z - CaveLevel) / (SurfaceLevel - CaveLevel));
	}
}

float ATerrainTile::FractalBrownianMotion(FVector fractalInput, float octaves, float frequency)
{
	//The book of shaders
	float result = 0;
	float amplitude = 0.5;
	float lacunarity = 2.0;
	float gain = 0.5;

	for (int i = 0; i < octaves; i++)
	{
		result += amplitude * FMath::PerlinNoise3D(frequency * fractalInput);
		frequency *= lacunarity;
		amplitude *= gain;
	}

	return result;
}

void ATerrainTile::CreateMesh()
{
	Triangles.Empty();
	Vertices.Empty();
	Normals.Empty();
	Tangents.Empty();
	UV0.Empty();
	FAxisAlignedBox3d BoundingBox(GetActorLocation(), FVector3d(GetActorLocation()) + FVector3d{ double(GridSizeX) , double(GridSizeY) , double(GridSizeZ) });
	if (UseCustomMultithreading)
	{
		mcWorker = new FCustomWorker(BoundingBox);
		if (mcWorker)
		{
			mcWorker->InputReady = true;
			if (mcWorker->Init())
			{
				mcWorker->Run();
			}
		}
	}
	else
	{
		//Marching Cubes
		FMarchingCubes MarchingCubes;
		MarchingCubes.Bounds = BoundingBox;
		MarchingCubes.bParallelCompute = true;
		MarchingCubes.Implicit = ATerrainTile::PerlinWrapper;
		MarchingCubes.CubeSize = 8;
		MarchingCubes.IsoValue = 0;
		MarchingCubes.Generate();

		MCTriangles = MarchingCubes.Triangles;
		MCVertices = MarchingCubes.Vertices;

		if (MCVertices.Num() > 0)
		{
			//Convert FIndex3i to Int32
			for (int i = 0; i < MCTriangles.Num(); i++)
			{
				Triangles.Push(int32(MCTriangles[i].A));
				Triangles.Push(int32(MCTriangles[i].B));
				Triangles.Push(int32(MCTriangles[i].C));
			}

			//Scale the vertices
			for (int i = 0; i < MCVertices.Num(); i++)
			{
				MCVertices[i] *= Scale;
			}
			Vertices = TArray<FVector>(MCVertices);

			//UV0 = TArray<FVector2D>(MarchingCubes.UVs);

			//Calculate normals
			CalculateNormals();

			//Create Procedural Mesh Section with Marching Cubes data
			ProcMesh->ClearAllMeshSections();
			ProcMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColour, Tangents, CreateCollision);
		}
	}
}

void ATerrainTile::CalculateNormals()
{
	Normals.Init({ 0,0,0 }, Vertices.Num());

	// Map of vertex to triangles in Triangles array
	TArray<TArray<int32>> VertToTriMap;
	VertToTriMap.Init(TArray<int32>{ int32{ -1 }, int32{ -1 }, int32{ -1 },
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
			}
		}		
	}
	
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
	
}



//void ATerrainTile::AssignTriangles()
//{
//	Triangles.Init(0, GridSizeX * GridSizeY * 6);
//	UKismetProceduralMeshLibrary* procLib;
//	procLib->CreateGridMeshTriangles(GridSizeX,GridSizeY, false, Triangles);
//}

// Called every frame
void ATerrainTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (UseCustomMultithreading)
	{
		if (mcWorker)
		{
			if (mcWorker->InputReady == false)
			{
				//Get data from multithreading worker
				MCVertices = mcWorker->mcVertices;
				MCTriangles = mcWorker->mcTriangles;

				if (MCVertices.Num() > 0)
				{
					//Convert FIndex3i to Int32
					for (int i = 0; i < MCTriangles.Num(); i++)
					{
						Triangles.Push(int32(MCTriangles[i].A));
						Triangles.Push(int32(MCTriangles[i].B));
						Triangles.Push(int32(MCTriangles[i].C));
					}

					//Scale the vertices
					for (int i = 0; i < MCVertices.Num(); i++)
					{
						MCVertices[i] *= Scale;
					}
					Vertices = TArray<FVector>(MCVertices);

					//UV0 = TArray<FVector2D>(MarchingCubes.UVs);

					//Calculate normals
					CalculateNormals();

					//Create Procedural Mesh Section with Marching Cubes data
					ProcMesh->ClearAllMeshSections();
					ProcMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColour, Tangents, CreateCollision);
					if (mcWorker)
					{
						delete mcWorker;
					}
				}
			}
		}
	}
	
}

void ATerrainTile::OnConstruction(const FTransform& Transform)
{
}