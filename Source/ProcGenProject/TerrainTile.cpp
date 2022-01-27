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

	//Create Procedural mesh and attach to root component
	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Procedural Mesh"));
	ProcMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ATerrainTile::BeginPlay()
{
	Super::BeginPlay();

}

//void ATerrainTile::CreateMesh()
//{
//	ProcMesh->ClearAllMeshSections();
//	Vertices.Empty();
//	for (int i = 0; i < GridSizeX; i++)
//	{
//		for (int j = 0; j < GridSizeY; j++)
//		{
//			Vertices.Push(FVector{float(i) * Scale,float(j) * Scale,0});
//		}
//	}
//	float counter = 0;
//	for (int i = 0; i < Vertices.Num(); i++)
//	{
//		counter += FMath::FRand();
//		Vertices[i].Z = Scale * FMath::PerlinNoise2D(FVector2D{ Vertices[i].X, Vertices[i].Y });/* FractalBrownianMotion(Vertices[i].X, Vertices[i].Y, Vertices[i].Z, NumOctaves, Lacunarity, Gain); */
//	}
//	AssignTriangles();
//	ProcMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColour, Tangents, CreateCollision);
//}

//void ATerrainTile::AssignColours()
//{
//	VertexColour.Init(FColor::Green, GridSizeX * GridSizeY);
//	for (int i = 0; i < GridSizeX * GridSizeY; i++)
//	{
//		if (Vertices[i].Z > 10)
//		{
//			VertexColour[i] = FColor::Silver;
//		}
//		else
//		{
//			VertexColour[i] = FColor::Green;
//		}
//	}
//}

double ATerrainTile::PerlinWrapper(FVector3<double> perlinInput)
{
	//double test = double(FractalBrownianMotion(FVector(perlinInput))) * 10000;
	//return test;
	
	int noiseScale = 50;

	//Scale noise input
	FVector3d noiseInput = perlinInput / noiseScale;

	//double test2DPerlin = FMath::PerlinNoise2D(FVector2D(perlinInput.X, perlinInput.Z));
	//double test = FractalBrownianMotion(FVector(perlinInput));

	float density = ( -noiseInput.Z / 3.5) + 1;
	//float density = 0;
	
	//if (perlinInput.Z > 1.5)
	//{
	//	density = 0;
	//	//density -= 0.5;
	//}

	//if (perlinInput.X > FMath::PerlinNoise2D(FVector2D(perlinInput.X + 5654, perlinInput.Y + 5654)) && perlinInput.Y > FMath::PerlinNoise2D(FVector2D(perlinInput.X + 5654, perlinInput.Y + 5654)))
	//{
	//	if (FMath::PerlinNoise2D(FVector2D(perlinInput.X, perlinInput.Y)) > 0)
	//	{
	//		density = -perlinInput.Z + 1;
	//	}
	//}

	//Add 3D noise partially
	density += FractalBrownianMotion(FVector(noiseInput) / 2, 9,0.1);

	//Add 2D noise
	density += FractalBrownianMotion(FVector(noiseInput.X, noiseInput.Y, 0),6,0.2);

	float density2 = FractalBrownianMotion(FVector(noiseInput), 9, 1);

	if (perlinInput.Z > 96)
	{
		return density;
	}
	else if(perlinInput.Z < 64)
	{
		return density2;
	}
	else
	{
		FMath::Lerp(density, density2, (perlinInput.Z - 64)/100);
	}

	return density;
}

float ATerrainTile::FractalBrownianMotion(FVector fractalInput, float octaves, float frequency)
{
	//The book of shaders

	float result = 0;
	float amplitude = 0.5;
	//float frequency = 0.5;
	//float octaves = 6;
	float lacunarity = 2.0;
	float gain = 0.5;

	for (int i = 0; i < octaves; i++)
	{
		result += amplitude * FMath::PerlinNoise3D(frequency * fractalInput);
		frequency *= lacunarity;
		amplitude *= gain;
	}

	return result;
}

void ATerrainTile::CreateMesh()
{
	//Marching Cubes
	FMarchingCubes MarchingCubes;
	FAxisAlignedBox3d BoundingBox(GetActorLocation(), FVector3d(GetActorLocation()) + FVector3d{ double(GridSizeX) , double(GridSizeY) , double(GridSizeZ) });
	MarchingCubes.Bounds = BoundingBox;
	MarchingCubes.bParallelCompute = true;
	//MarchingCubes.CellDimensions = FVector3i{ 100,100,100 };
	MarchingCubes.Implicit = ATerrainTile::PerlinWrapper;
	MarchingCubes.CubeSize = 5;
	MarchingCubes.IsoValue = 0;
	MarchingCubes.Generate();

	Triangles.Empty();
	Vertices.Empty();
	Normals.Empty();
	Tangents.Empty();
	UV0.Empty();


	//Convert FIndex3i to Int32
	for (int i = 0; i < MarchingCubes.Triangles.Num(); i++)
	{
		Triangles.Push(int32(MarchingCubes.Triangles[i].A));
		Triangles.Push(int32(MarchingCubes.Triangles[i].B));
		Triangles.Push(int32(MarchingCubes.Triangles[i].C));
	}

	//Scale the vertices
	for (int i = 0; i < MarchingCubes.Vertices.Num(); i++)
	{
		MarchingCubes.Vertices[i] *= Scale;
	}
	Vertices = TArray<FVector>(MarchingCubes.Vertices);

	UV0 = TArray<FVector2D>(MarchingCubes.UVs);

	//Calculate normals and tangents for textures
	UKismetProceduralMeshLibrary* procLib;
	procLib->CalculateTangentsForMesh(Vertices, Triangles, UV0, Normals, Tangents);

	//Normals = TArray<FVector>(MarchingCubes.Normals);

	//Create Procedural Mesh Section with Marching Cubes data
	ProcMesh->ClearAllMeshSections();
	ProcMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColour, Tangents, CreateCollision);
}



//void ATerrainTile::AssignTriangles()
//{
//	Triangles.Init(0, GridSizeX * GridSizeY * 6);
//	UKismetProceduralMeshLibrary* procLib;
//	procLib->CreateGridMeshTriangles(GridSizeX,GridSizeY, false, Triangles);
//}

// Called every frame
void ATerrainTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATerrainTile::OnConstruction(const FTransform& Transform)
{
	CreateMesh();
}

