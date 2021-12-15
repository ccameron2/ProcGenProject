// Fill out your copyright notice in the Description page of Project Settings.

#include "KismetProceduralMeshLibrary.h"
#include "TerrainTile.h"
// Sets default values
ATerrainTile::ATerrainTile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Procedural Mesh"));
	ProcMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ATerrainTile::BeginPlay()
{
	Super::BeginPlay();
	CreateMesh();
}

void ATerrainTile::CreateMesh()
{
	ProcMesh->ClearAllMeshSections();
	for (int i = 0; i < GridSizeX; i++)
	{
		for (int j = 0; j < GridSizeY; j++)
		{
			Vertices.Push(FVector{float(i) * Scale,float(j) * Scale,0});
		}
	}
	float counter = 0;
	for (int i = 0; i < Vertices.Num(); i++)
	{
		counter += FMath::FRand();
		Vertices[i].Z = Scale * FMath::PerlinNoise1D(counter);
	}
	AssignTriangles();
	ProcMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColour, Tangents, CreateCollision);
}

void ATerrainTile::AssignColours()
{
	VertexColour.Init(FColor::Green, GridSizeX * GridSizeY);
	for (int i = 0; i < GridSizeX * GridSizeY; i++)
	{
		if (Vertices[i].Z > 10)
		{
			VertexColour[i] = FColor::Silver;
		}
		else
		{
			VertexColour[i] = FColor::Green;
		}
	}
}

void ATerrainTile::AssignTriangles()
{
	Triangles.Init(0, GridSizeX * GridSizeY * 6);
	UKismetProceduralMeshLibrary* procLib;
	procLib->CreateGridMeshTriangles(GridSizeX,GridSizeY, false, Triangles);
}

// Called every frame
void ATerrainTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATerrainTile::OnConstruction(const FTransform& Transform)
{
	
}

