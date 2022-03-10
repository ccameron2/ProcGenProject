// Fill out your copyright notice in the DescrSiption page of Project Settings.
#include "TerrainTile.h"
#include "KismetProceduralMeshLibrary.h"
#include "Tree.h"
#include "delaunator.hpp"

float ATerrainTile::Seed = 0;
int ATerrainTile::Scale = 0;
int ATerrainTile::Octaves = 0;
float ATerrainTile::SurfaceFrequency = 0;
float ATerrainTile::CaveFrequency = 0;
int ATerrainTile::NoiseScale = 0;
int ATerrainTile::SurfaceLevel = 0;
int ATerrainTile::CaveLevel = 0;
int ATerrainTile::SurfaceNoiseScale = 0;
int ATerrainTile::CaveNoiseScale = 0;

// Sets default values
ATerrainTile::ATerrainTile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create Procedural mesh and attach to root component
	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Procedural Mesh"));
	SetRootComponent(ProcMesh);

	WaterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Water Mesh"));
	WaterMesh->SetupAttachment(RootComponent);
	WaterMesh->SetRelativeLocation(FVector{ 0,0,float((SurfaceLevel * Scale) - (10 * Scale)) });

	static ConstructorHelpers::FObjectFinder<UMaterial> TerrainMaterial(TEXT("Material'/Game/M_Terrain_Mesh'"));
	Material = TerrainMaterial.Object;
	ProcMesh->SetMaterial(0, Material);

	static ConstructorHelpers::FObjectFinder<UMaterial> WaterMaterial(TEXT("Material'/Game/M_Water_Mesh'"));
	WaterMeshMaterial = WaterMaterial.Object;

}

// Called when the game starts or when spawned
void ATerrainTile::BeginPlay()
{
	Super::BeginPlay();
}

void ATerrainTile::Init(float seed, int scale, int chunkSize, int chunkHeight, int octaves, float surfaceFrequency, float caveFrequency, 
							int noiseScale, int surfaceLevel, int caveLevel, int surfaceNoiseScale, int caveNoiseScale,
								int treeNoiseScale, int treeOctaves, float treeFrequency, float treeNoiseValueLimit, int waterLevel)
{
	Seed = seed;
	Scale = scale;
	GridSizeX = chunkSize;
	GridSizeY = chunkSize;
	GridSizeZ = chunkHeight;
	Octaves = octaves;
	SurfaceFrequency = surfaceFrequency;
	CaveFrequency = caveFrequency;
	NoiseScale = noiseScale;
	SurfaceLevel = surfaceLevel;
	CaveLevel = caveLevel;
	SurfaceNoiseScale = surfaceNoiseScale;
	CaveNoiseScale = caveNoiseScale;
	TreeNoiseScale = treeNoiseScale;
	TreeOctaves = treeOctaves;
	TreeFrequency = treeFrequency;
	WaterLevel = waterLevel;
	TreeNoiseValueLimit = treeNoiseValueLimit;
}

void ATerrainTile::GenerateTerrain()
{
	Triangles.Empty();
	Vertices.Empty();
	Normals.Empty();
	Tangents.Empty();
	UV0.Empty();

	FAxisAlignedBox3d BoundingBox(GetActorLocation(), FVector3d(GetActorLocation()) + 
									FVector3d{ double(GridSizeX) , double(GridSizeY) , double(GridSizeZ) });
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
		CalculateNormals();
	}
}

void ATerrainTile::CreateProcMesh()
{
	ProcMesh->ClearAllMeshSections();
	ProcMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColour, Tangents, CreateCollision);
	CreateTrees();
	CreateWaterMesh();
}

