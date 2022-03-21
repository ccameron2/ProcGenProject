// Fill out your copyright notice in the DescrSiption page of Project Settings.
#include "TerrainTile.h"
#include <vector>

#include "MeshDescription.h"
#include "MeshDescriptionBuilder.h"
#include "StaticMeshAttributes.h"

// Declare static variables for use in perlin wrapper function
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

	// Create Procedural mesh and attach to root component
	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Procedural Mesh"));
	SetRootComponent(ProcMesh);

	// Create static mesh and attach to procedural mesh
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("smComp"), false);
	StaticMesh->SetupAttachment(ProcMesh);

	// Get material by name from editor and set as mesh material
	static ConstructorHelpers::FObjectFinder<UMaterial> TerrainMaterial(TEXT("Material'/Game/M_Terrain_Mesh'"));
	Material = TerrainMaterial.Object;
	StaticMesh->SetMaterial(0, Material);

}

// Called when the game starts or when spawned
void ATerrainTile::BeginPlay()
{
	Super::BeginPlay();
}

// Receive parameters from manager
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

// Run marching cubes algorithm and generate mesh parameters
void ATerrainTile::GenerateTerrain()
{
	// Empty variables 
	Triangles.Empty();
	Vertices.Empty();
	Normals.Empty();
	Tangents.Empty();
	UV0.Empty();

	// Create bounding box to run marching cubes in
	FAxisAlignedBox3d BoundingBox(FVector3d(GetActorLocation()) - (FVector3d{ double(GridSizeX) , double(GridSizeY),0 } / 2), FVector3d(GetActorLocation() /*/ Scale*/) +
									FVector3d{ double(GridSizeX / 2) , double(GridSizeY / 2) , double(GridSizeZ) });

	// Create marching cubes object
	std::unique_ptr<FMarchingCubes> MarchingCubes = std::make_unique<FMarchingCubes>();
	
	// Set bounding box 
	MarchingCubes->Bounds = BoundingBox;
	
	// Allow parallel processing to improve performance
	MarchingCubes->bParallelCompute = true;
	
	// Set function to evaluate for density values
	MarchingCubes->Implicit = ATerrainTile::PerlinWrapper;

	// Set size of cubes that are marching
	MarchingCubes->CubeSize = CubeSize;

	// Set value to place surface
	MarchingCubes->IsoValue = 0;

	// Generate values to build mesh
	MarchingCubes->Generate();

	// Copy triangles and vertices from marching cubes object
	MCTriangles = MarchingCubes->Triangles;
	MCVertices = MarchingCubes->Vertices;

	// If there are vertices in the array
	if (MCVertices.Num() > 0)
	{
		//Convert FIndex3i to Int32 for mesh generation
		for (int i = 0; i < MCTriangles.Num(); i++)
		{
			Triangles.Push(int32(MCTriangles[i].A));
			Triangles.Push(int32(MCTriangles[i].B));
			Triangles.Push(int32(MCTriangles[i].C));
		}

		// Generate a random value for this tile's mesh to be offset in Z direction
		// This fixes flickering caused by Z-Fighting
		float rand = FMath::RandRange(100, 500);
		rand = rand / 200;

		// For each vertex
		for (int i = 0; i < MCVertices.Num(); i++)
		{
			// Multiply by Scale
			MCVertices[i] *= Scale;

			// Move to correct location relative to actor
			MCVertices[i] -= GetActorLocation();

			// Offset by random Z amount to fix Z-Fighting
			MCVertices[i].Z += rand;
		}

		// Copy adjusted vertices
		Vertices = TArray<FVector>(MCVertices);
		
		// Calculate normals for mesh
		CalculateNormals();
	}
}

