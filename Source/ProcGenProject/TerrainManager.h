// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TerrainTile.h"
#include "TerrainWorker.h"
#include "ProceduralMeshComponent.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		int CubeSize = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		int RenderDistance = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		int ChunkSize = 256;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		int ChunkHeight = 1800;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Chunks")
		int Scale = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		float Seed = 875694;			

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		int Octaves = 10;				

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		float SurfaceFrequency = 0.35;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		float CaveFrequency = 1;		

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		int NoiseScale = 50;			

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		int SurfaceLevel = 900;			

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		int CaveLevel = 800;			

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		int OverallNoiseScale = 23;		

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		int SurfaceNoiseScale = 18;		

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		bool GenerateCaves = true;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		int CaveNoiseScale = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trees")
		int TreeNoiseScale = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trees")
		int TreeOctaves = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trees")
		float TreeFrequency = 0.4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trees")
		float TreeNoiseValueLimit = 0.3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		float WaterLevel = 900;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		float WaterNoiseScale = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		int WaterOctaves = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		float WaterFrequency = 0.4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		float WaterNoiseValueLimit = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		float WaterStepSize = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		UMaterialInterface* WaterMeshMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Water")
		UProceduralMeshComponent* WaterMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Water")
		UStaticMeshComponent* WaterStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grass")
		float GrassNoiseScale = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grass")
		int GrassOctaves = 4;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grass")
		float GrassFrequency = 0.25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grass")
		float GrassNoiseValueLimit = 0.25;

	UPROPERTY(EditAnywhere, Category = "Rocks")
		float RockNoiseScale = 5;

	UPROPERTY(EditAnywhere, Category = "Rocks")
		int RockOctaves = 4;

	UPROPERTY(EditAnywhere, Category = "Rocks")
		float RockFrequency = 0.32;

	UPROPERTY(EditAnywhere, Category = "Rocks")
		float RockNoiseValueLimit = 0.4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal")
		float AnimalNoiseScale = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal")
		int AnimalOctaves = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal")
		float AnimalFrequency = 0.42;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal")
		float AnimalNoiseValueLimit = 0.4;


	void CreateWaterMesh();
	bool UpdateWater = false;

	UPROPERTY()
		TArray< FVector > WaterVertices;

	UPROPERTY()
		TArray< int32 > WaterTriangles;

	UPROPERTY()
		TArray< FVector > WaterNormals;

	UPROPERTY()
		TArray< FVector2D > WaterUV0;

	UPROPERTY()
		TArray< FColor > WaterVertexColour;

	UPROPERTY()
		TArray <FProcMeshTangent> WaterTangents;

	
	void CreateTileArray();
	FVector2D GetPlayerGridPosition();
	FVector2D GetTilePosition(int index);
	bool IsAlreadyThere(FVector2D position);
	bool hasRun = false;
	FVector2D LastPlayerPosition = { 0,0 };

	UPROPERTY(EditAnywhere, Category = "Procgen")
		TSubclassOf<ATerrainTile> TerrainClass;

	std::unique_ptr<FTerrainWorker> terrainWorker;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
