// Fill out your copyright notice in the Description page of Project Settings.


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

// Called every frame
void ATerrainManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

