// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProceduralMeshComponent.h"

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

	static float FractalBrownianMotion(FVector fractalInput);

	void AssignTriangles();
	void AssignColours();

	UFUNCTION(CallInEditor, Category = "ProcMesh")
		void CreateMesh();

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		UProceduralMeshComponent* ProcMesh;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		int Scale = 100;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		int GridSizeX = 5000;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		int GridSizeY = 5000;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		int GridSizeZ = 1000;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		int32 SectionIndex;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		TArray< FVector > Vertices;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		TArray< int32 > Triangles;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		TArray< FVector > Normals;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		TArray< FVector2D > UV0;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		TArray< FColor > VertexColour;

	UPROPERTY(EditAnywhere, Category = "ProcMesh")
		TArray <FProcMeshTangent> Tangents;

	UPROPERTY(EditAnywhere, Category = "Mesh")
		bool CreateCollision;


};
