// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainManager.h"
#include "Kismet\GameplayStatics.h"
#include <math.h>
#include "delaunator.hpp"

#include "MeshDescription.h"
#include "MeshDescriptionBuilder.h"
#include "StaticMeshAttributes.h"

// Sets default values
ATerrainManager::ATerrainManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a Procedural Mesh for the water and set it as the root component
	WaterMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Water Mesh"));
	WaterMesh->SetupAttachment(RootComponent);

	// Set material set in blueprint
	WaterMesh->SetMaterial(0, WaterMeshMaterial);
}

// Called when the game starts or when spawned
void ATerrainManager::BeginPlay()
{
	Super::BeginPlay();

	// Initialize tiles
	CreateTileArray();

	// Create multithreading worker with tile array
	terrainWorker = std::make_unique<FTerrainWorker>(TileArray);
}

FVector2D ATerrainManager::GetPlayerGridPosition()
{
	// Get player position and divide by chunksize * scale to get grid position
	FVector PlayerPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	FVector PlayerChunkPosition = PlayerPosition / (ChunkSize * Scale);
	return FVector2D(PlayerChunkPosition.X, PlayerChunkPosition.Y);
}

FVector2D ATerrainManager::GetTilePosition(int index)
{
	// Get tile location and divide by chunksize to get grid position
	FVector TilePosition = TileArray[index]->GetActorLocation();
	FVector TileGridPosition = TilePosition / ChunkSize;
	return FVector2D(TileGridPosition.X,TileGridPosition.Y);
}


bool ATerrainManager::IsAlreadyThere(FVector2D position)
{
	// Check if each grid position has a tile
	for (int i = 0; i < TileArray.Num(); i++)
	{
		if (GetTilePosition(i) == position)
		{
			return true;
		}
	}
	return false;
}

// Called every frame
void ATerrainManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If multithreading worker has been created
	if (terrainWorker)
	{
		// And has finished it's task
		if (terrainWorker->ThreadComplete)
		{
			// For each tile
			for (auto& tile : TileArray)
			{
				// If the tile does not have a mesh
				if (!tile->MeshCreated)
				{	
					// Create the procedural mesh
					tile->CreateProcMesh();
					tile->MeshCreated = true;

					// Finish spawning the actor with the same parameters
					FTransform SpawnParams(tile->GetActorRotation(), tile->GetActorLocation());
					tile->FinishSpawning(SpawnParams);
					
					// Water can now be created
					UpdateWater = true;
				}
			}
			// If water can be created
			if (UpdateWater)
			{
				// Create the water mesh
				CreateWaterMesh();
				UpdateWater = false;
			}
		}
	}

	// Get players position on grid
	auto PlayerGridPosition = GetPlayerGridPosition();
	PlayerGridPosition.X = round(PlayerGridPosition.X);
	PlayerGridPosition.Y = round(PlayerGridPosition.Y);

	// If multithreading worker has completed
	if (terrainWorker->ThreadComplete)
	{
		// For each tile
		for (int i = 0; i < TileArray.Num(); i++)
		{
			// Get the distance between the tile and the player
			auto PlayerLocation = FVector{ PlayerGridPosition.X * 255, PlayerGridPosition.Y * 255,0 };
			auto TileLocation = TileArray[i]->GetActorLocation();
			auto Distance = (PlayerLocation - TileLocation).Size();

			// Max distance to delete tiles
			auto MaxDistance = (((RenderDistance + 1) * ChunkSize));

			// If the tile is further than the max distance
			if (Distance >= MaxDistance)
			{
				// Remove all owned actors from tile
				TileArray[i]->RemoveTrees();
				TileArray[i]->RemoveRocks();
				TileArray[i]->RemoveGrass();
				TileArray[i]->RemoveAnimals();

				// Destroy the tile and remove from the array
				TileArray[i]->Destroy();
				TileArray.RemoveAt(i);
			}
		}
	}


	// If the player's grid position has changed
	if (PlayerGridPosition != LastPlayerPosition)
	{
		// Create any new tiles
		CreateTileArray();

		// If the multithreading worker has completed
		if (terrainWorker->ThreadComplete)
		{
			// Restart the multithreading worker with the new tiles
			terrainWorker->InputTiles(TileArray);
			terrainWorker->ThreadComplete = false;
		}
	}
	
	// Record the last player position
	LastPlayerPosition = PlayerGridPosition;	
}

