// Fill out your copyright notice in the DescrSiption page of Project Settings.
#include "TerrainTile.h"
#include <vector>

#include "MeshDescription.h"
#include "MeshDescriptionBuilder.h"
#include "StaticMeshAttributes.h"

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

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("smComp"), false);
	StaticMesh->SetupAttachment(ProcMesh);

	static ConstructorHelpers::FObjectFinder<UMaterial> TerrainMaterial(TEXT("Material'/Game/M_Terrain_Mesh'"));
	Material = TerrainMaterial.Object;
	StaticMesh->SetMaterial(0, Material);

}

// Called when the game starts or when spawned
void ATerrainTile::BeginPlay()
{
	Super::BeginPlay();
}

void ATerrainTile::Init(int cubeSize, float seed, int scale, int chunkSize, int chunkHeight, int octaves, float surfaceFrequency, float caveFrequency,
						float noiseScale, int surfaceLevel, int caveLevel, int overallNoiseScale, int surfaceNoiseScale, bool generateCaves, float caveNoiseScale,
						float treeNoiseScale, int treeOctaves, float treeFrequency, float treeNoiseValueLimit, TSubclassOf<class ATree> treeClass, TArray<UStaticMesh*> treeMeshList,
						float rockNoiseScale, int rockOctaves, float rockFrequency, float rockNoiseValueLimit, TSubclassOf<class ARock> rockClass, TArray<UStaticMesh*> rockMeshList,
						int waterLevel, float grassNoiseScale, int grassOctaves, float grassFrequency, float grassNoiseValueLimit, TSubclassOf<class AGrass> grassClass,
						TArray<UStaticMesh*> grassMeshList, float animalNoiseScale, int animalOctaves, float animalFrequency, float animalNoiseValueLimit, TArray<TSubclassOf<AAnimal>> animalClassList,
						bool useStaticMesh)
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
	TreeNoiseValueLimit = treeNoiseValueLimit;
	TreeClass = treeClass;
	TreeMeshList = treeMeshList;
	GenerateCaves = generateCaves;
	RockNoiseScale = rockNoiseScale;
	RockOctaves = rockOctaves;
	RockFrequency = rockFrequency;
	RockNoiseValueLimit = rockNoiseValueLimit;
	RockClass = rockClass;
	RockMeshList = rockMeshList;
	WaterLevel = waterLevel;
	GrassNoiseScale = grassNoiseScale;
	GrassOctaves = grassOctaves;
	GrassFrequency = grassFrequency;
	GrassNoiseValueLimit = grassNoiseValueLimit;
	GrassClass = grassClass;
	GrassMeshList = grassMeshList;
	AnimalNoiseScale = animalNoiseScale;
	AnimalOctaves = animalOctaves;
	AnimalFrequency = animalFrequency;
	AnimalNoiseValueLimit = animalNoiseValueLimit;
	AnimalClassList = animalClassList;
	UseStaticMesh = useStaticMesh;
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

		float rand = FMath::RandRange(100, 500);
		rand = rand / 200;
		//Scale the vertices and fix z fighting
		for (int i = 0; i < MCVertices.Num(); i++)
		{

			MCVertices[i] *= Scale;
			MCVertices[i] -= GetActorLocation();
			MCVertices[i].Z += rand;
		}

		Vertices = TArray<FVector>(MCVertices);
	
		CalculateNormals();
	}
}

