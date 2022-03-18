// Fill out your copyright notice in the Description page of Project Settings.


#include "Animal.h"

// Sets default values
AAnimal::AAnimal()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AnimalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Animal Mesh"));
	SetRootComponent(AnimalMesh);
}

// Called when the game starts or when spawned
void AAnimal::BeginPlay()
{
	Super::BeginPlay();
	FindNewLocation();
}

// Called every frame
void AAnimal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//auto Rotator = FRotationMatrix::MakeFromY(GetActorLocation() - Location).Rotator();
	//FaceRotation(Rotator);
	//auto NewLocation = GetActorLocation();
	//NewLocation.Y += (MovementSpeed * Scale) * DeltaTime;
	//SetActorLocation(NewLocation);

	//if ((GetActorLocation() - Location).Size() < 10)
	//{
	//	FindNewLocation();
	//}
}

// Called to bind functionality to input
void AAnimal::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AAnimal::FindNewLocation()
{
	auto RandX = (FMath::RandRange(0, MaxRoamRange / 2) * Scale);
	auto RandY = (FMath::RandRange(0, MaxRoamRange / 2) * Scale);
	FHitResult Hit;
	FVector Start = { float((GetActorLocation().X) + RandX),float((GetActorLocation().Y + RandY)), float(ChunkHeight * Scale) };
	FVector End = { float((GetActorLocation().X) + RandX),float((GetActorLocation().Y * Scale) + RandY), float(CaveLevel * Scale) };
	ECollisionChannel Channel = ECC_Visibility;
	FCollisionQueryParams Params;
	ActorLineTraceSingle(Hit, Start, End, Channel, Params);
	Location = Hit.Location;
}

