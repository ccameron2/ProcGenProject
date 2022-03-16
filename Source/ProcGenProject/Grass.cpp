// Fill out your copyright notice in the Description page of Project Settings.


#include "Grass.h"

// Sets default values
AGrass::AGrass()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GrassMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grass Mesh"));
	SetRootComponent(GrassMesh);
}

// Called when the game starts or when spawned
void AGrass::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGrass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

