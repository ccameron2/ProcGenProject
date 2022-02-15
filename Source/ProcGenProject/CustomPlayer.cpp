// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomPlayer.h"

// Sets default values
ACustomPlayer::ACustomPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACustomPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACustomPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ACustomPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(InputComponent);

	//Bind inputs to assigned functions
	InputComponent->BindAxis("Move Forwards", this, &ACustomPlayer::MoveForward);
	InputComponent->BindAxis("Turn", this, &ACustomPlayer::Turn);
	InputComponent->BindAxis("Look Up", this, &ACustomPlayer::LookUp);
	InputComponent->BindAxis("Strafe", this, &ACustomPlayer::Strafe);
	InputComponent->BindAxis("Move Down", this, &ACustomPlayer::MoveDown);
}

void ACustomPlayer::MoveForward(float AxisValue)
{
	//Move character forward
	AddMovementInput(GetActorForwardVector() * AxisValue);
}

void ACustomPlayer::Turn(float AxisValue)
{
	//Turn character left and right
	AddControllerYawInput(AxisValue);
}

void ACustomPlayer::LookUp(float AxisValue)
{
	//Rotate view up and down
	AddControllerPitchInput(AxisValue);
}

void ACustomPlayer::Strafe(float AxisValue)
{
	//Move character right
	AddMovementInput(GetActorRightVector() * AxisValue);
}

void ACustomPlayer::MoveDown(float AxisValue)
{
	//Move character down
	AddMovementInput(GetActorUpVector() * AxisValue);
}