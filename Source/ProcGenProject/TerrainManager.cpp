// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainManager.h"
#include "Kismet\GameplayStatics.h"
#include <math.h>
#include "KismetProceduralMeshLibrary.h"
#include "delaunator.hpp"

#include "MeshDescription.h"
#include "MeshDescriptionBuilder.h"
#include "StaticMeshAttributes.h"

// Sets default values
ATerrainManager::ATerrainManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WaterMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Water Mesh"));
	WaterMesh->SetupAttachment(RootComponent);
	WaterMesh->SetMaterial(0, WaterMeshMaterial);
}

// Called when the game starts or when spawned
void ATerrainManager::BeginPlay()
{
	Super::BeginPlay();
	
	auto PlayerGridPosition = GetPlayerGridPosition();
	PlayerGridPosition.X = round(PlayerGridPosition.X);
	PlayerGridPosition.Y = round(PlayerGridPosition.Y);

	CreateTileArray();

	terrainWorker = std::make_unique<FTerrainWorker>(TileArray);
}

FVector2D ATerrainManager::GetPlayerGridPosition()
{
	FVector PlayerPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	FVector PlayerChunkPosition = PlayerPosition / (ChunkSize * Scale);
	return FVector2D(PlayerChunkPosition.X, PlayerChunkPosition.Y);
}

FVector2D ATerrainManager::GetTilePosition(int index)
{
	FVector TilePosition = TileArray[index]->GetActorLocation();
	FVector TileGridPosition = TilePosition / ChunkSize;
	return FVector2D(TileGridPosition.X,TileGridPosition.Y);
}


bool ATerrainManager::IsAlreadyThere(FVector2D position)
{
	for (int i = 0; i < TileArray.Num(); i++)
	{
		if (GetTilePosition(i) == position)
		{
			return true;
		}
	}
	return false;
}

// Called every frame
void ATerrainManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (terrainWorker)
	{
		if (terrainWorker->ThreadComplete)
		{
			for (auto& tile : TileArray)
			{
				if (!tile->MeshCreated)
				{					
					tile->CreateProcMesh();
					tile->MeshCreated = true;
					FTransform SpawnParams(tile->GetActorRotation(), tile->GetActorLocation());
					tile->FinishSpawning(SpawnParams);
					UpdateWater = true;
				}
			}
			if (UpdateWater)
			{
				CreateWaterMesh();
				UpdateWater = false;
			}
		}
	}

	auto PlayerGridPosition = GetPlayerGridPosition();
	PlayerGridPosition.X = round(PlayerGridPosition.X);
	PlayerGridPosition.Y = round(PlayerGridPosition.Y);

	if (terrainWorker->ThreadComplete)
	{
		for (int i = 0; i < TileArray.Num(); i++)
		{
			auto PlayerLocation = FVector{ PlayerGridPosition.X * 255, PlayerGridPosition.Y * 255,0 };
			//auto PlayerLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
			auto TileLocation = TileArray[i]->GetActorLocation();
			//TileLocation *= Scale;
			auto Distance = (PlayerLocation - TileLocation).Size();
			auto MaxDistance = (((RenderDistance + 1) * ChunkSize));

			if (Distance >= MaxDistance + 1)
			{
				TileArray[i]->RemoveTrees();
				TileArray[i]->RemoveRocks();
				TileArray[i]->RemoveGrass();
				TileArray[i]->RemoveAnimals();
				TileArray[i]->Destroy();
				TileArray.RemoveAt(i);
			}
		}
	}

	if (PlayerGridPosition != LastPlayerPosition)
	{
		
		CreateTileArray();
		if (terrainWorker->ThreadComplete)
		{
			terrainWorker->InputTiles(TileArray);
			terrainWorker->ThreadComplete = false;
		}
	}
	
	LastPlayerPosition = PlayerGridPosition;	
}

void ATerrainManager::CreateTileArray()
{
	auto PlayerGridPosition = GetPlayerGridPosition();
	PlayerGridPosition.X = round(PlayerGridPosition.X);
	PlayerGridPosition.Y = round(PlayerGridPosition.Y);

	for (int x = -RenderDistance; x < RenderDistance; x++)
	{
		for (int y = -RenderDistance; y < RenderDistance; y++)
		{
			if (!IsAlreadyThere(FVector2D{ PlayerGridPosition.X + x,PlayerGridPosition.Y + y }))
			{
				FVector Location(((PlayerGridPosition.X + x) * ChunkSize) /** Scale*/, ((PlayerGridPosition.Y + y) * ChunkSize) /** Scale*/, 0.0f);
				FRotator Rotation(0.0f, 0.0f, 0.0f);
				FTransform SpawnParams(Rotation, Location);

				ATerrainTile* tile = GetWorld()->SpawnActorDeferred<ATerrainTile>(TerrainClass, SpawnParams);

				tile->Init(CubeSize, Seed, Scale, ChunkSize, ChunkHeight, Octaves, SurfaceFrequency, CaveFrequency, NoiseScale,
					SurfaceLevel, CaveLevel, OverallNoiseScale, SurfaceNoiseScale, GenerateCaves, CaveNoiseScale, TreeNoiseScale,
					TreeOctaves, TreeFrequency, TreeNoiseValueLimit, RockNoiseScale, RockOctaves, RockFrequency, RockNoiseValueLimit,
					WaterLevel, GrassNoiseScale, GrassOctaves, GrassFrequency, GrassNoiseValueLimit, AnimalNoiseScale, AnimalOctaves, 
					AnimalFrequency, AnimalNoiseValueLimit);

				TileArray.Push(tile);
			}
		}
	}
}

