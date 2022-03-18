// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Animal.generated.h"

UCLASS()
class PROCGENPROJECT_API AAnimal : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AAnimal();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void FindNewLocation();

	UPROPERTY(EditAnywhere)
		USkeletalMeshComponent* AnimalMesh;

	UPROPERTY(EditAnywhere)
		int MaxRoamRange = 100;

	UPROPERTY(EditAnywhere)
		int MovementSpeed = 10;

	UPROPERTY(VisibleAnywhere)
		int Scale = 200;

	UPROPERTY(VisibleAnywhere)
		int ChunkHeight = 1800;

	UPROPERTY(VisibleAnywhere)
		int CaveLevel = 700;

	UPROPERTY(VisibleAnywhere)
		FVector Location = { 0,0,0 };
};
