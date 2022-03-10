// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainManager.h"
#include "Kismet\GameplayStatics.h"
#include <math.h>

// Sets default values
ATerrainManager::ATerrainManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATerrainManager::BeginPlay()
{
	Super::BeginPlay();
	
	auto PlayerGridPosition = GetPlayerGridPosition();
	PlayerGridPosition.X = round(PlayerGridPosition.X);
	PlayerGridPosition.Y = round(PlayerGridPosition.Y);
	auto CurrentTileIndex = PlayerGridPosition.X + (PlayerGridPosition.Y * TileX);

	for (int x = -TileX; x < TileX; x++)
	{
		for (int y = -TileY; y < TileY; y++)
		{
			FVector Location((PlayerGridPosition.X + x) * ChunkSize, (PlayerGridPosition.Y + y) * ChunkSize, 0.0f);
			FRotator Rotation(0.0f, 0.0f, 0.0f);
			FTransform SpawnParams(Rotation, Location);

			ATerrainTile* tile = GetWorld()->SpawnActorDeferred<ATerrainTile>(TerrainClass, SpawnParams);

			tile->Init(Seed, Scale, ChunkSize, ChunkHeight, Octaves,SurfaceFrequency,CaveFrequency, NoiseScale,
						SurfaceLevel, CaveLevel, SurfaceNoiseScale, CaveNoiseScale, TreeNoiseScale, 
							TreeOctaves, TreeFrequency, TreeNoiseValueLimit, WaterLevel);

			TileArray.Push(tile);
		}
	}

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
	auto PlayerGridPosition = GetPlayerGridPosition();
	PlayerGridPosition.X = round(PlayerGridPosition.X);
	PlayerGridPosition.Y = round(PlayerGridPosition.Y);

	if (PlayerGridPosition != LastPlayerPosition)
	{
		for (int x = -TileX; x < TileX; x++)
		{
			for (int y = -TileY; y < TileY; y++)
			{
				FVector Location((PlayerGridPosition.X + x) * ChunkSize, (PlayerGridPosition.Y + y) * ChunkSize, 0.0f);
				if (!IsAlreadyThere(FVector2D{ PlayerGridPosition.X + x,PlayerGridPosition.Y + y }))
				{	
					FRotator Rotation(0.0f, 0.0f, 0.0f);
					UWorld* World = GetWorld();
					FTransform SpawnParams(Rotation, Location);
					ATerrainTile* tile = World->SpawnActorDeferred<ATerrainTile>(TerrainClass, SpawnParams);
					tile->Init(Seed, Scale, ChunkSize, ChunkHeight, Octaves, SurfaceFrequency, CaveFrequency, NoiseScale,
								SurfaceLevel, CaveLevel, SurfaceNoiseScale, CaveNoiseScale, TreeNoiseScale,
									TreeOctaves, TreeFrequency, TreeNoiseValueLimit, WaterLevel);
					TileArray.Push(tile);
					
				}
			}
		}
		if (terrainWorker->ThreadComplete)
		{
			terrainWorker->InputTiles(TileArray);
			terrainWorker->ThreadComplete = false;
		}
	}

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
				}
			}
		}
	}
	
	LastPlayerPosition = PlayerGridPosition;	
}