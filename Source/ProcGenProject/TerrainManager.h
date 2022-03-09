// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TerrainTile.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainManager.generated.h"

UCLASS()
class PROCGENPROJECT_API ATerrainManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATerrainManager();

	UPROPERTY(VisibleAnywhere, Category = "Chunks")
	TArray<ATerrainTile*> TileArray;

	UPROPERTY(EditAnywhere,Category = "Chunks")
		int TileX = 10;

	UPROPERTY(EditAnywhere, Category = "Chunks")
		int TileY = 10;

	UPROPERTY(EditAnywhere, Category = "Chunks")
		int ChunkSize = 256;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		bool UseCustomMultithreading = false;

	UPROPERTY(EditAnywhere, Category = "Chunks")
		int Scale = 100;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		float Seed = 875694;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		int Octaves = 10;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		float SurfaceFrequency = 0.35;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		float CaveFrequency = 1;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		int NoiseScale = 50;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		int SurfaceLevel = 1000;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		int CaveLevel = 600;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		int SurfaceNoiseScale = 18;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		int CaveNoiseScale = 6;

	UPROPERTY(EditAnywhere, Category = "Trees")
		int TreeNoiseScale = 5;

	UPROPERTY(EditAnywhere, Category = "Trees")
		int TreeOctaves = 4;

	UPROPERTY(EditAnywhere, Category = "Trees")
		float TreeFrequency = 0.4;

	UPROPERTY(EditAnywhere, Category = "Trees")
		float TreeNoiseValueLimit = 0.25;

	UPROPERTY(EditAnywhere, Category = "Water")
		float WaterLevel = 800;

	FVector2D GetPlayerGridPosition();
	FVector2D GetTilePosition(int index);
	bool IsAlreadyThere(FVector2D position);
	bool hasRun = false;
	FVector2D LastPlayerPosition = { 0,0 };

	UPROPERTY(EditAnywhere, Category = "Procgen")
		TSubclassOf<ATerrainTile> TerrainClass;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	static double PerlinWrapper(FVector3<double> perlinInput);

	static float FractalBrownianMotion(FVector fractalInput, float fractalOctaves, float frequency);

	static float seed;
	
	static int scale;
	static int octaves;
	static float surfaceFrequency;
	static float caveFrequency;
	static int noiseScale;
	static int surfaceLevel;
	static int caveLevel;
	static int surfaceNoiseScale;
	static int caveNoiseScale;

};
