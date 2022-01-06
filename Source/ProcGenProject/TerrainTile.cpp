// Fill out your copyright notice in the DescrSiption page of Project Settings.
#include "TerrainTile.h"
#include <functional>
#include "KismetProceduralMeshLibrary.h"
#include "Generators/MarchingCubes.h"
#include "HAL/Runnable.h"

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

	//CreateMesh();
	FMarchingCubes MarchingCubes;
	FAxisAlignedBox3d BoundingBox(FVector3d{0,0,0}, FVector3d{double(GridSizeX) , double(GridSizeY) , double(GridSizeZ) });
	MarchingCubes.Bounds = BoundingBox;
	//MarchingCubes.bParallelCompute = true;
	//MarchingCubes.CellDimensions = FVector3i{ 100,100,100 };
	MarchingCubes.Implicit = ATerrainTile::PerlinWrapper;
	MarchingCubes.CubeSize = 50;
	MarchingCubes.IsoValue = 0;
	MarchingCubes.Generate();

	//Convert FIndex3i to Int32
	for (int i = 0; i < MarchingCubes.Triangles.Num(); i++)
	{
		Triangles.Push(int32(MarchingCubes.Triangles[i].A));
		Triangles.Push(int32(MarchingCubes.Triangles[i].B));
		Triangles.Push(int32(MarchingCubes.Triangles[i].C));
	}

	Vertices = TArray<FVector>(MarchingCubes.Vertices);
	Normals = TArray<FVector>(MarchingCubes.Normals);
	UV0 = TArray<FVector2D>(MarchingCubes.UVs);
	ProcMesh->ClearAllMeshSections();
	ProcMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColour, Tangents, CreateCollision);

}

void ATerrainTile::CreateMesh()
{
	ProcMesh->ClearAllMeshSections();
	Vertices.Empty();
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
		Vertices[i].Z = Scale * FMath::PerlinNoise2D(FVector2D{ Vertices[i].X, Vertices[i].Y });/* FractalBrownianMotion(Vertices[i].X, Vertices[i].Y, Vertices[i].Z, NumOctaves, Lacunarity, Gain); */
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

double ATerrainTile::PerlinWrapper(FVector3<double> perlinInput)
{
	//double test = double(FractalBrownianMotion(FVector(perlinInput))) * 10000;
	//return test;

	perlinInput = perlinInput / 500;
	double test = FMath::PerlinNoise3D(FVector(perlinInput));
	return test;

}

float ATerrainTile::FractalBrownianMotion(FVector fractalInput)
{
	int scale = 10;

	float NumOctaves = 8;

	float Lacunarity = 4;

	float Gain = 0.5;

	float Sum = 0;
	float Amplitude = 1;
	float Frequency = 1;
	for (int i = 0; i < NumOctaves; i++)
	{
		Sum += Amplitude * FMath::PerlinNoise3D((FVector((fractalInput.X * Frequency) /scale, 
														(fractalInput.Y * Frequency) / scale, 
														(fractalInput.Z * Frequency)) / scale));
		Amplitude += Gain;
		Frequency += Lacunarity;
	}
	return Sum / NumOctaves;
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

