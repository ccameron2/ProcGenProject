// Fill out your copyright notice in the Description page of Project Settings.


#include "Rock.h"

// Sets default values
ARock::ARock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	RockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tree Mesh"));
	SetRootComponent(RockMesh);
}

// Called when the game starts or when spawned
void ARock::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