void ATerrainTile::CreateProcMesh()
{
	if (UseStaticMesh)
	{
		FMeshDescription meshDesc;
		FStaticMeshAttributes Attributes(meshDesc);
		Attributes.Register();

		FMeshDescriptionBuilder meshDescBuilder;
		meshDescBuilder.SetMeshDescription(&meshDesc);
		meshDescBuilder.EnablePolyGroups();
		meshDescBuilder.SetNumUVLayers(1);


		TArray< FVertexID > vertexIDs;
		vertexIDs.Init(FVertexID(0), Vertices.Num());

		for (int i = 0; i < Vertices.Num(); i++)
		{
			vertexIDs[i] = meshDescBuilder.AppendVertex(Vertices[i]);
		}

		TArray< FVertexInstanceID > vertexInstances;

		VertexColour.Init(FVector4{ 1,1,1,1 }, Vertices.Num());
		UV0.Init(FVector2D{ 0,0 }, Vertices.Num());
		for (auto& triangle : MCTriangles)
		{
			FVertexInstanceID instance = meshDescBuilder.AppendInstance(vertexIDs[triangle.A]);
			meshDescBuilder.SetInstanceNormal(instance, Normals[triangle.A]);
			meshDescBuilder.SetInstanceUV(instance, UV0[triangle.A]);
			meshDescBuilder.SetInstanceColor(instance, VertexColour[triangle.A]);
			vertexInstances.Add(instance);

			instance = meshDescBuilder.AppendInstance(vertexIDs[triangle.B]);
			meshDescBuilder.SetInstanceNormal(instance, Normals[triangle.B]);
			meshDescBuilder.SetInstanceUV(instance, UV0[triangle.B]);
			meshDescBuilder.SetInstanceColor(instance, VertexColour[triangle.B]);
			vertexInstances.Add(instance);

			instance = meshDescBuilder.AppendInstance(vertexIDs[triangle.C]);
			meshDescBuilder.SetInstanceNormal(instance, Normals[triangle.C]);
			meshDescBuilder.SetInstanceUV(instance, UV0[triangle.C]);
			meshDescBuilder.SetInstanceColor(instance, VertexColour[triangle.C]);
			vertexInstances.Add(instance);
		}

		// Allocate a polygon group
		FPolygonGroupID polygonGroup = meshDescBuilder.AppendPolygonGroup();
		
		for (int i = 0; i < vertexInstances.Num() / 3; i++)
		{
			meshDescBuilder.AppendTriangle(vertexInstances[(i * 3)], vertexInstances[(i * 3) + 1], vertexInstances[i * 3 + 2], polygonGroup);
		}
		
		UStaticMesh* staticMesh = NewObject<UStaticMesh>(this);
		staticMesh->GetStaticMaterials().Add(FStaticMaterial());

		UStaticMesh::FBuildMeshDescriptionsParams mdParams;
		mdParams.bBuildSimpleCollision = true;
		mdParams.bAllowCpuAccess = true;


		// Build static mesh
		TArray<const FMeshDescription*> meshDescPtrs;
		meshDescPtrs.Emplace(&meshDesc);
		staticMesh->BuildFromMeshDescriptions(meshDescPtrs, mdParams);
		staticMesh->bAllowCPUAccess = 1;

		StaticMesh->SetStaticMesh(staticMesh);
		StaticMesh->SetMaterial(0,Material);
	}
	else
	{
		ProcMesh->ClearAllMeshSections();
		ProcMesh->SetMaterial(0, Material);
		ProcMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, ProcVertexColour, Tangents, CreateCollision);
	}
		
	CreateTrees();
	CreateRocks();
	CreateGrass();
	CreateAnimals();
}

double ATerrainTile::PerlinWrapper(FVector3<double> perlinInput)
{
	//Scale noise input
	FVector3d noiseInput = (perlinInput + FVector{ Seed,Seed,0 }) / NoiseScale;

	float density = ( -noiseInput.Z / OverallNoiseScale ) + 1;
	
	//Add 2D noise
	density += FractalBrownianMotion(FVector(noiseInput.X / SurfaceNoiseScale, noiseInput.Y / SurfaceNoiseScale, 0), Octaves, SurfaceFrequency); //14

	float density2 = FractalBrownianMotion(FVector(noiseInput / CaveNoiseScale), Octaves, CaveFrequency);


	if (GenerateCaves)
	{
		if (perlinInput.Z < 1)//Cave floors
		{
			return 1;
		}

		if (perlinInput.Z >= SurfaceLevel)
		{
			return density;
		}
		else if (perlinInput.Z < CaveLevel)
		{
			return density2;
		}
		else
		{
			return FMath::Lerp(density2 + 0.2f, density, (perlinInput.Z - CaveLevel) / (SurfaceLevel - CaveLevel));
		}
		
	}
	else
	{
		if (perlinInput.Z == CaveLevel)
		{
			return 1;
		}

		if (perlinInput.Z >= SurfaceLevel)
		{
			return density;
		}
		else if (perlinInput.Z < CaveLevel)
		{
			return -1;
		}
		else
		{
			return FMath::Lerp(density2 + 0.1f, density, (perlinInput.Z - CaveLevel) / (SurfaceLevel - CaveLevel));
		}
	}

	return 1;
}