void ATerrainManager::CreateWaterMesh()
{
	FVector PlayerPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	WaterMesh->ClearMeshSection(0);
	WaterVertices.Empty();
	WaterTriangles.Empty();
	WaterNormals.Empty();
	for (int i = -((RenderDistance + 1) * ChunkSize); i < ((RenderDistance + 1) * ChunkSize); i+=WaterStepSize)
	{
		for (int j = -((RenderDistance +1) * ChunkSize); j < ((RenderDistance + 1) * ChunkSize); j+=WaterStepSize)
		{
			//5,4,0.4,0.25
			float waterNoise = ATerrainTile::FractalBrownianMotion(FVector{ PlayerPosition.X + float(i), PlayerPosition.Y + float(j), 0 } / WaterNoiseScale, WaterOctaves, WaterFrequency);
			if (waterNoise > WaterNoiseValueLimit)
			{
				FHitResult Hit;
				FVector Start = { float((PlayerPosition.X) + (i * Scale)),float((PlayerPosition.Y) + (j * Scale)), float(ChunkHeight * Scale) };
				FVector End = { float((PlayerPosition.X) + (i * Scale)),float((PlayerPosition.Y) + (j * Scale)), float(CaveLevel * Scale) };
				ECollisionChannel Channel = ECC_Visibility;
				FCollisionQueryParams Params;
				GetWorld()->LineTraceSingleByChannel(Hit, Start, End, Channel, Params);
				FVector Offset = FVector{ 0,0,float(Scale * 1) };
				FVector Location = Hit.Location + Offset;
				if (Hit.Location != FVector{ 0, 0, 0 })
				{
					if (Hit.Location != Offset)
					{
						if (Hit.Location.Z < (WaterLevel)*Scale)
						{
							if (Hit.Location.Z < (WaterLevel) * Scale)
							{
								Location.Z = (WaterLevel) * Scale;
							}
							WaterVertices.Push(Location);
						}
					}
				}
			}
		}
	}

	std::vector<double> coords;

	bool inARowX = true;
	bool inARowY = true;
	float lastXVertex = 0;
	float lastYVertex = 0;

	for (auto& vertex : WaterVertices)
	{
		if (lastXVertex != 0)
		{
			if (vertex.X != lastXVertex)
			{

				inARowX = false;
			}
			if (vertex.Y != lastYVertex)
			{
				inARowY = false;
			}
		}
		coords.push_back(vertex.X);
		coords.push_back(vertex.Y);
		lastXVertex = vertex.X;
		lastYVertex = vertex.Y;
	}

	if (coords.size() > 6)
	{
		if (!inARowX && !inARowY)
		{
			//triangulation happens here
			delaunator::Delaunator d(coords);

			for (auto& triangle : d.triangles)
			{
				WaterTriangles.Push(triangle);
			}
		}
	}

	//for (auto& vertex : WaterVertices)
	//{
	//	vertex -= GetActorLocation();
	//}

	/*UKismetProceduralMeshLibrary* kismet;
	kismet->CalculateTangentsForMesh(WaterVertices, WaterTriangles, WaterUV0, WaterNormals, WaterTangents);*/

	/*WaterNormals.Init({ 0,0,0 }, WaterVertices.Num());*/

	//// Map of vertex to triangles in Triangles array
	//TArray<TArray<int32>> VertToTriMap;
	//VertToTriMap.Init(TArray<int32>{ int32{ -1 }, int32{ -1 }, int32{ -1 },
	//	int32{ -1 }, int32{ -1 }, int32{ -1 },
	//	int32{ -1 }, int32{ -1 } }, WaterVertices.Num());

	//// For each triangle for each vertex add triangle to vertex array entry
	//for (int i = 0; i < WaterTriangles.Num(); i++)
	//{
	//	for (int j = 0; j < 8; j++)
	//	{
	//		if (VertToTriMap[WaterTriangles[i]][j] < 0)
	//		{
	//			VertToTriMap[WaterTriangles[i]][j] = i / 3;
	//			break;
	//		}
	//	}
	//}

	//convert to findex3i to reuse normals code

	/*TArray<FIndex3i> triangleIndexes;
	triangleIndexes.Init(FIndex3i{ 0,0,0 }, WaterTriangles.Num() / 3);
	
	int triIndex = 0;
	for (int i = 0; i < WaterTriangles.Num() / 3; i+=3)
	{
		triangleIndexes[triIndex].A = WaterTriangles[i];
		triangleIndexes[triIndex].B = WaterTriangles[i+1];
		triangleIndexes[triIndex].C = WaterTriangles[i+2];
		triIndex++;
	}*/

	//	if (i >= WaterVertices.Num() - 3) break;


	//For each vertex collect the triangles that share it and calculate the face normal
	//for (int i = 0; i < WaterVertices.Num(); i++)
	//{
	//	for (auto& triangle : VertToTriMap[i])
	//	{
	//		//This shouldnt happen
	//		if (triangle < 0)
	//		{
	//			continue;
	//		}
	//		auto A = WaterVertices[triangleIndexes[triangle].A];
	//		auto B = WaterVertices[triangleIndexes[triangle].B];
	//		auto C = WaterVertices[triangleIndexes[triangle].C];
	//		auto E1 = A - B;
	//		auto E2 = C - B;
	//		auto Normal = E1 ^ E2;
	//		Normal.Normalize();
	//		WaterNormals[i] += Normal;
	//	}
	//}

	////Average the face normals
	//for (auto& normal : WaterNormals)
	//{
	//	normal.Normalize();
	//}


	//FMeshDescription meshDesc;
	//FStaticMeshAttributes Attributes(meshDesc);
	//Attributes.Register();

	//FMeshDescriptionBuilder meshDescBuilder;
	//meshDescBuilder.SetMeshDescription(&meshDesc);
	//meshDescBuilder.EnablePolyGroups();
	//meshDescBuilder.SetNumUVLayers(1);


	//TArray< FVertexID > vertexIDs;
	//vertexIDs.Init(FVertexID(0), WaterVertices.Num());

	//for (int i = 0; i < WaterVertices.Num(); i++)
	//{
	//	vertexIDs[i] = meshDescBuilder.AppendVertex(WaterVertices[i]);
	//}

	//TArray< FVertexInstanceID > vertexInstances;

	//WaterVertexColour.Init(FVector4{ 1,1,1,1 }, WaterVertices.Num());
	//WaterUV0.Init(FVector2D{ 0,0 }, WaterVertices.Num());
	//for (auto& triangle : triangleIndexes)
	//{
	//	FVertexInstanceID instance = meshDescBuilder.AppendInstance(vertexIDs[triangle.A]);
	//	meshDescBuilder.SetInstanceNormal(instance, WaterNormals[triangle.A]);
	//	meshDescBuilder.SetInstanceUV(instance, WaterUV0[triangle.A]);
	//	meshDescBuilder.SetInstanceColor(instance, WaterVertexColour[triangle.A]);
	//	vertexInstances.Add(instance);

	//	instance = meshDescBuilder.AppendInstance(vertexIDs[triangle.B]);
	//	meshDescBuilder.SetInstanceNormal(instance, WaterNormals[triangle.B]);
	//	meshDescBuilder.SetInstanceUV(instance, WaterUV0[triangle.B]);
	//	meshDescBuilder.SetInstanceColor(instance, WaterVertexColour[triangle.B]);
	//	vertexInstances.Add(instance);

	//	instance = meshDescBuilder.AppendInstance(vertexIDs[triangle.C]);
	//	meshDescBuilder.SetInstanceNormal(instance, WaterNormals[triangle.C]);
	//	meshDescBuilder.SetInstanceUV(instance, WaterUV0[triangle.C]);
	//	meshDescBuilder.SetInstanceColor(instance, WaterVertexColour[triangle.C]);
	//	vertexInstances.Add(instance);
	//}

	//// Allocate a polygon group
	//FPolygonGroupID polygonGroup = meshDescBuilder.AppendPolygonGroup();

	//for (int i = 0; i < vertexInstances.Num() / 3; i++)
	//{
	//	meshDescBuilder.AppendTriangle(vertexInstances[(i * 3)], vertexInstances[(i * 3) + 1], vertexInstances[i * 3 + 2], polygonGroup);
	//}

	//UStaticMesh* staticMesh = NewObject<UStaticMesh>(this);
	//staticMesh->GetStaticMaterials().Add(FStaticMaterial());

	//UStaticMesh::FBuildMeshDescriptionsParams mdParams;
	//mdParams.bBuildSimpleCollision = true;


	//// Build static mesh
	//TArray<const FMeshDescription*> meshDescPtrs;
	//meshDescPtrs.Emplace(&meshDesc);
	//staticMesh->BuildFromMeshDescriptions(meshDescPtrs, mdParams);

	//WaterStaticMesh->SetStaticMesh(staticMesh);
	//WaterStaticMesh->SetMaterial(0, WaterMeshMaterial);


	WaterMesh->CreateMeshSection(0, WaterVertices, WaterTriangles, WaterNormals, WaterUV0, WaterVertexColour, WaterTangents, false);
	WaterMesh->SetMaterial(0, WaterMeshMaterial);
}