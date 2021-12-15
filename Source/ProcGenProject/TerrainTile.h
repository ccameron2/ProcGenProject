// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProceduralMeshComponent.h"

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

	void AssignTriangles();
	void AssignColours();

	UFUNCTION(CallInEditor, Category = "ProcGen")
		void CreateMesh();

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		UProceduralMeshComponent* ProcMesh;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		int Scale = 100;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		int GridSizeX = 16;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		int GridSizeY = 16;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		int32 SectionIndex;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		TArray< FVector > Vertices;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		TArray< int32 > Triangles;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		TArray< FVector > Normals;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		TArray< FVector2D > UV0;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		TArray< FColor > VertexColour;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		TArray <FProcMeshTangent> Tangents;

	UPROPERTY(EditAnywhere, Category = "ProcGen")
		bool CreateCollision;


};