void ATerrainTile::CreateProcMesh()
{
	// If static mesh should be created instead of procedural mesh
	if (UseStaticMesh)
	{
		// Create mesh description and attributes to hold mesh information
		FMeshDescription meshDesc;
		FStaticMeshAttributes Attributes(meshDesc);
		Attributes.Register();

		// Create mesh description builder to populate mesh description
		FMeshDescriptionBuilder meshDescBuilder;
		meshDescBuilder.SetMeshDescription(&meshDesc);

		// Enable polygon groups and set the number UV layers to 1
		meshDescBuilder.EnablePolyGroups();
		meshDescBuilder.SetNumUVLayers(1);

		// Create an array to hold vertex IDs and initialise it to 0s
		TArray< FVertexID > vertexIDs;
		vertexIDs.Init(FVertexID(0), Vertices.Num());

		// For each vertex, get an ID from the builder
		for (int i = 0; i < Vertices.Num(); i++)
		{
			vertexIDs[i] = meshDescBuilder.AppendVertex(Vertices[i]);
		}
		
		// Create an array to hold vertex instances and initialise
		TArray< FVertexInstanceID > vertexInstances;
		VertexColour.Init(FVector4{ 1,1,1,1 }, Vertices.Num());

		// Initialise UV0 array
		UV0.Init(FVector2D{ 0,0 }, Vertices.Num());

		// For each triangle (FIndex3i)
		for (auto& triangle : MCTriangles)
		{ 
			// Create a vertex instance from the builder with each triangle index
			// This includes normals generated earlier. UV and Color set to 0
			// Add each instance to instances array
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
		
		// For each instance append the relevant triangle
		for (int i = 0; i < vertexInstances.Num() / 3; i++)
		{
			meshDescBuilder.AppendTriangle(vertexInstances[(i * 3)], vertexInstances[(i * 3) + 1], vertexInstances[i * 3 + 2], polygonGroup);
		}
		
		// Create a new static mesh object for the component to hold
		UStaticMesh* staticMesh = NewObject<UStaticMesh>(this);

		// Get default material (required)
		staticMesh->GetStaticMaterials().Add(FStaticMaterial());

		// Create parameters for mesh generation
		UStaticMesh::FBuildMeshDescriptionsParams mdParams;
		mdParams.bBuildSimpleCollision = true;
		mdParams.bAllowCpuAccess = true;

		// Build static mesh from mesh description and parameters
		TArray<const FMeshDescription*> meshDescPtrs;
		meshDescPtrs.Emplace(&meshDesc);
		staticMesh->BuildFromMeshDescriptions(meshDescPtrs, mdParams);
		staticMesh->bAllowCPUAccess = 1;

		// Set static mesh component to hold this mesh
		StaticMesh->SetStaticMesh(staticMesh);

		// Set material of component to terrain material
		StaticMesh->SetMaterial(0,Material);
	}
	else
	{
		// Set material to terrain material and generate procedural mesh with parameters from marching cubes and calculated normals
		ProcMesh->ClearAllMeshSections();
		ProcMesh->SetMaterial(0, Material);
		ProcMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, ProcVertexColour, Tangents, CreateCollision);
	}
	
	// Generate scenery
	CreateTrees();
	CreateRocks();
	CreateGrass();
	CreateAnimals();
}

