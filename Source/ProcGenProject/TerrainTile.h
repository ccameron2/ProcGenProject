// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProceduralMeshComponent.h"
#include "Generators/MarchingCubes.h"
#include "VectorTypes.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HAL/Runnable.h"
#include "CustomWorker.h"
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

	//void AssignTriangles();
	//void AssignColours();

	void Init(bool useCustomMultithreading, float seed, int scale, int octaves, float surfaceFrequency,
				float caveFrequency, int noiseScale, int surfaceLevel, int caveLevel, int surfaceNoiseScale,
					int caveNoiseScale, int treeNoiseScale, int treeOctaves, float treeFrequency, float treeNoiseValueLimit);
	
	bool UseCustomMultithreading;
	static float Seed;

	static int Scale;
	static int Octaves;
	static float SurfaceFrequency;
	static float CaveFrequency;
	static int NoiseScale;
	static int SurfaceLevel;
	static int CaveLevel;
	static int SurfaceNoiseScale;
	static int CaveNoiseScale;

	int TreeNoiseScale;
	int TreeOctaves;
	float TreeFrequency;
	float TreeNoiseValueLimit;

	void CreateMesh();

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		UProceduralMeshComponent* ProcMesh;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		int GridSizeX = 256;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		int GridSizeY = 256;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		int GridSizeZ = 1800;

	UPROPERTY(VisibleAnywhere, Category = "ProcMesh")
		FVector2D ChunkPos = { 0,0 };

	UPROPERTY(EditAnywhere, Category = "Water")
		UStaticMeshComponent* WaterMesh;

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
		TArray< FColor > VertexColour;

	UPROPERTY()
		TArray <FProcMeshTangent> Tangents;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		bool CreateCollision = true;

	TArray<FIndex3i> MCTriangles;

	TArray<FVector3d> MCVertices;

	UPROPERTY(EditAnywhere, Category = "Trees")
		TSubclassOf<class ATree> TreeClass;

	UPROPERTY(EditAnywhere, Category = "Trees")
	TArray<UStaticMesh*> treeMeshList;

	TArray<ATree*> TreeList;

	static bool WaterMeshAdded;



};
