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
	static ConstructorHelpers::FObjectFinder<UMaterial> TerrainMaterial(TEXT("Material'/Game/M_Terrain_Mesh'"));
	Material = TerrainMaterial.Object;
	ProcMesh->SetMaterial(0, Material);
}

// Called when the game starts or when spawned
void ATerrainTile::BeginPlay()
{
	Super::BeginPlay();
	CreateMesh();
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

	float density = ( -noiseInput.Z / 15) + 1;
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
	density += FractalBrownianMotion(FVector(noiseInput) / 5, 6,0.04);
	
	//Add 2D noise
	density += FractalBrownianMotion(FVector(noiseInput.X, noiseInput.Y, 0),8,0.06);

	//density = FMath::PerlinNoise2D(FVector2D(noiseInput.X, noiseInput.Y));

	float density2 = FractalBrownianMotion(FVector(noiseInput / 5), 8, 1);

	if (perlinInput.Z >= 768)
	{
		return density;
	}
	else if(perlinInput.Z < 512)
	{
		return density2;
	}
	else
	{
		return FMath::Lerp(density2, density, (perlinInput.Z - 512) / (768 - 512));
	}
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
	MarchingCubes.CubeSize = 4;
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

	//Calculate normals for textures
	//UKismetProceduralMeshLibrary* procLib;
	//procLib->CalculateTangentsForMesh(Vertices, Triangles, UV0, Normals, Tangents);

	//Normals = TArray<FVector>(MarchingCubes.Normals);

	CalculateNormals();


	//for (int i = 0; i < Vertices.Num(); i += 3)
	//{
	//	if (i > Vertices.Num() - 3) { break; }
	//	FVector edge1 = Vertices[i + 1] - Vertices[i + 2];
	//	FVector edge2 = Vertices[i] - Vertices[i + 2];
	//	FVector normal = FVector::CrossProduct(edge1, edge2);
	//	normal.Normalize();
	//	Normals[i] += normal;
	//	Normals[i + 1] += normal;
	//	Normals[i + 2] += normal; 
	//}

	//Create Procedural Mesh Section with Marching Cubes data
	ProcMesh->ClearAllMeshSections();
	ProcMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColour, Tangents, CreateCollision);
}

void ATerrainTile::CalculateNormals()
{
	Normals.Init({ 0,0,0 }, Vertices.Num());
	for (int i = 0; i < Vertices.Num(); i++)
	{
		FVector nP = { 0,0,0 }; //North point
		FVector sP = { 0,0,0 }; //South point
		FVector eP = { 0,0,0 }; //East point
		FVector wP = { 0,0,0 }; //West point
		FVector cP = { 0,0,0 }; //Centre point

		if (i == 0) //North West Corner
		{
			cP = Vertices[i];
			sP = Vertices[i + GridSizeX];
			eP = Vertices[i + 1];
			Normals[i] = FVector{ (cP.Z - nP.Z) * 2, (eP.Z - cP.Z) * 2,1 };
		}
		else if (i == GridSizeX - 1) //North East Corner
		{
			cP = Vertices[i];
			sP = Vertices[i + GridSizeX];
			wP = Vertices[i - 1];
			Normals[i] = FVector{ (sP.Z - cP.Z) * 2, (wP.Z - cP.Z) * 2, 1 };
		}
		else if (i == Vertices.Num() - GridSizeX) //South West Corner
		{
			cP = Vertices[i];
			nP = Vertices[i - GridSizeX];
			eP = Vertices[i + 1];
			Normals[i] = FVector{ (cP.Z - nP.Z) * 2,(eP.Z - cP.Z) * 2, 1 };
		}
		else if (i == Vertices.Num() - 1) //South East Corner
		{
			cP = Vertices[i];
			nP = Vertices[i - GridSizeX];
			wP = Vertices[i - 1];
			Normals[i] = FVector{ (cP.Z - nP.Z) * 2, (cP.Z - wP.Z) * 2, 1 };
		}
		else if (i < GridSizeX) //North Row
		{
			cP = Vertices[i];
			sP = Vertices[i + GridSizeX];
			eP = Vertices[i + 1];
			wP = Vertices[i - 1];
			Normals[i] = FVector{ (sP.Z - cP.Z) * 2, eP.Z - wP.Z, 1 };
		}
		else if (GridSizeX % i == 0) //West row
		{
			nP = Vertices[i - GridSizeX];
			sP = Vertices[i + GridSizeX];
			cP = Vertices[i];
			eP = Vertices[i + 1];
			Normals[i] = FVector{ sP.Z - nP.Z, (eP.Z - cP.Z) * 2, 1 };
		}
		else if (GridSizeX % i == 1) //East row
		{
			nP = Vertices[i - GridSizeX];
			sP = Vertices[i + GridSizeX];
			cP = Vertices[i];
			wP = Vertices[i - 1];
			Normals[i] = FVector{ sP.Z - nP.Z, (cP.Z - wP.Z) * 2, 1 };
		}
		else if (i > Vertices.Num() - GridSizeX) //South row
		{
			nP = Vertices[i - GridSizeX];
			cP = Vertices[i];
			eP = Vertices[i + 1];
			wP = Vertices[i - 1];
			Normals[i] = FVector{ (cP.Z - nP.Z) * 2, eP.Z - wP.Z, 1 };
		}
		else
		{
			nP = Vertices[i - GridSizeX];
			sP = Vertices[i + GridSizeX];
			eP = Vertices[i + 1];
			wP = Vertices[i - 1];
			Normals[i] = FVector{ sP.Z - nP.Z, wP.Z - eP.Z, 1 };
		}
	}

	for (auto normal : Normals)
	{
		normal.Normalize();
	}
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
}