double ATerrainTile::PerlinWrapper(FVector3<double> perlinInput)
{
	// Scale noise input
	FVector3d noiseInput = (perlinInput + FVector{ Seed,Seed,0 }) / NoiseScale;

	//
	float density = ( -noiseInput.Z / OverallNoiseScale ) + 1;
	
	// Sample 2D noise for surface
	density += FractalBrownianMotion(FVector(noiseInput.X / SurfaceNoiseScale, noiseInput.Y / SurfaceNoiseScale, 0), Octaves, SurfaceFrequency); //14

	// Sample 3D noise for caves
	float density2 = FractalBrownianMotion(FVector(noiseInput / CaveNoiseScale), Octaves, CaveFrequency);

	// If caves should be generated
	if (GenerateCaves)
	{
		if (perlinInput.Z < 1)//Cave floors
		{
			return 1;
		}

		// Lerp between surface density and cave density based on the Z value
		// Surface level and Cave level set in editor to allow customisation
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
			// Boost cave density value slightly during lerp to partially fill holes
			return FMath::Lerp(density2 + 0.2f, density, (perlinInput.Z - CaveLevel) / (SurfaceLevel - CaveLevel));
		}
		
	}
	else
	{
		// Cave floors 
		if (perlinInput.Z == CaveLevel)
		{
			return 1;
		}

		// Return lerped values but return -1 below lerp area 
		// This disables caves but keeps surface blend interesting
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
	// https://thebookofshaders.com/13/
	// Fractal brownian motion applied to 3D perlin noise
	// Modified to also take octaves and frequency as an input to allow for reuse to generate multiple noise maps
	float result = 0;
	float amplitude = 0.5;
	float lacunarity = 2.0;
	float gain = 0.5;

	// Add iterations of noise at different frequencies to get more detail from perlin noise
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
	
	// For each vertex collect the triangles that share it and calculate the face normal
	for (int i = 0; i < Vertices.Num(); i++)
	{
		for (auto& triangle : VertToTriMap[i])
		{	
			// This shouldnt happen
			if (triangle < 0)
			{
				continue;
			}

			// Get vertices from triangle index
			auto A = Vertices[MCTriangles[triangle].A];
			auto B = Vertices[MCTriangles[triangle].B];
			auto C = Vertices[MCTriangles[triangle].C];

			// Calculate edges
			auto E1 = A - B;
			auto E2 = C - B;

			// Calculate normal with cross product
			auto Normal = E1 ^ E2;

			// Normalise result and add to normals array
			Normal.Normalize();
			Normals[i] += Normal;
		}
	}

	// Average the face normals
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

// Place trees on landscape
void ATerrainTile::CreateTrees()
{
	// Scan the tile's mesh area
	for (int i = -GridSizeX / 2; i < GridSizeX / 2; i += 8)
	{
		for (int j = -GridSizeY / 2; j < GridSizeY / 2; j += 8)
		{
			// Sample noise for tree placement with parameters specified in manager
			float treeNoise = FractalBrownianMotion(FVector{ GetActorLocation().X + float(i),GetActorLocation().Y + float(j),0 } / TreeNoiseScale,TreeOctaves,TreeFrequency);
			
			// If noise exceeds parameter value
			if (treeNoise > TreeNoiseValueLimit	)
			{
				// Perform a raycast from the top of the tile to the cave level
				FHitResult Hit;
				FVector Start = { float((GetActorLocation().X * Scale) + (i * Scale)),float((GetActorLocation().Y * Scale) + (j * Scale)), float(GridSizeZ * Scale) };
				FVector End = { float((GetActorLocation().X * Scale) + (i * Scale)),float((GetActorLocation().Y * Scale) + (j * Scale)), float(CaveLevel * Scale) };
				ECollisionChannel Channel = ECC_Visibility;
				FCollisionQueryParams Params;
				ActorLineTraceSingle(Hit, Start, End, Channel, Params);

				//Save the hit location and offset slightly in Z direction
				FVector Location = Hit.Location + FVector{0,0,float(Scale / 2)};
				if (Hit.Location != FVector{ 0, 0, 0 })
				{
					// If location is above water level
					if (Hit.Location.Z > (WaterLevel) * Scale)
					{
						// Create spawn parameters 
						FRotator Rotation = { 0,float(FMath::Rand()),0 };
						FActorSpawnParameters SpawnParams;

						// Spawn tree actor
						ATree* tree = GetWorld()->SpawnActor<ATree>(TreeClass, Location, Rotation, SpawnParams);
						if (tree != nullptr)
						{
							// Set owner of tree as this tile
							tree->SetOwner(this);

							// Scale the tree
							tree->SetActorScale3D(FVector{ float(Scale / 5), float(Scale / 5), float(Scale / 5) });
							
							// Pick a random mesh to use from list
							tree->TreeMesh->SetStaticMesh(TreeMeshList[FMath::RandRange(0, TreeMeshList.Num() - 1)]);

							// Push tree onto tree list
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
			// Sample noise for grass placement with parameters specified in manager
			float grassNoise = FractalBrownianMotion(FVector{ GetActorLocation().X + float(i),GetActorLocation().Y + float(j),0 } / GrassNoiseScale, GrassOctaves, GrassFrequency);
			
			// If noise exceeds parameter value
			if (grassNoise > GrassNoiseValueLimit)
			{
				// Perform a raycast from the top of the tile to the cave level
				FHitResult Hit;
				FVector Start = { float((GetActorLocation().X * Scale) + (i * Scale)),float((GetActorLocation().Y * Scale) + (j * Scale)), float(GridSizeZ * Scale) };
				FVector End = { float((GetActorLocation().X * Scale) + (i * Scale)),float((GetActorLocation().Y * Scale) + (j * Scale)), float(CaveLevel * Scale) };
				ECollisionChannel Channel = ECC_Visibility;
				FCollisionQueryParams Params;
				ActorLineTraceSingle(Hit, Start, End, Channel, Params);

				//Save the hit location and offset slightly in Z direction
				FVector Location = Hit.Location + FVector{ 0,0,float(Scale / 2) };
				if (Hit.Location != FVector{ 0, 0, 0 })
				{
					// If location is above water level
					if (Hit.Location.Z > (WaterLevel)*Scale)
					{
						// Create spawn parameters 
						FRotator Rotation = { 0,float(FMath::Rand()),0 };
						FActorSpawnParameters SpawnParams;

						// Spawn grass actor
						AGrass* grass = GetWorld()->SpawnActor<AGrass>(GrassClass, Location, Rotation, SpawnParams);
						if (grass != nullptr)
						{
							// Set owner of grass as this tile
							grass->SetOwner(this);

							// Scale the grass
							grass->SetActorScale3D(FVector{ float(Scale / 30), float(Scale / 30), float(Scale / 30) });

							// Pick a random mesh to use from list
							grass->GrassMesh->SetStaticMesh(GrassMeshList[FMath::RandRange(0, GrassMeshList.Num() - 2)]);
							GrassList.Push(grass);
						}
					}
					else // If under the water 
					{
						// Create spawn parameters 
						FRotator Rotation = { 0,float(FMath::Rand()),0 };
						FActorSpawnParameters SpawnParams;

						// Set location to slightly above water level
						Location = { Location.X,Location.Y,float((WaterLevel + 0.5) * Scale) };

						// Spawn grass actor
						AGrass* grass = GetWorld()->SpawnActor<AGrass>(GrassClass, Location, Rotation, SpawnParams);
						if (grass != nullptr)
						{
							// Set owner of grass to this tile
							grass->SetOwner(this);

							// Scale the grass
							grass->SetActorScale3D(FVector{ float(Scale / 30), float(Scale / 30), float(Scale / 30) });

							// Take the last mesh from the list to use as water foliage
							grass->GrassMesh->SetStaticMesh(GrassMeshList[GrassMeshList.Num() - 1]); //Lilypads

							// Push grass onto grass list
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
			// Sample noise for rock placement with parameters specified in manager
			float rockNoise = FractalBrownianMotion(FVector{ GetActorLocation().X + float(i),GetActorLocation().Y + float(j),0 } / RockNoiseScale, RockOctaves, RockFrequency);
			
			// If noise exceeds parameter value
			if (rockNoise > RockNoiseValueLimit)
			{
				// Perform a raycast from the top of the tile to the cave level
				FHitResult Hit;
				FVector Start = { float((GetActorLocation().X * Scale) + (i * Scale)),float((GetActorLocation().Y * Scale) + (j * Scale)), float(GridSizeZ * Scale) };
				FVector End = { float((GetActorLocation().X * Scale) + (i * Scale)),float((GetActorLocation().Y * Scale) + (j * Scale)), float(CaveLevel * Scale) };
				ECollisionChannel Channel = ECC_Visibility;
				FCollisionQueryParams Params;
				ActorLineTraceSingle(Hit, Start, End, Channel, Params);

				//Save the hit location and offset slightly in Z direction
				FVector Location = Hit.Location + FVector{ 0,0,float(Scale / 2) };
				if (Hit.Location != FVector{ 0, 0, 0 })
				{
					// If location is above water level
					if (Hit.Location.Z > (WaterLevel)*Scale)
					{
						// Create spawn parameters
						FRotator Rotation = { 0,float(FMath::Rand()),0 };
						FActorSpawnParameters SpawnParams;

						// Spawn rock actor
						ARock* rock = GetWorld()->SpawnActor<ARock>(RockClass, (Location - FVector{0,0,float( - Scale)}), Rotation, SpawnParams);
						if (rock != nullptr)
						{
							// Set owner of rock to this tile
							rock->SetOwner(this);

							// Scale the rock
							rock->SetActorScale3D(FVector{ float(Scale / 10), float(Scale / 10), float(Scale / 10) });

							// Pick a random mesh to use from list
							rock->RockMesh->SetStaticMesh(RockMeshList[FMath::RandRange(0, RockMeshList.Num() - 1)]);

							// Push rock onto rock list
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
			// Sample noise for animal placement with parameters specified in manager
			float animalNoise = FractalBrownianMotion(FVector{ GetActorLocation().X + float(i),GetActorLocation().Y + float(j),0 } / AnimalNoiseScale, AnimalOctaves, AnimalFrequency);
			
			// If noise exceeds parameter value
			if (animalNoise > AnimalNoiseValueLimit)
			{
				// Perform a raycast from the top of the tile to the cave level
				FHitResult Hit;
				FVector Start = { float((GetActorLocation().X * Scale) + (i * Scale)),float((GetActorLocation().Y * Scale) + (j * Scale)), float(GridSizeZ * Scale) };
				FVector End = { float((GetActorLocation().X * Scale) + (i * Scale)),float((GetActorLocation().Y * Scale) + (j * Scale)), float(CaveLevel * Scale) };
				ECollisionChannel Channel = ECC_Visibility;
				FCollisionQueryParams Params;
				ActorLineTraceSingle(Hit, Start, End, Channel, Params);

				//Save the hit location and offset slightly in Z direction
				FVector Location = Hit.Location + FVector{ 0,0,float(Scale / 2) };
				if (Hit.Location != FVector{ 0, 0, 0 })
				{
					// If location is above water level
					if (Hit.Location.Z > (WaterLevel)*Scale)
					{
						// Create spawn parameters
						FRotator Rotation = { 0,float(FMath::Rand()),0 };
						FActorSpawnParameters SpawnParams;

						// Spawn animal actor with random class from list set in editor
						AAnimal* animal = GetWorld()->SpawnActor<AAnimal>(AnimalClassList[FMath::RandRange(0,AnimalClassList.Num()-1)],
							(Location - FVector{0,0,float(-Scale)}), Rotation, SpawnParams);
						if (animal != nullptr)
						{
							// Set owner of animal as this tile
							animal->SetOwner(this);

							// Scale the animal
							animal->SetActorScale3D(FVector{ float(Scale / 30), float(Scale / 30), float(Scale / 30) });

							// Push animal onto animal list
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