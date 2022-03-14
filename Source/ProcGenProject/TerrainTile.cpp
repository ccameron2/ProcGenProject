// Fill out your copyright notice in the DescrSiption page of Project Settings.
#include "TerrainTile.h"
#include "KismetProceduralMeshLibrary.h"

#include "delaunator.hpp"

int ATerrainTile::CubeSize = 0;
float ATerrainTile::Seed = 0;
int ATerrainTile::Scale = 0;
int ATerrainTile::Octaves = 0;
float ATerrainTile::SurfaceFrequency = 0;
float ATerrainTile::CaveFrequency = 0;
int ATerrainTile::NoiseScale = 0;
int ATerrainTile::SurfaceLevel = 0;
int ATerrainTile::CaveLevel = 0;
int ATerrainTile::OverallNoiseScale = 0;
int ATerrainTile::SurfaceNoiseScale = 0;
bool ATerrainTile::GenerateCaves = false;
int ATerrainTile::CaveNoiseScale = 0;

// Sets default values
ATerrainTile::ATerrainTile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create Procedural mesh and attach to root component
	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Procedural Mesh"));
	SetRootComponent(ProcMesh);

/*	RuntimeMesh = CreateDefaultSubobject<URuntimeMeshComponentStatic>(TEXT("Runtime Mesh"));
	SetRootComponent(RuntimeMesh)*/;

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

void ATerrainTile::Init(
	int cubeSize, float seed, int scale, int chunkSize, int chunkHeight, int octaves, float surfaceFrequency, float caveFrequency,
		float noiseScale, int surfaceLevel, int caveLevel, int overallNoiseScale, int surfaceNoiseScale, bool generateCaves, float caveNoiseScale,
			float treeNoiseScale, int treeOctaves, float treeFrequency, float treeNoiseValueLimit, int waterLevel, float waterNoiseScale, int waterOctaves,
				float waterFrequency, float waterNoiseValueLimit, float rockNoiseScale, int rockOctaves, float rockFrequency, float rockNoiseValueLimit)
{
	CubeSize = cubeSize;
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
	OverallNoiseScale = overallNoiseScale;
	SurfaceNoiseScale = surfaceNoiseScale;
	CaveNoiseScale = caveNoiseScale;
	TreeNoiseScale = treeNoiseScale;
	TreeOctaves = treeOctaves;
	TreeFrequency = treeFrequency;
	WaterLevel = waterLevel;
	TreeNoiseValueLimit = treeNoiseValueLimit;
	GenerateCaves = generateCaves;
	WaterNoiseScale = waterNoiseScale;
	WaterOctaves = waterOctaves;
	WaterFrequency = waterFrequency;
	WaterNoiseValueLimit = waterNoiseValueLimit;
	RockNoiseScale = rockNoiseScale;
	RockOctaves = rockOctaves;
	RockFrequency = rockFrequency;
	RockNoiseValueLimit = rockNoiseValueLimit;

}

void ATerrainTile::GenerateTerrain()
{
	Triangles.Empty();
	Vertices.Empty();
	Normals.Empty();
	Tangents.Empty();
	UV0.Empty();

	FAxisAlignedBox3d BoundingBox(FVector3d(GetActorLocation()) - (FVector3d{ double(GridSizeX) , double(GridSizeY),0 } / 2), FVector3d(GetActorLocation() /*/ Scale*/) +
									FVector3d{ double(GridSizeX / 2) , double(GridSizeY / 2) , double(GridSizeZ) });

	std::unique_ptr<FMarchingCubes> MarchingCubes = std::make_unique<FMarchingCubes>();
	MarchingCubes->Bounds = BoundingBox;
	MarchingCubes->bParallelCompute = true;
	MarchingCubes->Implicit = ATerrainTile::PerlinWrapper;
	MarchingCubes->CubeSize = CubeSize;
	MarchingCubes->IsoValue = 0;
	MarchingCubes->Generate();

	MCTriangles = MarchingCubes->Triangles;
	MCVertices = MarchingCubes->Vertices;

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
			MCVertices[i] -= GetActorLocation();
		}

		Vertices = TArray<FVector>(MCVertices);
	
		CalculateNormals();
	}
}

void ATerrainTile::CreateProcMesh()
{
	ProcMesh->ClearAllMeshSections();
	ProcMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColour, Tangents, CreateCollision);
	
	/*RuntimeMesh->SetupMaterialSlot(0, TEXT("TerrainMat"), Material);
	RuntimeMesh->CreateSectionFromComponents(0, sectionCount++, 0, Vertices, Triangles, Normals, UV0, VertexColour, Tangents, ERuntimeMeshUpdateFrequency::Infrequent,CreateCollision);*/
	CreateTrees();
	CreateWaterMesh();
	CreateRocks();
}

