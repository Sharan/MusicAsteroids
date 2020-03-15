// Fill out your copyright notice in the Description page of Project Settings.


#include "Asteroid.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"

// Sets default values
AAsteroid::AAsteroid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> AsteroidMesh(TEXT("/Game/TwinStick/Meshes/Asteroid"));
	// Create the mesh component
	AsteroidMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AsteroidMesh"));
	RootComponent = AsteroidMeshComponent;
	AsteroidMeshComponent->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	AsteroidMeshComponent->SetStaticMesh(AsteroidMesh.Object);

	//TODO: Use different materials for different sizes, i.e.,  LP, 45, and cd for large, medium, and small
	CDMaterial = CreateDefaultSubobject<UMaterial>(TEXT("CD_material"));
	AsteroidMeshComponent->SetMaterial(0, CDMaterial);
}

// Called when the game starts or when spawned
void AAsteroid::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAsteroid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