void ATerrainManager::CreateTileArray()
{
	// Get the player's grid position
	auto PlayerGridPosition = GetPlayerGridPosition();
	PlayerGridPosition.X = round(PlayerGridPosition.X);
	PlayerGridPosition.Y = round(PlayerGridPosition.Y);

	// Test the area around the player to ensure no duplicate tiles
	for (int x = -RenderDistance; x < RenderDistance; x++)
	{
		for (int y = -RenderDistance; y < RenderDistance; y++)
		{
			// If there is room for a tile
			if (!IsAlreadyThere(FVector2D{ PlayerGridPosition.X + x,PlayerGridPosition.Y + y }))
			{
				// Create spawn parameters
				FVector Location(((PlayerGridPosition.X + x) * ChunkSize) /** Scale*/, ((PlayerGridPosition.Y + y) * ChunkSize) /** Scale*/, 0.0f);
				FRotator Rotation(0.0f, 0.0f, 0.0f);
				FTransform SpawnParams(Rotation, Location);

				// Begin the spawning of the actor. Use deferred spawning to allow mulithreading worker to complete.
				ATerrainTile* tile = GetWorld()->SpawnActorDeferred<ATerrainTile>(TerrainClass, SpawnParams);

				// Initialize the tile with the values set in the editor
				tile->Init(CubeSize, Seed, Scale, ChunkSize, ChunkHeight, Octaves, SurfaceFrequency, CaveFrequency, NoiseScale,
					SurfaceLevel, CaveLevel, OverallNoiseScale, SurfaceNoiseScale, GenerateCaves, CaveNoiseScale, TreeNoiseScale,
					TreeOctaves, TreeFrequency, TreeNoiseValueLimit, TreeClass, TreeMeshList, RockNoiseScale, RockOctaves, RockFrequency,
					RockNoiseValueLimit, RockClass, RockMeshList, WaterLevel, GrassNoiseScale, GrassOctaves, GrassFrequency, GrassNoiseValueLimit,
					GrassClass, GrassMeshList, AnimalNoiseScale, AnimalOctaves, AnimalFrequency, AnimalNoiseValueLimit, AnimalClassList, UseStaticMesh);

				// Save the tile in the array
				TileArray.Push(tile);
			}
		}
	}
}

void ATerrainManager::CreateWaterMesh()
{
	//Get the player's position and empty variables
	FVector PlayerPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	WaterMesh->ClearMeshSection(0);
	WaterVertices.Empty();
	WaterTriangles.Empty();
	WaterNormals.Empty();

	// In the area around the player
	for (int i = -((RenderDistance + 1) * ChunkSize); i < ((RenderDistance + 1) * ChunkSize); i += WaterStepSize)
	{
		for (int j = -((RenderDistance + 1) * ChunkSize); j < ((RenderDistance + 1) * ChunkSize); j += WaterStepSize)
		{
			// Raycast from the chunkheight to the cave level
			FHitResult Hit;
			FVector Start = { float((PlayerPosition.X) + (i * Scale)),float((PlayerPosition.Y) + (j * Scale)), float(ChunkHeight * Scale) };
			FVector End = { float((PlayerPosition.X) + (i * Scale)),float((PlayerPosition.Y) + (j * Scale)), float(CaveLevel * Scale) };
			ECollisionChannel Channel = ECC_Visibility;
			FCollisionQueryParams Params;
			GetWorld()->LineTraceSingleByChannel(Hit, Start, End, Channel, Params);

			// Save hit location
			auto Location = Hit.Location;
			if (Hit.Location != FVector{ 0, 0, 0 })
			{
				// If location is below water level
				if (Hit.Location.Z < (WaterLevel)*Scale)
				{
					// Place water vertex at water level
					Location.Z = (WaterLevel)*Scale;
					WaterVertices.Push(Location);
				}
				
			}
		}
	}

	// Coordinates for Delaunay triangulation
	std::vector<double> coords;

	// Bools to check for errors
	bool inARowX = true;
	bool inARowY = true;
	float lastXVertex = 0;
	float lastYVertex = 0;

	// Check if vertices are in a row (cannot triangulate)
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
		// Push coordinates into list in required format
		coords.push_back(vertex.X);
		coords.push_back(vertex.Y);

		lastXVertex = vertex.X;
		lastYVertex = vertex.Y;
	}

	// If there are at least 3 vertices
	if (coords.size() > 6)
	{
		// And they are not in a row
		if (!inARowX && !inARowY)
		{
			// Use external library to triangulate
			delaunator::Delaunator d(coords);

			// Push resultant triangles into triangles list
			for (auto& triangle : d.triangles)
			{
				WaterTriangles.Push(triangle);
			}
		}
	}

	// Create mesh section with Vertices and Triangles
	WaterMesh->CreateMeshSection(0, WaterVertices, WaterTriangles, WaterNormals, WaterUV0, WaterVertexColour, WaterTangents, false);

	// Set material set in blueprint
	WaterMesh->SetMaterial(0, WaterMeshMaterial);
}