double ATerrainTile::PerlinWrapper(FVector3<double> perlinInput)
{
	//Scale noise input
	FVector3d noiseInput = (perlinInput + FVector{ Seed,Seed,0 }) / NoiseScale;

	float density = ( -noiseInput.Z / OverallNoiseScale ) + 1;

	//Add 3D noise partially
	//density += FractalBrownianMotion(FVector(noiseInput) / 5, 6,0.5);
	
	//Add 2D noise
	density += FractalBrownianMotion(FVector(noiseInput.X / SurfaceNoiseScale, noiseInput.Y / SurfaceNoiseScale, 0), Octaves, SurfaceFrequency); //14

	float density2 = FractalBrownianMotion(FVector(noiseInput / CaveNoiseScale), Octaves, CaveFrequency);

	//if (FractalBrownianMotion(FVector{ float(noiseInput.X),float(noiseInput.Y),0 } / 8, 4, 0.8) > 0.4)
	//{
	//	if (perlinInput.Z >= SurfaceLevel * Scale)
	//	{
	//		return density - 0.2;
	//	}
	//}

	if (GenerateCaves)
	{
		if (perlinInput.Z < 1)//Cave floors
		{
			return 1;
		}

		if (perlinInput.Z >= SurfaceLevel)//640
		{
			return density;
		}
		else if (perlinInput.Z < CaveLevel)//384
		{
			return density2;
		}
		else
		{
			return FMath::Lerp(density2 + 0.2f, density, (perlinInput.Z - CaveLevel) / (SurfaceLevel - CaveLevel)); //0.1
		}
		
	}
	else
	{
		if (perlinInput.Z == CaveLevel)//384
		{
			return 1;
		}

		if (perlinInput.Z >= SurfaceLevel)//640
		{
			return density;
		}
		else if (perlinInput.Z < CaveLevel)//384
		{
			return -1;
		}
		else
		{
			return FMath::Lerp(density2 + 0.1f, density, (perlinInput.Z - CaveLevel) / (SurfaceLevel - CaveLevel)); //0.1
		}
	}
	

	return 1;

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
	for (int i = -GridSizeX / 2; i < GridSizeX / 2; i+=8)
	{
		for (int j = -GridSizeY / 2; j < GridSizeY / 2; j += 8)
		{
			float treeNoise = FractalBrownianMotion(FVector{ GetActorLocation().X + float(i),GetActorLocation().Y + float(j),0 } / TreeNoiseScale,TreeOctaves,TreeFrequency);
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
							tree->SetOwner(this);
							tree->SetActorScale3D(FVector{ float(Scale / 5), float(Scale / 5), float(Scale / 5) });
							tree->TreeMesh->SetStaticMesh(TreeMeshList[FMath::RandRange(0, TreeMeshList.Num() - 1)]);
							TreeList.Push(tree);
						}
					}			
				}
				
			}
		}
	}
}

void ATerrainTile::RemoveTrees()
{
	for (auto& tree : TreeList)
	{
		tree->Destroy();
	}
}


void ATerrainTile::RemoveRocks()
{
	for (auto& rock : RockList)
	{
		rock->Destroy();
	}
}

void ATerrainTile::CreateWaterMesh()
{

	for (int i = -GridSizeX / 2; i < GridSizeX / 2; i++)
	{
		for (int j = -GridSizeY / 2; j < GridSizeY / 2; j++)
		{
			//5,4,0.4,0.25
			float waterNoise = FractalBrownianMotion(FVector{ GetActorLocation().X + float(i),GetActorLocation().Y + float(j),0 } / WaterNoiseScale , WaterOctaves, WaterFrequency);
			if (waterNoise > WaterNoiseValueLimit)
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

	for (auto& vertex : WaterVertices)
	{
		vertex -= GetActorLocation();
	}

	UKismetProceduralMeshLibrary* kismet;
	kismet->CalculateTangentsForMesh(WaterVertices, WaterTriangles, WaterUV0, WaterNormals, WaterTangents);

	ProcMesh->CreateMeshSection(1, WaterVertices, WaterTriangles, WaterNormals, WaterUV0, WaterVertexColour, WaterTangents, false);
	ProcMesh->SetMaterial(1, WaterMeshMaterial);
}

void ATerrainTile::CreateRocks()
{
	for (int i = -GridSizeX / 2; i < GridSizeX / 2; i += 8)
	{
		for (int j = -GridSizeY / 2; j < GridSizeY / 2; j += 8)
		{
			float treeNoise = FractalBrownianMotion(FVector{ GetActorLocation().X + float(i),GetActorLocation().Y + float(j),0 } / RockNoiseScale, RockOctaves, RockFrequency);
			if (treeNoise > RockNoiseValueLimit)
			{
				FHitResult Hit;
				FVector Start = { float((GetActorLocation().X * Scale) + (i * Scale)),float((GetActorLocation().Y * Scale) + (j * Scale)), float(GridSizeZ * Scale) };
				FVector End = { float((GetActorLocation().X * Scale) + (i * Scale)),float((GetActorLocation().Y * Scale) + (j * Scale)), float(CaveLevel * Scale) };
				ECollisionChannel Channel = ECC_Visibility;
				FCollisionQueryParams Params;
				ActorLineTraceSingle(Hit, Start, End, Channel, Params);
				FVector Location = Hit.Location + FVector{ 0,0,float(Scale / 2) };
				if (Hit.Location != FVector{ 0, 0, 0 })
				{
					if (Hit.Location.Z > (WaterLevel)*Scale)
					{
						FRotator Rotation = { 0,float(FMath::Rand()),0 };
						FActorSpawnParameters SpawnParams;
						//SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
						ARock* rock = GetWorld()->SpawnActor<ARock>(RockClass, (Location - FVector{0,0,float( - Scale)}), Rotation, SpawnParams);
						if (rock != nullptr)
						{
							rock->SetOwner(this);
							rock->SetActorScale3D(FVector{ float(Scale / 10), float(Scale / 10), float(Scale / 10) });
							rock->RockMesh->SetStaticMesh(RockMeshList[FMath::RandRange(0, RockMeshList.Num() - 1)]);
							RockList.Push(rock);
						}
					}
				}

			}
		}
	}
}

// Called every frame
void ATerrainTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATerrainTile::OnConstruction(const FTransform& Transform)
{
}