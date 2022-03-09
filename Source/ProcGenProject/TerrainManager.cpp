// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainManager.h"
#include "CustomWorker.h"
#include "Kismet\GameplayStatics.h"
#include <math.h>

float ATerrainManager::seed = 0;
int	  ATerrainManager::scale = 0;
int   ATerrainManager::octaves = 0;
float ATerrainManager::surfaceFrequency = 0;
float ATerrainManager::caveFrequency = 0;
int   ATerrainManager::noiseScale = 0;
int   ATerrainManager::surfaceLevel = 0;
int   ATerrainManager::caveLevel = 0;
int   ATerrainManager::surfaceNoiseScale = 0;
int   ATerrainManager::caveNoiseScale = 0;

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
	
	seed = Seed;
	scale = Scale;
	octaves = Octaves;
	surfaceFrequency = SurfaceFrequency;
	caveFrequency = CaveFrequency;
	noiseScale = NoiseScale;
	surfaceLevel = SurfaceLevel;
	caveLevel = CaveLevel;
	surfaceNoiseScale = SurfaceNoiseScale;
	caveNoiseScale = CaveNoiseScale;

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
			//FActorSpawnParameters SpawnParams;
			//ATerrainTile* tile = GetWorld()->SpawnActor<ATerrainTile>(Location, Rotation, SpawnParams);
			UWorld* World = GetWorld();
			FTransform SpawnParams(Rotation, Location);
			ATerrainTile* tile = World->SpawnActorDeferred<ATerrainTile>(TerrainClass, SpawnParams);
			tile->Init(UseCustomMultithreading, Seed, Scale, Octaves,SurfaceFrequency,CaveFrequency,
						NoiseScale, SurfaceLevel, CaveLevel, SurfaceNoiseScale, CaveNoiseScale,
							TreeNoiseScale, TreeOctaves, TreeFrequency, TreeNoiseValueLimit, WaterLevel);
			UGameplayStatics::FinishSpawningActor(tile, SpawnParams);

			//tile->FinishSpawning(SpawnParams);
			tile->CreateMesh();
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
					tile->Init(UseCustomMultithreading, Seed, Scale, Octaves, SurfaceFrequency, CaveFrequency,
								NoiseScale, SurfaceLevel, CaveLevel, SurfaceNoiseScale, CaveNoiseScale,
									TreeNoiseScale, TreeOctaves, TreeFrequency, TreeNoiseValueLimit, WaterLevel);

					UGameplayStatics::FinishSpawningActor(tile, SpawnParams);
					//tile->FinishSpawning(SpawnParams);
					tile->CreateMesh();
					TileArray.Push(tile);
				}

			}
		}

	}

	LastPlayerPosition = PlayerGridPosition;

	/*if (CurrentTileIndex > TileArray.Num() - TileX)
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
	}*/
}

double ATerrainManager::PerlinWrapper(FVector3<double> perlinInput)
{
	//Scale noise input
	FVector3d noiseInput = (perlinInput + FVector{ seed,seed,0 }) / noiseScale;

	float density = (-noiseInput.Z / 23) + 1;

	//Add 3D noise partially
	//density += FractalBrownianMotion(FVector(noiseInput) / 5, 6,0.5);

	//Add 2D noise
	density += FractalBrownianMotion(FVector(noiseInput.X / surfaceNoiseScale, noiseInput.Y / surfaceNoiseScale, 0), octaves, surfaceFrequency); //14


	//density = FMath::PerlinNoise2D(FVector2D(noiseInput.X, noiseInput.Y));

	float density2 = FractalBrownianMotion(FVector(noiseInput / caveNoiseScale), octaves, caveFrequency);

	if (perlinInput.Z < 1)//Cave floors
	{
		return 1;
	}

	if (FractalBrownianMotion(FVector{ float(noiseInput.X),float(noiseInput.Y),0 } / 8, 4, 0.8) > 0.4)
	{
		if (perlinInput.Z >= surfaceLevel * scale)
		{
			return density - 0.2;
		}
		/*if (perlinInput.Z < CaveLevel)
		{
			return density2;
		}
		else
		{
			return FMath::Lerp(density2, density, (perlinInput.Z - CaveLevel) / (SurfaceLevel - CaveLevel));
		}*/
		//else
		//{
		//	return -1;
		//}
	}

	if (perlinInput.Z >= surfaceLevel)//640
	{
		return density;
	}
	else if (perlinInput.Z < caveLevel)//384
	{
		return density2;
	}
	else
	{
		return FMath::Lerp(density2 + 0.2f, density, (perlinInput.Z - caveLevel) / (surfaceLevel - caveLevel)); //0.1
	}
}

float ATerrainManager::FractalBrownianMotion(FVector fractalInput, float fractalOctaves, float frequency)
{
	//The book of shaders
	float result = 0;
	float amplitude = 0.5;
	float lacunarity = 2.0;
	float gain = 0.5;

	for (int i = 0; i < fractalOctaves; i++)
	{
		result += amplitude * FMath::PerlinNoise3D(frequency * fractalInput);
		frequency *= lacunarity;
		amplitude *= gain;
	}

	return result;
}