float ATerrainTile::FractalBrownianMotion(FVector fractalInput, float octaves, float frequency)
{
	// The book of shaders
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

			// Get vertices from triangle index
			auto A = Vertices[MCTriangles[triangle].A];
			auto B = Vertices[MCTriangles[triangle].B];
			auto C = Vertices[MCTriangles[triangle].C];

			//Calculate edges
			auto E1 = A - B;
			auto E2 = C - B;

			//Calculate normal with cross product
			auto Normal = E1 ^ E2;

			// Normalise result and add to normals array
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

void ATerrainTile::RemoveGrass()
{
	for (auto& grass : GrassList)
	{
		grass->Destroy();
	}
}

void ATerrainTile::RemoveAnimals()
{
	for (auto& animal : AnimalList)
	{
		animal->Destroy();
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


void ATerrainTile::CreateGrass()
{
	for (int i = -GridSizeX / 2; i < GridSizeX / 2; i += 8)
	{
		for (int j = -GridSizeY / 2; j < GridSizeY / 2; j += 8)
		{
			float grassNoise = FractalBrownianMotion(FVector{ GetActorLocation().X + float(i),GetActorLocation().Y + float(j),0 } / GrassNoiseScale, GrassOctaves, GrassFrequency);
			if (grassNoise > GrassNoiseValueLimit)
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
						AGrass* grass = GetWorld()->SpawnActor<AGrass>(GrassClass, Location, Rotation, SpawnParams);
						if (grass != nullptr)
						{
							grass->SetOwner(this);
							grass->SetActorScale3D(FVector{ float(Scale / 30), float(Scale / 30), float(Scale / 30) });
							grass->GrassMesh->SetStaticMesh(GrassMeshList[FMath::RandRange(0, GrassMeshList.Num() - 2)]);
							GrassList.Push(grass);
						}
					}
					else
					{
						FRotator Rotation = { 0,float(FMath::Rand()),0 };
						FActorSpawnParameters SpawnParams;
						//SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
						Location = { Location.X,Location.Y,float((WaterLevel + 0.5) * Scale) };
						AGrass* grass = GetWorld()->SpawnActor<AGrass>(GrassClass, Location, Rotation, SpawnParams);
						if (grass != nullptr)
						{
							grass->SetOwner(this);
							grass->SetActorScale3D(FVector{ float(Scale / 30), float(Scale / 30), float(Scale / 30) });
							grass->GrassMesh->SetStaticMesh(GrassMeshList[GrassMeshList.Num() - 1]); //Lilypads
							GrassList.Push(grass);
						}
					}
				}

			}
		}
	}
}



void ATerrainTile::CreateRocks()
{
	for (int i = -GridSizeX / 2; i < GridSizeX / 2; i += 8)
	{
		for (int j = -GridSizeY / 2; j < GridSizeY / 2; j += 8)
		{
			float rockNoise = FractalBrownianMotion(FVector{ GetActorLocation().X + float(i),GetActorLocation().Y + float(j),0 } / RockNoiseScale, RockOctaves, RockFrequency);
			if (rockNoise > RockNoiseValueLimit)
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

void ATerrainTile::CreateAnimals()
{
	for (int i = -GridSizeX / 2; i < GridSizeX / 2; i += 8)
	{
		for (int j = -GridSizeY / 2; j < GridSizeY / 2; j += 8)
		{
			float animalNoise = FractalBrownianMotion(FVector{ GetActorLocation().X + float(i),GetActorLocation().Y + float(j),0 } / AnimalNoiseScale, AnimalOctaves, AnimalFrequency);
			if (animalNoise > AnimalNoiseValueLimit)
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
						AAnimal* animal = GetWorld()->SpawnActor<AAnimal>(AnimalClassList[FMath::RandRange(0,AnimalClassList.Num()-1)],
							(Location - FVector{0,0,float(-Scale)}), Rotation, SpawnParams);
						if (animal != nullptr)
						{
							animal->SetOwner(this);
							animal->SetActorScale3D(FVector{ float(Scale / 30), float(Scale / 30), float(Scale / 30) });
							AnimalList.Push(animal);
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