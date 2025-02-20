// Fill out your copyright notice in the Description page of Project Settings.


#include "Asteroid.h"
#include "MusicAsteroidsPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AAsteroid::AAsteroid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//static ConstructorHelpers::FObjectFinder<UStaticMesh> AsteroidMesh(TEXT("/Game/TwinStick/Meshes/Asteroid"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> AsteroidMesh(TEXT("/Game/TwinStick/Meshes/SM_VinylRecord"));
	// Create the mesh component
	AsteroidMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AsteroidMesh"));
	RootComponent = AsteroidMeshComponent;
	AsteroidMeshComponent->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	AsteroidMeshComponent->BodyInstance.SetCollisionProfileName("Projectile");
	AsteroidMeshComponent->OnComponentHit.AddDynamic(this, &AAsteroid::OnHit);		// set up a notification for when this component hits something
	//AsteroidMeshComponent->SetSimulatePhysics(true);
	AsteroidMeshComponent->SetStaticMesh(AsteroidMesh.Object);
	AsteroidMeshComponent->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));
	FRotator newRot;
	newRot.Pitch = -0.5;
	AsteroidMeshComponent->AddLocalRotation(newRot);

	//TODO: Use different materials for different sizes, i.e.,  LP, 45, and cd for large, medium, and small
	CDMaterial = CreateDefaultSubobject<UMaterial>(TEXT("/Game/TwinStick/Materials/CD_material"));
	AsteroidMeshComponent->SetMaterial(0, CDMaterial);

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement0"));
	ProjectileMovement->UpdatedComponent = AsteroidMeshComponent;
	ProjectileMovement->InitialSpeed = 200.f;
	ProjectileMovement->MaxSpeed = 300.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->ProjectileGravityScale = 0.f; // No gravity
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

void AAsteroid::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		if (OtherActor->IsA(AMusicAsteroidsPawn::StaticClass()))
		{

		}
		OtherComp->AddImpulseAtLocation(GetVelocity() * 20.0f, GetActorLocation());
	}
}

void AAsteroid::AsteroidHit()
{
	//TODO: animate explosion, add sound (I wanted to add some procedural audio by playing note sound files from the same scale depending on asterpoid size)
	AsteroidMeshComponent->ToggleVisibility();
}