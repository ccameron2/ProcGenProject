// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "Generators/MarchingCubes.h"
#include "VectorTypes.h"
#include "GameFramework/Actor.h"
//#include "Components/RuntimeMeshComponentStatic.h"
#include "Tree.h"
#include "Rock.h"
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

	void Init(int cubeSize, float seed, int scale, int chunkSize, int chunkHeight, int octaves, float surfaceFrequency, float caveFrequency,
				float noiseScale, int surfaceLevel, int caveLevel, int overallNoiseScale, int surfaceNoiseScale, bool generateCaves, float caveNoiseScale,
					float treeNoiseScale, int treeOctaves, float treeFrequency, float treeNoiseValueLimit, float rockNoiseScale, int rockOctaves, float rockFrequency,
						float rockNoiseValueLimit, int waterLevel);

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

	UPROPERTY(VisibleAnywhere, Category = "ProcMesh")
		UMaterialInterface* Material;

	UPROPERTY(VisibleAnywhere, Category = "ProcMesh")
		UMaterialInterface* WaterMeshMaterial;

	UPROPERTY(VisibleAnywhere, Category = "ProcMesh")
		UProceduralMeshComponent* ProcMesh;

	UPROPERTY(VisibleAnywhere, Category = "StaticMesh")
		UStaticMeshComponent* StaticMesh;

	//UPROPERTY(VisibleAnywhere, Category = "ProcMesh")
	//	URuntimeMeshComponentStatic* RuntimeMesh;

	UPROPERTY(VisibleAnywhere, Category = "ProcMesh")
		int GridSizeX = 256;

	UPROPERTY(VisibleAnywhere, Category = "ProcMesh")
		int GridSizeY = 256;

	UPROPERTY(VisibleAnywhere, Category = "ProcMesh")
		int GridSizeZ = 1800;

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
		TArray <FProcMeshTangent> Tangents;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		bool CreateCollision = true;

	TArray<FIndex3i> MCTriangles;

	TArray<FVector3d> MCVertices;


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

	UPROPERTY(EditAnywhere, Category = "Trees")
		TSubclassOf<class ATree> TreeClass;

	UPROPERTY(EditAnywhere, Category = "Trees")
	TArray<UStaticMesh*> TreeMeshList;

	TArray<ATree*> TreeList;

	UPROPERTY(EditAnywhere, Category = "Trees")
		TSubclassOf<class ARock> RockClass;

	UPROPERTY(EditAnywhere, Category = "Trees")
		TArray<UStaticMesh*> RockMeshList;

	TArray<ARock*> RockList;
};
