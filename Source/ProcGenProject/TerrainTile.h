// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "Generators/MarchingCubes.h"
#include "VectorTypes.h"
#include "GameFramework/Actor.h"
#include "Tree.h"
#include "Rock.h"
#include "Grass.h"
#include "Animal.h"
#include "TerrainTile.generated.h"

UCLASS()
class PROCGENPROJECT_API ATerrainTile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATerrainTile();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OnConstruction(const FTransform& Transform) override;

	static double PerlinWrapper(FVector3<double> perlinInput);

	static float FractalBrownianMotion(FVector fractalInput, float octaves, float frequency);
	
	void CalculateNormals();

	void CreateTrees();

	void RemoveTrees();

	void CreateGrass();

	void RemoveGrass();

	void CreateAnimals();

	void RemoveAnimals();

	void Init(int cubeSize, float seed, int scale, int chunkSize, int chunkHeight, int octaves, float surfaceFrequency, float caveFrequency,
			  float noiseScale, int surfaceLevel, int caveLevel, int overallNoiseScale, int surfaceNoiseScale, bool generateCaves, float caveNoiseScale,
			  float treeNoiseScale, int treeOctaves, float treeFrequency, float treeNoiseValueLimit, TSubclassOf<class ATree> treeClass, TArray<UStaticMesh*> treeMeshList,
			  float rockNoiseScale, int rockOctaves, float rockFrequency, float rockNoiseValueLimit, TSubclassOf<class ARock> rockClass, TArray<UStaticMesh*> rockMeshList, 
			  int waterLevel, float grassNoiseScale, int grassOctaves, float grassFrequency, float grassNoiseValueLimit, TSubclassOf<class AGrass> grassClass, 
			  TArray<UStaticMesh*> grassMeshList, float animalNoiseScale, int animalOctaves, float animalFrequency, float animalNoiseValueLimit, TArray<TSubclassOf<AAnimal>> animalClassList,
			  bool useStaticMesh);

	void GenerateTerrain();

	void CreateRocks();
	void RemoveRocks();

	void CreateProcMesh();

	bool MeshCreated = false;
	
	static int CubeSize;
	static float Seed;
	static int Scale;
	static int Octaves;
	static float SurfaceFrequency;
	static float CaveFrequency;
	static int NoiseScale;
	static int SurfaceLevel;
	static int CaveLevel;
	static int OverallNoiseScale;
	static int SurfaceNoiseScale;
	static int CaveNoiseScale;
	static bool GenerateCaves;

	int TreeNoiseScale;
	int TreeOctaves;
	float TreeFrequency;
	float TreeNoiseValueLimit;
	
	int WaterLevel;
	float WaterNoiseScale;
	int WaterOctaves;
	float WaterFrequency;
	float WaterNoiseValueLimit;

	float RockNoiseScale;
	int	  RockOctaves;
	float RockFrequency;
	float RockNoiseValueLimit;

	float GrassNoiseScale;
	int   GrassOctaves;
	float GrassFrequency;
	float GrassNoiseValueLimit;

	float AnimalNoiseScale;
	int   AnimalOctaves;
	float AnimalFrequency;
	float AnimalNoiseValueLimit;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		UMaterialInterface* Material;

	UPROPERTY()
		UProceduralMeshComponent* ProcMesh;

	UPROPERTY()
		UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		bool UseStaticMesh = false;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		int GridSizeX = 256;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		int GridSizeY = 256;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		int GridSizeZ = 1800;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		bool CreateCollision = true;

	UPROPERTY()
		int32 SectionIndex;

	UPROPERTY()
		TArray< FVector > Vertices;

	UPROPERTY()
		TArray< int32 > Triangles;

	UPROPERTY()
		TArray< FVector > Normals;

	UPROPERTY()
		TArray< FVector2D > UV0;

	UPROPERTY()
		TArray< FVector4 > VertexColour;

	UPROPERTY()
		TArray< FColor > ProcVertexColour;

	UPROPERTY()
		TArray <FProcMeshTangent> Tangents;

	TArray<FIndex3i> MCTriangles;

	TArray<FVector3d> MCVertices;

	UPROPERTY()
		TSubclassOf<class ATree> TreeClass;

	UPROPERTY()
	TArray<UStaticMesh*> TreeMeshList;

	TArray<ATree*> TreeList;

	UPROPERTY()
		TSubclassOf<class ARock> RockClass;

	UPROPERTY()
		TArray<UStaticMesh*> RockMeshList;

	TArray<ARock*> RockList;

	UPROPERTY()
		TSubclassOf<class AGrass> GrassClass;

	UPROPERTY()
		TArray<UStaticMesh*> GrassMeshList;

	TArray<AGrass*> GrassList;

	UPROPERTY()
		TArray<TSubclassOf<AAnimal>> AnimalClassList;

	TArray<AAnimal*> AnimalList;

};
