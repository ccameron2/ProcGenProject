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
				}
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

			if (Distance > MaxDistance)
			{
				TileArray[i]->RemoveTrees();
				TileArray[i]->RemoveRocks();
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
						SurfaceLevel, CaveLevel, OverallNoiseScale, SurfaceNoiseScale, GenerateCaves,CaveNoiseScale, TreeNoiseScale,
						TreeOctaves, TreeFrequency, TreeNoiseValueLimit, WaterLevel, WaterNoiseScale, WaterOctaves, WaterFrequency, 
						WaterNoiseValueLimit, RockNoiseScale,RockOctaves ,RockFrequency,RockNoiseValueLimit);

				TileArray.Push(tile);
			}
		}
	}
}