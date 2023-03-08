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

	// Class to use for tiles
	UPROPERTY(EditAnywhere, Category = "Terrain Generation|Chunks")
		TSubclassOf<ATerrainTile> TerrainClass;

	// Array to keep track of tiles
	UPROPERTY(VisibleAnywhere, Category = "Terrain Generation|Chunks")
		TArray<ATerrainTile*> TileArray;

	// Marching cubes size of each cube (Resolution of terrain)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Chunks")
		int CubeSize = 16;

	// Number of tiles to place in each direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Chunks")
		int RenderDistance = 5;

	// Size (x,y) of each tile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Chunks")
		int ChunkSize = 256;

	// Height of each tile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Chunks")
		int ChunkHeight = 1800;

	// Scale of the mesh generated
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Chunks")
		int Scale = 50;

	// Create a static mesh if true or a procedural mesh if false (Static mesh collision does not work when packaged into exe)
	// This can be faster than procedural mesh as it cannot be updated at runtime
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Chunks")
		bool UseStaticMesh = false;

	// Seed to offset perlin noise by
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		float Seed = 875694;			

	// Octaves of noise to use in terrain generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		int Octaves = 10;				
	
	// Frequency of noise to use in surface terrain generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		float SurfaceFrequency = 0.35;	

	// Frequency of noise to use in cave terrain generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		float CaveFrequency = 1;		

	// Scale of noise to use in terrain generation overall
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		int NoiseScale = 50;			

	// Z location of the surface for cave blending
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		int SurfaceLevel = 900;			

	// Z location of the cave layer for surface blending
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		int CaveLevel = 800;			

	// Overall scale of terrain generation noise
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		int OverallNoiseScale = 23;		

	// Additional scale for the surface noise
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		int SurfaceNoiseScale = 18;		

	// Generate caves when true
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		bool GenerateCaves = true;	

	// Additional scale for the cave noise 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Terrain")
		int CaveNoiseScale = 6;

	// Scale of noise used in tree generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Trees")
		int TreeNoiseScale = 4;

	// Octaves of noise used in tree generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Trees")
		int TreeOctaves = 4;

	// Frequency of noise used in tree generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Trees")
		float TreeFrequency = 0.4;

	// Limit at which to place a tree when reading noise value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Trees")
		float TreeNoiseValueLimit = 0.3;

	// Class to use when spawning a tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Trees")
		TSubclassOf<class ATree> TreeClass;

	// List of available meshes when placing a tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Trees")
		TArray<UStaticMesh*> TreeMeshList;

	// Level water should be placed 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Water")
		float WaterLevel = 900;

	// Step between each raycast check for water placement when scanning the terrain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Water")
		float WaterStepSize = 8;

	// Material to use for water
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Water")
		UMaterialInterface* WaterMeshMaterial;

	// Mesh for water generation
	UPROPERTY(BlueprintReadWrite, Category = "Terrain Generation|Water")
		UProceduralMeshComponent* WaterMesh;

	// Scale of noise used in grass generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Grass")
		float GrassNoiseScale = 5;

	// Octaves of noise used in grass generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Grass")
		int GrassOctaves = 4;
	
	// Frequency of noise used in grass generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Grass")
		float GrassFrequency = 0.25;

	// Limit at which to place grass when reading noise value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Grass")
		float GrassNoiseValueLimit = 0.25;

	// Class to use when spawning grass
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Grass")
		TSubclassOf<class AGrass> GrassClass;

	// List of available meshes when placing grass
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Grass")
		TArray<UStaticMesh*> GrassMeshList;

	// Scale of noise used in rock generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Terrain Generation|Rocks")
		float RockNoiseScale = 5;

	// Octaves of noise used in rock generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Rocks")
		int RockOctaves = 4;

	// Frequency of noise used in rock generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Rocks")
		float RockFrequency = 0.32;

	// Limit at which to place rocks when reading noise value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Rocks")
		float RockNoiseValueLimit = 0.4;

	// Class to use when spawning rocks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Rocks")
		TSubclassOf<class ARock> RockClass;

	// List of available meshes when placing a rock
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Rocks")
		TArray<UStaticMesh*> RockMeshList;

	// Scale of noise used in animal generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Animals")
		float AnimalNoiseScale = 5;

	// Octaves of noise used in animal generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Animals")
		int AnimalOctaves = 4;

	// Frequency of noise used in animal generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Animals")
		float AnimalFrequency = 0.42;

	// Limit at which to place animals when reading noise value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Animals")
		float AnimalNoiseValueLimit = 0.4;

	// Classes to use when spawning animals
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation|Animals")
		TArray<TSubclassOf<AAnimal>> AnimalClassList;

	// Create water
	void CreateWaterMesh();
	void CalculateNormals();
	bool UpdateWater = false;

	// Water variables for mesh generation
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

	// Begin spawning new tiles in required locations
	void CreateTileArray();

	// Returns player grid position
	FVector2D GetPlayerGridPosition();

	// Returns tile grid position
	FVector2D GetTilePosition(int index);

	// Returns false if theres is no tile at position
	bool IsAlreadyThere(FVector2D position);

	// Last player position recorded
	FVector2D LastPlayerPosition = { 0,0 };

	// Multithreading worker for terrain generation
	std::unique_ptr<FTerrainWorker> terrainWorker;

	void LoadModels();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
