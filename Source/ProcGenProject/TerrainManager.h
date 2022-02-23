// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TerrainTile.h"

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
	UPROPERTY(EditAnywhere,Category = "Chunks")
		int TileX = 10;
	UPROPERTY(EditAnywhere, Category = "Chunks")
		int TileY = 10;
	UPROPERTY(EditAnywhere, Category = "Chunks")
		int ChunkSize = 256;

	//From TerrainTile
	UPROPERTY(EditAnywhere, Category = "Chunks")
		int Scale = 100;

	FVector2D GetPlayerGridPosition();
	FVector2D GetTilePosition(int index);
	bool IsAlreadyThere(FVector2D position);
	bool hasRun = false;
	FVector2D LastPlayerPosition = { 0,0 };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
