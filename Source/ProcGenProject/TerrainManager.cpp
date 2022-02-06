// Fill out your copyright notice in the Description page of Project Settings.

#include <math.h>
#include "TerrainManager.h"

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
	for (float i = 0; i < TileX; i++)
	{
		for (float j = 0; j < TileY; j++)
		{
			FVector Location(i * ChunkSize, j * ChunkSize, 0.0f);
			FRotator Rotation(0.0f, 0.0f, 0.0f);
			FActorSpawnParameters SpawnParams;
			ATerrainTile* tile = GetWorld()->SpawnActor<ATerrainTile>(Location, Rotation, SpawnParams);
			TileArray.Push(tile);
		}
	}
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
	FVector TileGridPosition = TilePosition / ChunkSize * Scale;
	return FVector2D(TileGridPosition.X,TileGridPosition.Y);
}

// Called every frame
void ATerrainManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	auto PlayerGridPosition = GetPlayerGridPosition();
	PlayerGridPosition.X = round(PlayerGridPosition.X);
	PlayerGridPosition.Y = round(PlayerGridPosition.Y);
	auto CurrentTileIndex = PlayerGridPosition.X + (PlayerGridPosition.Y * TileX);
	if (CurrentTileIndex > TileArray.Num() - TileX)
	{
		for (int i = 0; i < TileX; i++)
		{
			FVector Location((GetPlayerGridPosition().X + i) * ChunkSize, (GetPlayerGridPosition().Y) * ChunkSize, 0.0f);
			FRotator Rotation(0.0f, 0.0f, 0.0f);
			FActorSpawnParameters SpawnParams;
			ATerrainTile* tile = GetWorld()->SpawnActor<ATerrainTile>(Location, Rotation, SpawnParams);
			TileArray.Push(tile);
		}	
		return;
	}
	//auto TilePosition = GetTilePosition(CurrentTileIndex);
	//if (PlayerGridPosition != TilePosition)
	//{
	//	//if (hasRun) { return; }

	//	if (TileArray[CurrentTileIndex])
	//	{

	//	}
	//	FVector Location((GetPlayerGridPosition().X) * ChunkSize, GetPlayerGridPosition().Y * ChunkSize, 0.0f);
	//	FRotator Rotation(0.0f, 0.0f, 0.0f);
	//	FActorSpawnParameters SpawnParams;
	//	ATerrainTile* tile = GetWorld()->SpawnActor<ATerrainTile>(Location, Rotation, SpawnParams);
	//	TileArray.Push(tile);
	//	//hasRun = true;
	//}
}

