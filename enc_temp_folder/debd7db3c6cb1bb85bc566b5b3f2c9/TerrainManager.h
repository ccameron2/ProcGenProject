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

	UPROPERTY(EditAnywhere, Category = "Terrain Generation|Chunks")
		TSubclassOf<ATerrainTile> TerrainClass;

	UPROPERTY(VisibleAnywhere, Category = "Terrain Generation|Chunks")
		TArray<ATerrainTile*> TileArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Chunks")
		int CubeSize = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Chunks")
		int RenderDistance = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Chunks")
		int ChunkSize = 256;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Chunks")
		int ChunkHeight = 1800;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Chunks")
		int Scale = 200;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Chunks")
		bool UseStaticMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		float Seed = 875694;			

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		int Octaves = 10;				

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		float SurfaceFrequency = 0.35;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		float CaveFrequency = 1;		

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		int NoiseScale = 50;			

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		int SurfaceLevel = 900;			

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		int CaveLevel = 800;			

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		int OverallNoiseScale = 23;		

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		int SurfaceNoiseScale = 18;		

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		bool GenerateCaves = true;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		int CaveNoiseScale = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Trees")
		int TreeNoiseScale = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Trees")
		int TreeOctaves = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Trees")
		float TreeFrequency = 0.4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Trees")
		float TreeNoiseValueLimit = 0.3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Water")
		float WaterLevel = 900;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Water")
		float WaterNoiseScale = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Water")
		int WaterOctaves = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Water")
		float WaterFrequency = 0.4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Water")
		float WaterNoiseValueLimit = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Water")
		float WaterStepSize = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Water")
		UMaterialInterface* WaterMeshMaterial;

	UPROPERTY(BlueprintReadWrite, Category = "Terrain Generation|Water")
		UProceduralMeshComponent* WaterMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Grass")
		float GrassNoiseScale = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Grass")
		int GrassOctaves = 4;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Grass")
		float GrassFrequency = 0.25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Grass")
		float GrassNoiseValueLimit = 0.25;

	UPROPERTY(EditAnywhere, Category = "Terrain Generation|Rocks")
		float RockNoiseScale = 5;

	UPROPERTY(EditAnywhere, Category = "Terrain Generation|Rocks")
		int RockOctaves = 4;

	UPROPERTY(EditAnywhere, Category = "Terrain Generation|Rocks")
		float RockFrequency = 0.32;

	UPROPERTY(EditAnywhere, Category = "Terrain Generation|Rocks")
		float RockNoiseValueLimit = 0.4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Animals")
		float AnimalNoiseScale = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Animals")
		int AnimalOctaves = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Animals")
		float AnimalFrequency = 0.42;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Animals")
		float AnimalNoiseValueLimit = 0.4;

	UPROPERTY(EditAnywhere, Category = "Terrain Generation|Animals")
		TArray<TSubclassOf<AAnimal>> AnimalClassList;

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
	FVector2D LastPlayerPosition = { 0,0 };

	std::unique_ptr<FTerrainWorker> terrainWorker;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
