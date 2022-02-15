// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProceduralMeshComponent.h"
#include "Generators/MarchingCubes.h"
#include "VectorTypes.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

	//void AssignTriangles();
	//void AssignColours();

	UFUNCTION()
		void CreateMesh();

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		UProceduralMeshComponent* ProcMesh;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		int Scale = 100;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		int GridSizeX = 256;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		int GridSizeY = 256;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		int GridSizeZ = 1024;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		FVector2D ChunkPos = { 0,0 };

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

	UPROPERTY(EditAnywhere, Category = "Mesh")
		bool CreateCollision;

	FMarchingCubes MarchingCubes;

};
