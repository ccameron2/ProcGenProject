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

	UPROPERTY(EditAnywhere, Category = "Chunks")
		int CubeSize = 8;

	UPROPERTY(EditAnywhere,Category = "Chunks")
		int RenderDistance = 5;

	UPROPERTY(EditAnywhere, Category = "Chunks")
		int ChunkSize = 256;

	UPROPERTY(EditAnywhere, Category = "Chunks")
		int ChunkHeight = 1800;

	UPROPERTY(EditAnywhere, Category = "Chunks")
		int Scale = 100;

	UPROPERTY(EditAnywhere, Category = "Terrain")
		float Seed = 875694;			

	UPROPERTY(EditAnywhere, Category = "Terrain")
		int Octaves = 10;				

	UPROPERTY(EditAnywhere, Category = "Terrain")
		float SurfaceFrequency = 0.35;	

	UPROPERTY(EditAnywhere, Category = "Terrain")
		float CaveFrequency = 1;		

	UPROPERTY(EditAnywhere, Category = "Terrain")
		int NoiseScale = 50;			

	UPROPERTY(EditAnywhere, Category = "Terrain")
		int SurfaceLevel = 900;			

	UPROPERTY(EditAnywhere, Category = "Terrain")
		int CaveLevel = 700;			

	UPROPERTY(EditAnywhere, Category = "Terrain")
		int OverallNoiseScale = 23;		

	UPROPERTY(EditAnywhere, Category = "Terrain")
		int SurfaceNoiseScale = 18;		

	UPROPERTY(EditAnywhere, Category = "Terrain")
		bool GenerateCaves = true;	

	UPROPERTY(EditAnywhere, Category = "Terrain")
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

	UPROPERTY(EditAnywhere, Category = "Water")
		float WaterNoiseScale = 5;

	UPROPERTY(EditAnywhere, Category = "Water")
		int WaterOctaves = 4;

	UPROPERTY(EditAnywhere, Category = "Water")
		float WaterFrequency = 0.4;

	UPROPERTY(EditAnywhere, Category = "Water")
		float WaterNoiseValueLimit = 0.25;

	UPROPERTY(EditAnywhere, Category = "Water")
		float WaterStepSize = 8;

	UPROPERTY(EditAnywhere, Category = "Water")
		UMaterialInterface* WaterMeshMaterial;

	UPROPERTY(VisibleAnywhere, Category = "Water")
		UProceduralMeshComponent* WaterMesh;

	UPROPERTY(VisibleAnywhere, Category = "Water")
		UStaticMeshComponent* WaterStaticMesh;

	UPROPERTY(EditAnywhere, Category = "Grass")
		float GrassNoiseScale = 5;

	UPROPERTY(EditAnywhere, Category = "Grass")
		int GrassOctaves = 4;
	
	UPROPERTY(EditAnywhere, Category = "Grass")
		float GrassFrequency = 0.2;

	UPROPERTY(EditAnywhere, Category = "Grass")
		float GrassNoiseValueLimit = 0.2;


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

	UPROPERTY(EditAnywhere, Category = "Rocks")
		float RockNoiseScale = 5;

	UPROPERTY(EditAnywhere, Category = "Rocks")
		int RockOctaves = 4;

	UPROPERTY(EditAnywhere, Category = "Rocks")
		float RockFrequency = 0.4;

	UPROPERTY(EditAnywhere, Category = "Rocks")
		float RockNoiseValueLimit = 0.25;

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