double ATerrainTile::PerlinWrapper(FVector3<double> perlinInput)
{
	//Scale noise input
	FVector3d noiseInput = (perlinInput + FVector{ Seed,Seed,0 }) / NoiseScale;

	float density = ( -noiseInput.Z / 23 ) + 1;

	//Add 3D noise partially
	//density += FractalBrownianMotion(FVector(noiseInput) / 5, 6,0.5);
	
	//Add 2D noise
	density += FractalBrownianMotion(FVector(noiseInput.X / SurfaceNoiseScale, noiseInput.Y / SurfaceNoiseScale, 0), Octaves, SurfaceFrequency); //14

	float density2 = FractalBrownianMotion(FVector(noiseInput / CaveNoiseScale), Octaves, CaveFrequency);

	if (perlinInput.Z < 1)//Cave floors
	{
		return 1;
	}

	if (FractalBrownianMotion(FVector{ float(noiseInput.X),float(noiseInput.Y),0 } / 8, 4, 0.8) > 0.4)
	{
		if (perlinInput.Z >= SurfaceLevel * Scale)
		{
			return density - 0.2;
		}
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
		return FMath::Lerp(density2 + 0.2f, density, (perlinInput.Z - CaveLevel) / (SurfaceLevel - CaveLevel)); //0.1
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
				break;
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

void ATerrainTile::CreateTrees()
{
	for (int i = 0; i < GridSizeX; i+=8)
	{
		for (int j = 0; j < GridSizeY; j+=8)
		{
			float treeNoise = FractalBrownianMotion(FVector{ float(i),float(j),0 } / TreeNoiseScale,TreeOctaves,TreeFrequency);
			if (treeNoise > TreeNoiseValueLimit	)
			{
				FHitResult Hit;
				FVector Start = { float((GetActorLocation().X * Scale) + (i * Scale)),float((GetActorLocation().Y * Scale) + (j * Scale)), float(GridSizeZ * Scale) };
				FVector End = { float((GetActorLocation().X * Scale) + (i * Scale)),float((GetActorLocation().Y * Scale) + (j * Scale)), float(CaveLevel * Scale) };
				ECollisionChannel Channel = ECC_Visibility;
				FCollisionQueryParams Params;
				ActorLineTraceSingle(Hit, Start, End, Channel, Params);
				FVector Location = Hit.Location + FVector{0,0,float(Scale / 2)};
				if (Hit.Location != FVector{ 0, 0, 0 })
				{
					if (Hit.Location.Z > (WaterLevel) * Scale)
					{
						FRotator Rotation = { 0,float(FMath::Rand()),0 };
						FActorSpawnParameters SpawnParams;
						//SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
						ATree* tree = GetWorld()->SpawnActor<ATree>(TreeClass, Location, Rotation, SpawnParams);
						if (tree != nullptr)
						{
							tree->SetActorScale3D(FVector{ float(10), float(10), float(10) });
							tree->TreeMesh->SetStaticMesh(treeMeshList[FMath::RandRange(0, treeMeshList.Num() - 1)]);
							TreeList.Push(tree);
						}
					}			
				}
				
			}
		}
	}
}

void ATerrainTile::CreateWaterMesh()
{

	for (int i = 0; i < GridSizeX /*+ Scale*/; i++)
	{
		for (int j = 0; j < GridSizeY /*+ Scale*/; j++)
		{
			float waterNoise = FractalBrownianMotion(FVector{ float(i),float(j),0 } / 5, 4, 0.4);
			if (waterNoise > 0.25)
			{
				FHitResult Hit;
				FVector Start = { float((GetActorLocation().X * Scale) + (i * Scale)),float((GetActorLocation().Y * Scale) + (j * Scale)), float(GridSizeZ * Scale) };
				FVector End = { float((GetActorLocation().X * Scale) + (i * Scale)),float((GetActorLocation().Y * Scale) + (j * Scale)), float(CaveLevel * Scale) };
				ECollisionChannel Channel = ECC_Visibility;
				FCollisionQueryParams Params;
				ActorLineTraceSingle(Hit, Start, End, Channel, Params);
				FVector Offset = FVector{ 0,0,float(Scale * 1) };
				FVector Location = Hit.Location + Offset;
				if (Hit.Location != FVector{ 0, 0, 0 })
				{
					if (Hit.Location != Offset)
					{
						if (Hit.Location.Z < (WaterLevel) * Scale)
						{
							if (Hit.Location.Z < (WaterLevel - 20) * Scale)
							{
								Location.Z = (WaterLevel - 20) * Scale;
							}
							WaterVertices.Push(Location);
						}
					}
				}
			}
		}
	}

	std::vector<double> coords;

	bool inARowX = true;
	bool inARowY = true;
	float lastXVertex = 0;
	float lastYVertex = 0;

	for (auto& vertex : WaterVertices)
	{
		if (lastXVertex != 0)
		{
			if (vertex.X != lastXVertex)
			{

				inARowX = false;
			}
			if (vertex.Y != lastYVertex)
			{
				inARowY = false;
			}
		}		
		coords.push_back(vertex.X);
		coords.push_back(vertex.Y);
		lastXVertex = vertex.X;
		lastYVertex = vertex.Y;
	}

	if (coords.size() > 6)
	{
		if (!inARowX && !inARowY)
		{
			//triangulation happens here
			delaunator::Delaunator d(coords);

			for (auto& triangle : d.triangles)
			{
				WaterTriangles.Push(triangle);
			}
		}	
	}

	UKismetProceduralMeshLibrary* kismet;
	kismet->CalculateTangentsForMesh(WaterVertices, WaterTriangles, WaterUV0, WaterNormals, WaterTangents);

	ProcMesh->CreateMeshSection(1, WaterVertices, WaterTriangles, WaterNormals, WaterUV0, WaterVertexColour, WaterTangents, false);
	ProcMesh->SetMaterial(1, WaterMeshMaterial);
}

// Called every frame
void ATerrainTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATerrainTile::OnConstruction(const FTransform& Transform)
{
}