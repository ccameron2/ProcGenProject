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

	// Implicit function to evaluate with marching cubes algorithm to generate density values
	static double PerlinWrapper(FVector3<double> perlinInput);

	// Fractal brownian motion to create more detailed noise at desired frequency and octaves
	static float FractalBrownianMotion(FVector fractalInput, float octaves, float frequency);
	
	// Calculate normals to use in terrain material
	void CalculateNormals();

	// Place trees on terrain
	void CreateTrees();

	// Remove trees from terrain
	void RemoveTrees();

	// Place grass on terrain
	void CreateGrass();

	// Remove grass from terrain
	void RemoveGrass();

	// Create animals on terrain
	void CreateAnimals();

	// Remove animals from terrain
	void RemoveAnimals();

	// Place rocks on terrain
	void CreateRocks();

	// Remove rocks from terrain
	void RemoveRocks();

	// Receive generation parameters and mesh lists to use for scenery from manager
	void Init(int cubeSize, float seed, int scale, int chunkSize, int chunkHeight, int octaves, float surfaceFrequency, float caveFrequency,
			  float noiseScale, int surfaceLevel, int caveLevel, int overallNoiseScale, int surfaceNoiseScale, bool generateCaves, float caveNoiseScale,
			  float treeNoiseScale, int treeOctaves, float treeFrequency, float treeNoiseValueLimit, TSubclassOf<class ATree> treeClass, TArray<UStaticMesh*> treeMeshList,
			  float rockNoiseScale, int rockOctaves, float rockFrequency, float rockNoiseValueLimit, TSubclassOf<class ARock> rockClass, TArray<UStaticMesh*> rockMeshList, 
			  int waterLevel, float grassNoiseScale, int grassOctaves, float grassFrequency, float grassNoiseValueLimit, TSubclassOf<class AGrass> grassClass, 
			  TArray<UStaticMesh*> grassMeshList, float animalNoiseScale, int animalOctaves, float animalFrequency, float animalNoiseValueLimit, TArray<TSubclassOf<AAnimal>> animalClassList,
			  bool useStaticMesh);

	// Generate parameters used to generate mesh
	void GenerateTerrain();

	// Use generated values to create a mesh
	void CreateProcMesh();

	// Determines if mesh has already been created
	bool MeshCreated = false;
	
	// Static variables used in PerlinWrapper
	// Received from manager in init function
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

	// Variables for placing trees on terrain
	int TreeNoiseScale;
	int TreeOctaves;
	float TreeFrequency;
	float TreeNoiseValueLimit;
	
	// Where water is placed in the world
	int WaterLevel;

	// Variables for placing rocks on terrain
	float RockNoiseScale;
	int	  RockOctaves;
	float RockFrequency;
	float RockNoiseValueLimit;

	// Variables for placing grass on terrain
	float GrassNoiseScale;
	int   GrassOctaves;
	float GrassFrequency;
	float GrassNoiseValueLimit;

	// Variables for placing animals on terrain
	float AnimalNoiseScale;
	int   AnimalOctaves;
	float AnimalFrequency;
	float AnimalNoiseValueLimit;

	// Terrain material
	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		UMaterialInterface* Material;

	// Procedural Mesh Component 
	UPROPERTY()
		UProceduralMeshComponent* ProcMesh;

	// Static Mesh Component
	UPROPERTY()
		UStaticMeshComponent* StaticMesh;

	// Which type of mesh to generate
	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		bool UseStaticMesh = false;

	// Tile generation bounds
	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		int GridSizeX = 256;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		int GridSizeY = 256;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		int GridSizeZ = 1800;

	// Determines if collision should be created
	// Required for scenery to be placed on terrain
	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		bool CreateCollision = true;

	// Data for mesh building
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

	// Triangles received from marching cubes
	TArray<FIndex3i> MCTriangles;

	// Vertices received from marching cubes
	TArray<FVector3d> MCVertices;

	// Class to use when placing trees
	UPROPERTY()
		TSubclassOf<class ATree> TreeClass;

	// Meshes to use when placing trees
	UPROPERTY()
	TArray<UStaticMesh*> TreeMeshList;

	// List of trees
	UPROPERTY()
	TArray<ATree*> TreeList;

	// Class to use when placing rocks
	UPROPERTY(EditAnywhere)
		TSubclassOf<class ARock> RockClass;

	// Meshes to use when placing rocks
	UPROPERTY()
		TArray<UStaticMesh*> RockMeshList;

	// List of rocks
	UPROPERTY()
	TArray<ARock*> RockList;

	// Class to use when placing grass
	UPROPERTY()
		TSubclassOf<class AGrass> GrassClass;

	// Meshes to use when placing grass
	UPROPERTY()
		TArray<UStaticMesh*> GrassMeshList;

	// List of grass 
	UPROPERTY()
	TArray<AGrass*> GrassList;

	// Classes to use when placing animals
	UPROPERTY()
		TArray<TSubclassOf<AAnimal>> AnimalClassList;

	// List of animals
	UPROPERTY()
	TArray<AAnimal*> AnimalList;

	FVector2D CalculateUV(const FVector& normal);

};
