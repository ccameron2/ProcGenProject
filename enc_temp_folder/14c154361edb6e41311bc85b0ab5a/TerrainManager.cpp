// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainManager.h"
#include "Kismet\GameplayStatics.h"
#include <math.h>
#include "KismetProceduralMeshLibrary.h"
#include "delaunator.hpp"

#include "MeshDescription.h"
#include "MeshDescriptionBuilder.h"
#include "StaticMeshAttributes.h"

// Sets default values
ATerrainManager::ATerrainManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WaterMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Water Mesh"));
	WaterMesh->SetupAttachment(RootComponent);
	WaterMesh->SetMaterial(0, WaterMeshMaterial);
}

// Called when the game starts or when spawned
void ATerrainManager::BeginPlay()
{
	Super::BeginPlay();
	
	auto PlayerGridPosition = GetPlayerGridPosition();
	PlayerGridPosition.X = round(PlayerGridPosition.X);
	PlayerGridPosition.Y = round(PlayerGridPosition.Y);

	CreateTileArray();

	terrainWorker = std::make_unique<FTerrainWorker>(TileArray);
}

FVector2D ATerrainManager::GetPlayerGridPosition()
{
	FVector PlayerPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	FVector PlayerChunkPosition = PlayerPosition / (ChunkSize * Scale);
	return FVector2D(PlayerChunkPosition.X, PlayerChunkPosition.Y);
}

FVector2D ATerrainManager::GetTilePosition(int index)
{
	FVector TilePosition = TileArray[index]->GetActorLocation();
	FVector TileGridPosition = TilePosition / ChunkSize;
	return FVector2D(TileGridPosition.X,TileGridPosition.Y);
}


bool ATerrainManager::IsAlreadyThere(FVector2D position)
{
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

	if (terrainWorker)
	{
		if (terrainWorker->ThreadComplete)
		{
			for (auto& tile : TileArray)
			{
				if (!tile->MeshCreated)
				{					
					tile->CreateProcMesh();
					tile->MeshCreated = true;
					FTransform SpawnParams(tile->GetActorRotation(), tile->GetActorLocation());
					tile->FinishSpawning(SpawnParams);
					UpdateWater = true;
				}
			}
			if (UpdateWater)
			{
				CreateWaterMesh();
				UpdateWater = false;
			}
		}
	}

	auto PlayerGridPosition = GetPlayerGridPosition();
	PlayerGridPosition.X = round(PlayerGridPosition.X);
	PlayerGridPosition.Y = round(PlayerGridPosition.Y);

	if (terrainWorker->ThreadComplete)
	{
		for (int i = 0; i < TileArray.Num(); i++)
		{
			auto PlayerLocation = FVector{ PlayerGridPosition.X * 255, PlayerGridPosition.Y * 255,0 };
			//auto PlayerLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
			auto TileLocation = TileArray[i]->GetActorLocation();
			//TileLocation *= Scale;
			auto Distance = (PlayerLocation - TileLocation).Size();
			auto MaxDistance = (((RenderDistance + 1) * ChunkSize));

			if (Distance >= MaxDistance + 1)
			{
				TileArray[i]->RemoveTrees();
				TileArray[i]->RemoveRocks();
				TileArray[i]->RemoveGrass();
				TileArray[i]->RemoveAnimals();
				TileArray[i]->Destroy();
				TileArray.RemoveAt(i);
			}
		}
	}

	if (PlayerGridPosition != LastPlayerPosition)
	{
		
		CreateTileArray();
		if (terrainWorker->ThreadComplete)
		{
			terrainWorker->InputTiles(TileArray);
			terrainWorker->ThreadComplete = false;
		}
	}
	
	LastPlayerPosition = PlayerGridPosition;	
}

void ATerrainManager::CreateTileArray()
{
	auto PlayerGridPosition = GetPlayerGridPosition();
	PlayerGridPosition.X = round(PlayerGridPosition.X);
	PlayerGridPosition.Y = round(PlayerGridPosition.Y);

	for (int x = -RenderDistance; x < RenderDistance; x++)
	{
		for (int y = -RenderDistance; y < RenderDistance; y++)
		{
			if (!IsAlreadyThere(FVector2D{ PlayerGridPosition.X + x,PlayerGridPosition.Y + y }))
			{
				FVector Location(((PlayerGridPosition.X + x) * ChunkSize) /** Scale*/, ((PlayerGridPosition.Y + y) * ChunkSize) /** Scale*/, 0.0f);
				FRotator Rotation(0.0f, 0.0f, 0.0f);
				FTransform SpawnParams(Rotation, Location);

				ATerrainTile* tile = GetWorld()->SpawnActorDeferred<ATerrainTile>(TerrainClass, SpawnParams);

				tile->Init(CubeSize, Seed, Scale, ChunkSize, ChunkHeight, Octaves, SurfaceFrequency, CaveFrequency, NoiseScale,
					SurfaceLevel, CaveLevel, OverallNoiseScale, SurfaceNoiseScale, GenerateCaves, CaveNoiseScale, TreeNoiseScale,
					TreeOctaves, TreeFrequency, TreeNoiseValueLimit, RockNoiseScale, RockOctaves, RockFrequency, RockNoiseValueLimit,
					WaterLevel, GrassNoiseScale, GrassOctaves, GrassFrequency, GrassNoiseValueLimit, AnimalNoiseScale, AnimalOctaves, 
					AnimalFrequency, AnimalNoiseValueLimit,AnimalClassList,UseStaticMesh);

				TileArray.Push(tile);
			}
		}
	}
}

void ATerrainManager::CreateWaterMesh()
{
	FVector PlayerPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	WaterMesh->ClearMeshSection(0);
	WaterVertices.Empty();
	WaterTriangles.Empty();
	WaterNormals.Empty();
	for (int i = -((RenderDistance + 1) * ChunkSize); i < ((RenderDistance + 1) * ChunkSize); i+=WaterStepSize)
	{
		for (int j = -((RenderDistance +1) * ChunkSize); j < ((RenderDistance + 1) * ChunkSize); j+=WaterStepSize)
		{
			//5,4,0.4,0.25
			float waterNoise = ATerrainTile::FractalBrownianMotion(FVector{ PlayerPosition.X + float(i), PlayerPosition.Y + float(j), 0 } / WaterNoiseScale, WaterOctaves, WaterFrequency);
			if (waterNoise > WaterNoiseValueLimit)
			{
				FHitResult Hit;
				FVector Start = { float((PlayerPosition.X) + (i * Scale)),float((PlayerPosition.Y) + (j * Scale)), float(ChunkHeight * Scale) };
				FVector End = { float((PlayerPosition.X) + (i * Scale)),float((PlayerPosition.Y) + (j * Scale)), float(CaveLevel * Scale) };
				ECollisionChannel Channel = ECC_Visibility;
				FCollisionQueryParams Params;
				GetWorld()->LineTraceSingleByChannel(Hit, Start, End, Channel, Params);
				FVector Offset = FVector{ 0,0,float(Scale * 1) };
				FVector Location = Hit.Location + Offset;
				if (Hit.Location != FVector{ 0, 0, 0 })
				{
					if (Hit.Location != Offset)
					{
						if (Hit.Location.Z < (WaterLevel)*Scale)
						{
							if (Hit.Location.Z < (WaterLevel) * Scale)
							{
								Location.Z = (WaterLevel) * Scale;
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

	WaterMesh->CreateMeshSection(0, WaterVertices, WaterTriangles, WaterNormals, WaterUV0, WaterVertexColour, WaterTangents, false);
	WaterMesh->SetMaterial(0, WaterMeshMaterial);
}